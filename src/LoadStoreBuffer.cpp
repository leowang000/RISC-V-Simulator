#include "ALU.h"
#include "Decoder.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "RegisterFile.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

LoadStoreBuffer::LoadStoreBuffer(const Clock &clock) : lsb_(), to_mem_(), wc_(clock) {}

void LoadStoreBuffer::Debug() const {
  std::cout << "Load/Store Buffer:\n";
  std::cout << "\tlsb_ = {\n";
  for (int i = lsb_.GetCur().BeginId(); i != lsb_.GetCur().EndId(); i = (i + 1) % (kLSBSize + 1)) {
    std::cout << "\t" << i << "\t" << lsb_.GetCur()[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\tto_mem_ = " << to_mem_.GetCur().ToString() << "\n\n";
}

bool LoadStoreBuffer::IsFull() const {
  return lsb_.GetCur().IsFull();
}

void LoadStoreBuffer::Update() {
  lsb_.Update();
  to_mem_.Update();
  wc_.Update();
}

void LoadStoreBuffer::Execute(const ALU &alu, const Decoder &decoder, const Memory &memory, const ReorderBuffer &rb,
                              const RegisterFile &rf, const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, flush = rb.flush_.GetCur().flush_, reg_value = rf.GetRegisterValue(rb),
      reg_status = rf.GetRegisterStatus(rb), rb_to_mem = rb.to_mem_.GetCur(), rb_queue = rb.GetRB(memory, alu),
      from_decoder = decoder.output_.GetCur(), from_mem = memory.output_.GetCur(), from_alu = alu.output_.GetCur(),
      is_mem_busy = memory.IsDataBusy(), stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), IsFull())]() {
    if (flush) {
      Flush();
      return;
    }
    bool is_front_load = !lsb_.GetCur().IsEmpty() &&
                         (lsb_.GetCur().Front().inst_type_ == kLB || lsb_.GetCur().Front().inst_type_ == kLH ||
                          lsb_.GetCur().Front().inst_type_ == kLW || lsb_.GetCur().Front().inst_type_ == kLBU ||
                          lsb_.GetCur().Front().inst_type_ == kLHU);
    bool is_new_inst_load = (from_decoder.inst_type_ == kLB || from_decoder.inst_type_ == kLH ||
                             from_decoder.inst_type_ == kLW || from_decoder.inst_type_ == kLBU ||
                             from_decoder.inst_type_ == kLHU);
    bool is_new_inst_store = (from_decoder.inst_type_ == kSB || from_decoder.inst_type_ == kSH ||
                              from_decoder.inst_type_ == kSW);
    EnqueueInst(stall, is_new_inst_store, is_new_inst_load, from_decoder, reg_value, reg_status, rb_queue);
    UpdateDependencies(from_mem, from_alu);
    bool dequeue_load = WriteToMemory(is_front_load, is_mem_busy);
    if (dequeue_load || rb_to_mem.store_) {
      lsb_.New().Dequeue();
    }
  };
  wc_.Set(write_func, 1);
}

void LoadStoreBuffer::Write() {
  wc_.Write();
}

void LoadStoreBuffer::ForceWrite() {
  wc_.ForceWrite();
}

void LoadStoreBuffer::Flush() {
  lsb_.New().Clear();
  to_mem_.New().load_ = false;
}

void LoadStoreBuffer::EnqueueInst(bool stall, bool is_new_inst_store, bool is_new_inst_load,
                                  const DecoderOutput &from_decoder, const std::array<uint32_t, kXLen> &reg_value,
                                  const std::array<int, kXLen> &reg_status,
                                  const CircularQueue<RoBEntry, kRoBSize> &rb_queue) {
  if (stall || !from_decoder.get_inst_ || (!is_new_inst_store && !is_new_inst_load)) {
    return;
  }
  LSBEntry lsb_entry;
  lsb_entry.inst_type_ = from_decoder.inst_type_;
  lsb_entry.id_ = rb_queue.EndId();
  lsb_entry.Q1_ = reg_status[from_decoder.rs1_];
  if (lsb_entry.Q1_ == -1) {
    lsb_entry.V1_ = reg_value[from_decoder.rs1_] + from_decoder.imm_;
  }
  else {
    if (rb_queue[lsb_entry.Q1_].done_) {
      lsb_entry.V1_ = rb_queue[lsb_entry.Q1_].val_ + from_decoder.imm_;
      lsb_entry.Q1_ = -1;
    }
    else {
      lsb_entry.V1_ = from_decoder.imm_;
    }
  }
  if (is_new_inst_load) {
    lsb_entry.Q2_ = -1;
  }
  if (is_new_inst_store) {
    lsb_entry.Q2_ = reg_status[from_decoder.rs2_];
    if (lsb_entry.Q2_ == -1) {
      lsb_entry.V2_ = reg_value[from_decoder.rs2_];
    }
    else {
      if (rb_queue[lsb_entry.Q2_].done_) {
        lsb_entry.V2_ = rb_queue[lsb_entry.Q2_].val_;
        lsb_entry.Q2_ = -1;
      }
    }
  }
  lsb_.New().Enqueue(lsb_entry);
}

void LoadStoreBuffer::UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu) {
  if (from_mem.done_) {
    for (int i = lsb_.New().BeginId(); i != lsb_.New().EndId(); i = (i + 1) % (kLSBSize + 1)) {
      if (lsb_.New()[i].Q1_ == from_mem.id_) {
        lsb_.New()[i].Q1_ = -1;
        lsb_.New()[i].V1_ += from_mem.val_;
      }
      if (lsb_.New()[i].Q2_ == from_mem.id_) {
        lsb_.New()[i].Q2_ = -1;
        lsb_.New()[i].V2_ = from_mem.val_;
      }
    }
  }
  if (from_alu.done_) {
    for (int i = lsb_.New().BeginId(); i != lsb_.New().EndId(); i = (i + 1) % (kLSBSize + 1)) {
      if (lsb_.New()[i].Q1_ == from_alu.id_) {
        lsb_.New()[i].Q1_ = -1;
        lsb_.New()[i].V1_ += from_alu.val_;
      }
      if (lsb_.New()[i].Q2_ == from_alu.id_) {
        lsb_.New()[i].Q2_ = -1;
        lsb_.New()[i].V2_ = from_alu.val_;
      }
    }
  }
}

bool LoadStoreBuffer::WriteToMemory(bool is_front_load, bool is_mem_busy) {
  to_mem_.New().load_ = false;
  if (!is_front_load || is_mem_busy || to_mem_.GetCur().load_ || lsb_.GetCur().Front().Q1_ != -1) {
    return false;
  }
  to_mem_.New().load_ = true;
  to_mem_.New().load_addr_ = lsb_.GetCur().Front().V1_;
  to_mem_.New().inst_type_ = lsb_.GetCur().Front().inst_type_;
  to_mem_.New().id_ = lsb_.GetCur().Front().id_;
  return true;
}

}