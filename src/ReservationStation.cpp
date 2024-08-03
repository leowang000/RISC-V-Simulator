#include "ALU.h"
#include "Decoder.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "RegisterFile.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

ReservationStation::ReservationStation(const Clock &clock) : rs_(), to_alu_(), wc_(clock) {}

void ReservationStation::Debug() const {
  std::cout << "Reservation Station:\n";
  std::cout << "\trs_ = {\n";
  for (int i = 0; i < kRSSize; i++) {
    std::cout << "\t" << i << "\t" << rs_.GetCur()[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\tto_alu_ = " << to_alu_.GetCur().ToString() << "\n\n";
}

bool ReservationStation::IsFull() const {
  for (int i = 0; i < kRSSize; i++) {
    if (!rs_.GetCur()[i].busy_) {
      return false;
    }
  }
  return true;
}

void ReservationStation::Update() {
  rs_.Update();
  to_alu_.Update();
  wc_.Update();
}

#ifdef _DEBUG
void
ReservationStation::Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                            const ReorderBuffer &rb, const RegisterFile &rf) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, flush = rb.flush_.GetCur().flush_, reg_value = rf.GetRegisterValue(rb),
      reg_status = rf.GetRegisterStatus(rb), rb_queue = rb.GetRB(memory, alu), from_decoder = decoder.output_.GetCur(),
      from_mem = memory.output_.GetCur(), from_alu = alu.output_.GetCur(),
      stall = decoder.IsStallNeeded(rb.IsFull(), IsFull(), lsb.IsFull())]() {
    if (flush) {
      Flush();
      return;
    }
    InsertInst(stall, from_decoder, reg_value, reg_status, rb_queue);
    UpdateDependencies(from_mem, from_alu);
    int rs_id = WriteToALU(rb_queue);
    if (rs_id != -1) {
      rs_.New()[rs_id].busy_ = false;
    }
  };
  wc_.Set(write_func, 1);
}
#else

void
ReservationStation::Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                            const ReorderBuffer &rb, const RegisterFile &rf) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [](ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                       RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) {
    bool stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), lsb.IsFull());
    if (rb.flush_.GetCur().flush_) {
      rs.Flush();
      return;
    }
    rs.InsertInst(stall, decoder.output_.GetCur(), rf, rb, memory, alu);
    rs.UpdateDependencies(memory.output_.GetCur(), alu.output_.GetCur());
    int rs_id = rs.WriteToALU(rb, memory, alu);
    if (rs_id != -1) {
      rs.rs_.New()[rs_id].busy_ = false;
    }
  };
  wc_.Set(write_func, 1);
}

#endif

#ifdef _DEBUG
void ReservationStation::Write() {
  wc_.Write();
}

void ReservationStation::ForceWrite() {
  wc_.ForceWrite();
}
#else

void ReservationStation::Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                               RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) {
  wc_.Write(alu, decoder, iu, lsb, memory, rf, rb, rs);
}

void
ReservationStation::ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                               RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) {
  wc_.ForceWrite(alu, decoder, iu, lsb, memory, rf, rb, rs);
}

#endif

void ReservationStation::Flush() {
  for (int i = 0; i < kRSSize; i++) {
    rs_.New()[i].busy_ = false;
  }
  to_alu_.New().execute_ = false;
}

void ReservationStation::InsertInst(bool stall, const DecoderOutput &from_decoder,
                                    const std::array<uint32_t, kXLen> &reg_value,
                                    const std::array<int, kXLen> &reg_status,
                                    const CircularQueue<RoBEntry, kRoBSize> &rb_queue) {
  if (stall || !from_decoder.get_inst_) {
    return;
  }
  bool two_op = false;
  switch (from_decoder.inst_type_) {
    case kLUI:
    case kAUIPC:
    case kJAL:
    case kLB:
    case kLH:
    case kLW:
    case kLBU:
    case kLHU:
    case kSB:
    case kSH:
    case kSW:
    case kHALT:
      return;
    case kJALR:
    case kADDI:
    case kSLTI:
    case kSLTIU:
    case kXORI:
    case kORI:
    case kANDI:
    case kSLLI:
    case kSRLI:
    case kSRAI:
      break;
    case kBEQ:
    case kBNE:
    case kBLT:
    case kBGE:
    case kBLTU:
    case kBGEU:
    case kADD:
    case kSUB:
    case kSLL:
    case kSLT:
    case kSLTU:
    case kXOR:
    case kSRL:
    case kSRA:
    case kOR:
    case kAND:
      two_op = true;
      break;
  }
  RSEntry rs_entry;
  rs_entry.busy_ = true;
  rs_entry.id_ = rb_queue.EndId();
  int rs_id;
  for (int i = 0; i < kRSSize; i++) {
    if (!rs_.GetCur()[i].busy_) {
      rs_id = i;
      break;
    }
  }
  rs_entry.Q1_ = reg_status[from_decoder.rs1_];
  if (rs_entry.Q1_ == -1) {
    rs_entry.V1_ = reg_value[from_decoder.rs1_];
  }
  else {
    if (rb_queue[rs_entry.Q1_].done_) {
      rs_entry.V1_ = rb_queue[rs_entry.Q1_].val_;
      rs_entry.Q1_ = -1;
    }
  }
  if (two_op) {
    rs_entry.Q2_ = reg_status[from_decoder.rs2_];
    if (rs_entry.Q2_ == -1) {
      rs_entry.V2_ = reg_value[from_decoder.rs2_];
    }
    else {
      if (rb_queue[rs_entry.Q2_].done_) {
        rs_entry.V2_ = rb_queue[rs_entry.Q2_].val_;
        rs_entry.Q2_ = -1;
      }
    }
  }
  else {
    rs_entry.Q2_ = -1;
    rs_entry.V2_ = from_decoder.imm_;
  }
  rs_.New()[rs_id] = rs_entry;
}

void ReservationStation::InsertInst(bool stall, const DecoderOutput &from_decoder, const RegisterFile &rf,
                                    const ReorderBuffer &rb, const Memory &memory, const ALU &alu) {
  if (stall || !from_decoder.get_inst_) {
    return;
  }
  bool two_op = false;
  switch (from_decoder.inst_type_) {
    case kLUI:
    case kAUIPC:
    case kJAL:
    case kLB:
    case kLH:
    case kLW:
    case kLBU:
    case kLHU:
    case kSB:
    case kSH:
    case kSW:
    case kHALT:
      return;
    case kJALR:
    case kADDI:
    case kSLTI:
    case kSLTIU:
    case kXORI:
    case kORI:
    case kANDI:
    case kSLLI:
    case kSRLI:
    case kSRAI:
      break;
    case kBEQ:
    case kBNE:
    case kBLT:
    case kBGE:
    case kBLTU:
    case kBGEU:
    case kADD:
    case kSUB:
    case kSLL:
    case kSLT:
    case kSLTU:
    case kXOR:
    case kSRL:
    case kSRA:
    case kOR:
    case kAND:
      two_op = true;
      break;
  }
  RSEntry rs_entry;
  rs_entry.busy_ = true;
  rs_entry.id_ = rb.rb_.GetCur().EndId();
  int rs_id;
  for (int i = 0; i < kRSSize; i++) {
    if (!rs_.GetCur()[i].busy_) {
      rs_id = i;
      break;
    }
  }
  rs_entry.Q1_ = rf.GetRegisterStatus(from_decoder.rs1_, rb);
  if (rs_entry.Q1_ == -1) {
    rs_entry.V1_ = rf.GetRegisterValue(from_decoder.rs1_, rb);
  }
  else {
    RoBEntry rb_Q1 = rb.GetRB(rs_entry.Q1_, memory, alu);
    if (rb_Q1.done_) {
      rs_entry.V1_ = rb_Q1.val_;
      rs_entry.Q1_ = -1;
    }
  }
  if (two_op) {
    rs_entry.Q2_ = rf.GetRegisterStatus(from_decoder.rs2_, rb);
    if (rs_entry.Q2_ == -1) {
      rs_entry.V2_ = rf.GetRegisterValue(from_decoder.rs2_, rb);
    }
    else {
      RoBEntry rb_Q2 = rb.GetRB(rs_entry.Q2_, memory, alu);
      if (rb_Q2.done_) {
        rs_entry.V2_ = rb_Q2.val_;
        rs_entry.Q2_ = -1;
      }
    }
  }
  else {
    rs_entry.Q2_ = -1;
    rs_entry.V2_ = from_decoder.imm_;
  }
  rs_.New()[rs_id] = rs_entry;
}

void ReservationStation::UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu) {
  if (from_mem.done_) {
    for (int i = 0; i < kRSSize; i++) {
      if (rs_.New()[i].busy_) {
        if (rs_.New()[i].Q1_ == from_mem.id_) {
          rs_.New()[i].Q1_ = -1;
          rs_.New()[i].V1_ = from_mem.val_;
        }
        if (rs_.New()[i].Q2_ == from_mem.id_) {
          rs_.New()[i].Q2_ = -1;
          rs_.New()[i].V2_ = from_mem.val_;
        }
      }
    }
  }
  if (from_alu.done_) {
    for (int i = 0; i < kRSSize; i++) {
      if (rs_.New()[i].busy_) {
        if (rs_.New()[i].Q1_ == from_alu.id_) {
          rs_.New()[i].Q1_ = -1;
          rs_.New()[i].V1_ = from_alu.val_;
        }
        if (rs_.New()[i].Q2_ == from_alu.id_) {
          rs_.New()[i].Q2_ = -1;
          rs_.New()[i].V2_ = from_alu.val_;
        }
      }
    }
  }
}

int ReservationStation::WriteToALU(const CircularQueue<RoBEntry, kRoBSize> &rb_queue) {
  to_alu_.New().execute_ = false;
  int rs_id = -1;
  for (int i = 0; i < kRSSize; i++) {
    if (rs_.GetCur()[i].busy_ && rs_.GetCur()[i].Q1_ == -1 && rs_.GetCur()[i].Q2_ == -1) {
      rs_id = i;
      break;
    }
  }
  if (rs_id == -1) {
    return -1;
  }
  RSToALU to_alu;
  to_alu.execute_ = true;
  to_alu.in1_ = rs_.GetCur()[rs_id].V1_;
  to_alu.in2_ = rs_.GetCur()[rs_id].V2_;
  to_alu.id_ = rs_.GetCur()[rs_id].id_;
  switch (rb_queue[rs_.GetCur()[rs_id].id_].inst_type_) {
    case kJALR:
    case kADD:
    case kADDI:
      to_alu.alu_op_type_ = kAdd;
      break;
    case kSUB:
      to_alu.alu_op_type_ = kSub;
      break;
    case kAND:
    case kANDI:
      to_alu.alu_op_type_ = kAnd;
      break;
    case kOR:
    case kORI:
      to_alu.alu_op_type_ = kOr;
      break;
    case kXOR:
    case kXORI:
      to_alu.alu_op_type_ = kXor;
      break;
    case kSLL:
    case kSLLI:
      to_alu.alu_op_type_ = kShiftLeftLogical;
      break;
    case kSRL:
    case kSRLI:
      to_alu.alu_op_type_ = kShiftRightLogical;
      break;
    case kSRA:
    case kSRAI:
      to_alu.alu_op_type_ = kShiftRightArithmetic;
      break;
    case kBEQ:
      to_alu.alu_op_type_ = kEqual;
      break;
    case kBNE:
      to_alu.alu_op_type_ = kNotEqual;
      break;
    case kSLT:
    case kSLTI:
    case kBLT:
      to_alu.alu_op_type_ = kLessThan;
      break;
    case kSLTU:
    case kSLTIU:
    case kBLTU:
      to_alu.alu_op_type_ = kLessThanUnsigned;
      break;
    case kBGE:
      to_alu.alu_op_type_ = kGreaterOrEqual;
      break;
    case kBGEU:
      to_alu.alu_op_type_ = kGreaterOrEqualUnsigned;
      break;
    default:
      break;
  }
  to_alu_.Write(to_alu);
  return rs_id;
}

int ReservationStation::WriteToALU(const ReorderBuffer &rb, const Memory &memory, const ALU &alu) {
  to_alu_.New().execute_ = false;
  int rs_id = -1;
  for (int i = 0; i < kRSSize; i++) {
    if (rs_.GetCur()[i].busy_ && rs_.GetCur()[i].Q1_ == -1 && rs_.GetCur()[i].Q2_ == -1) {
      rs_id = i;
      break;
    }
  }
  if (rs_id == -1) {
    return -1;
  }
  RSToALU to_alu;
  to_alu.execute_ = true;
  to_alu.in1_ = rs_.GetCur()[rs_id].V1_;
  to_alu.in2_ = rs_.GetCur()[rs_id].V2_;
  to_alu.id_ = rs_.GetCur()[rs_id].id_;
  switch (rb.GetRB(rs_.GetCur()[rs_id].id_, memory, alu).inst_type_) {
    case kJALR:
    case kADD:
    case kADDI:
      to_alu.alu_op_type_ = kAdd;
      break;
    case kSUB:
      to_alu.alu_op_type_ = kSub;
      break;
    case kAND:
    case kANDI:
      to_alu.alu_op_type_ = kAnd;
      break;
    case kOR:
    case kORI:
      to_alu.alu_op_type_ = kOr;
      break;
    case kXOR:
    case kXORI:
      to_alu.alu_op_type_ = kXor;
      break;
    case kSLL:
    case kSLLI:
      to_alu.alu_op_type_ = kShiftLeftLogical;
      break;
    case kSRL:
    case kSRLI:
      to_alu.alu_op_type_ = kShiftRightLogical;
      break;
    case kSRA:
    case kSRAI:
      to_alu.alu_op_type_ = kShiftRightArithmetic;
      break;
    case kBEQ:
      to_alu.alu_op_type_ = kEqual;
      break;
    case kBNE:
      to_alu.alu_op_type_ = kNotEqual;
      break;
    case kSLT:
    case kSLTI:
    case kBLT:
      to_alu.alu_op_type_ = kLessThan;
      break;
    case kSLTU:
    case kSLTIU:
    case kBLTU:
      to_alu.alu_op_type_ = kLessThanUnsigned;
      break;
    case kBGE:
      to_alu.alu_op_type_ = kGreaterOrEqual;
      break;
    case kBGEU:
      to_alu.alu_op_type_ = kGreaterOrEqualUnsigned;
      break;
    default:
      break;
  }
  to_alu_.Write(to_alu);
  return rs_id;
}

}