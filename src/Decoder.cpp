#include "utils/NumberOperation.h"

#include "Decoder.h"
#include "InstructionUnit.h"
#include "LoadStoreBuffer.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

Decoder::Decoder(const Clock &clock) : wc_(clock), output_() {}

bool Decoder::IsStallNeeded(bool is_rb_full, bool is_rs_full, bool is_lsb_full) const {
  if (!output_.GetCur().get_inst_) {
    return false;
  }
  switch (output_.GetCur().inst_type_) {
    case kHALT:
    case kLUI:
    case kAUIPC:
    case kJAL:
      return is_rb_full;
    case kLB:
    case kLH:
    case kLW:
    case kLBU:
    case kLHU:
    case kSB:
    case kSH:
    case kSW:
      return is_rb_full || is_lsb_full;
    default:
      return is_rb_full || is_rs_full;
  }
}

void Decoder::Update() {
  output_.Update();
  wc_.Update();
}

void Decoder::Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb,
                      const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, from_iu = iu.to_decoder_.GetCur(), flush = rb.flush_.GetCur().flush_,
      is_rb_full = rb.IsFull(), is_rs_full = rs.IsFull(), is_lsb_full = lsb.IsFull()] {
    bool stall = IsStallNeeded(is_rb_full, is_rs_full, is_lsb_full);
    if (flush) {
      Flush();
      return;
    }
    if (!stall || !output_.GetCur().get_inst_) {
      WriteToOutput(from_iu);
    }
  };
  wc_.Set(write_func, 1);
}

void Decoder::Write() {
  wc_.Write();
}

void Decoder::ForceWrite() {
  wc_.ForceWrite();
}

bool Decoder::GetOperands(DecoderOutput &out, uint32_t inst) {
  switch (inst & 0b1111111) {
    case 0b0110111: // U-type
    case 0b0010111:
      out.imm_ = GetSub(inst, 31, 12) << 12;
      out.rd_ = GetSub(inst, 11, 7);
      break;
    case 0b1101111: // J-type
      out.imm_ = SignExtend((GetSub(inst, 31, 31) << 20) | (GetSub(inst, 19, 12) << 12) | (GetSub(inst, 20, 20) << 11) |
                            (GetSub(inst, 30, 25) << 5) | (GetSub(inst, 24, 21) << 1), 20);
      out.rd_ = GetSub(inst, 11, 7);
      break;
    case 0b1100011: // B-type
      out.imm_ = SignExtend((GetSub(inst, 31, 31) << 12 | (GetSub(inst, 7, 7) << 11) | (GetSub(inst, 30, 25) << 5) |
                             (GetSub(inst, 11, 8) << 1)), 12);
      out.rs1_ = GetSub(inst, 19, 15);
      out.rs2_ = GetSub(inst, 24, 20);
      break;
    case 0b0100011: // S-type
      out.imm_ = SignExtend((GetSub(inst, 31, 31) << 11) | (GetSub(inst, 30, 25) << 5) | GetSub(inst, 11, 7), 11);
      out.rs1_ = GetSub(inst, 19, 15);
      out.rs2_ = GetSub(inst, 24, 20);
      break;
    case 0b0110011: // R-type
      out.rd_ = GetSub(inst, 11, 7);
      out.rs1_ = GetSub(inst, 19, 15);
      out.rs2_ = GetSub(inst, 24, 20);
      break;
    case 0b1100111: // I-type
    case 0b0010011:
    case 0b0000011:
      out.rd_ = GetSub(inst, 11, 7);
      out.rs1_ = GetSub(inst, 19, 15);
      out.imm_ = (inst & 0b1111111) == 0b0010011 && GetSub(inst, 13, 12) == 0b01 ? GetSub(inst, 24, 20) : SignExtend(
          GetSub(inst, 31, 20), 11);
      break;
    default:
      return false;
  }
  return true;
}

bool Decoder::GetInstType(DecoderOutput &out, uint32_t inst) {
  if (inst == 0x0ff00513) {
    out.inst_type_ = kHALT;
    return true;
  }
  switch (inst & 0b1111111) {
    case 0b0110111:
      out.inst_type_ = kLUI;
      break;
    case 0010111:
      out.inst_type_ = kAUIPC;
      break;
    case 0b1101111:
      out.inst_type_ = kJAL;
      break;
    case 0b1100111:
      out.inst_type_ = kJALR;
      break;
    case 0b1100011:
      switch (GetSub(inst, 14, 12)) {
        case 0b000:
          out.inst_type_ = kBEQ;
          break;
        case 0b001:
          out.inst_type_ = kBNE;
          break;
        case 0b100:
          out.inst_type_ = kBLT;
          break;
        case 0b101:
          out.inst_type_ = kBGE;
          break;
        case 0b110:
          out.inst_type_ = kBLTU;
          break;
        case 0b111:
          out.inst_type_ = kBGEU;
          break;
        default:
          return false;
      }
      break;
    case 0b0100011:
      switch (GetSub(inst, 14, 12)) {
        case 0b000:
          out.inst_type_ = kSB;
          break;
        case 0b001:
          out.inst_type_ = kSH;
          break;
        case 0b010:
          out.inst_type_ = kSW;
          break;
        default:
          return false;
      }
      break;
    case 0b0110011:
      switch (GetSub(inst, 14, 12)) {
        case 0b000:
          out.inst_type_ = (GetSub(inst, 30, 30) ? kSUB : kADD);
          break;
        case 0b001:
          out.inst_type_ = kSLL;
          break;
        case 0b010:
          out.inst_type_ = kSLT;
          break;
        case 0b011:
          out.inst_type_ = kSLTU;
          break;
        case 0b100:
          out.inst_type_ = kXOR;
          break;
        case 0b101:
          out.inst_type_ = (GetSub(inst, 30, 30) ? kSRA : kSRL);
          break;
        case 0b110:
          out.inst_type_ = kOR;
          break;
        case 0b111:
          out.inst_type_ = kAND;
          break;
      }
      break;
    case 0b0010011:
      switch (GetSub(inst, 14, 12)) {
        case 0b000:
          out.inst_type_ = kADDI;
          break;
        case 0b001:
          out.inst_type_ = kSLLI;
          break;
        case 0b010:
          out.inst_type_ = kSLTI;
          break;
        case 0b011:
          out.inst_type_ = kSLTIU;
          break;
        case 0b100:
          out.inst_type_ = kXORI;
          break;
        case 0b101:
          out.inst_type_ = (GetSub(inst, 30, 30) ? kSRAI : kSRLI);
          break;
        case 0b110:
          out.inst_type_ = kORI;
          break;
        case 0b111:
          out.inst_type_ = kANDI;
          break;
      }
      break;
    case 0b0000011:
      switch (GetSub(inst, 14, 12)) {
        case 0b000:
          out.inst_type_ = kLB;
          break;
        case 0b001:
          out.inst_type_ = kLH;
          break;
        case 0b010:
          out.inst_type_ = kLW;
          break;
        case 0b100:
          out.inst_type_ = kLBU;
          break;
        case 0b101:
          out.inst_type_ = kLHU;
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

void Decoder::Flush() {
  output_.New().get_inst_ = false;
}

void Decoder::WriteToOutput(const IUToDecoder &from_iu) {
  output_.New().get_inst_ = false;
  if (!from_iu.get_inst_) {
    return;
  }
  DecoderOutput out;
  out.addr_ = from_iu.addr_;
  out.is_jump_predicted_ = from_iu.is_jump_predicted_;
  bool get_operands_success = GetOperands(out, from_iu.inst_);
  if (!get_operands_success) {
    return;
  }
  bool get_inst_type_success = GetInstType(out, from_iu.inst_);
  if (!get_inst_type_success) {
    return;
  }
  if (out.rd_ > 0b11111 || out.rs1_ > 0b11111 || out.rs2_ > 0b11111) {
    return;
  }
  out.get_inst_ = true;
  output_.Write(out);
}

}
