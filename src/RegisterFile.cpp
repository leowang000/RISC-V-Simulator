#include "Decoder.h"
#include "LoadStoreBuffer.h"
#include "RegisterFile.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

RegisterFile::RegisterFile(const Clock &clock) : wc_(clock), value_(), status_() {
  for (auto &reg_status: status_) {
    reg_status = Register<int>(-1);
  }
}

void RegisterFile::Debug(const ReorderBuffer &rb) const {
  std::cout << "Register File:\n";
  std::cout << "\tid\tV\tV_r\tQ\tQ_r\t\tid\tV\tV_r\tQ\tQ_r\n";
  auto reg_value = GetRegisterValue(rb);
  auto reg_status = GetRegisterStatus(rb);
  for (int i = 0; i < kXLen / 2; i++) {
    std::cout << "\t" << i << "\t" << value_[i].GetCur() << "\t" << reg_value[i] << "\t" << status_[i].GetCur() << "\t"
              << reg_status[i] << "\t\t" << i + kXLen / 2 << "\t" << value_[i + kXLen / 2].GetCur() << "\t"
              << reg_value[i + kXLen / 2] << "\t" << status_[i + kXLen / 2].GetCur() << "\t"
              << reg_status[i + kXLen / 2] << "\n";
  }
  std::cout << "\n";
}

std::array<uint32_t, kXLen> RegisterFile::GetRegisterValue(const ReorderBuffer &rb) const {
  std::array<uint32_t, kXLen> res;
  res[0] = 0;
  for (int i = 1; i < kXLen; i++) {
    res[i] =
        rb.to_rf_.GetCur().write_ && i == rb.to_rf_.GetCur().rd_ ? rb.to_rf_.GetCur().val_ : value_[i].GetCur();
  }
  return res;
}

std::array<int, kXLen> RegisterFile::GetRegisterStatus(const ReorderBuffer &rb) const {
  std::array<int, kXLen> res;
  res[0] = -1;
  for (int i = 1; i < kXLen; i++) {
    int prev_begin_id = rb.rb_.GetCur().BeginId() == 0 ? kRoBSize : rb.rb_.GetCur().BeginId() - 1;
    res[i] = rb.to_rf_.GetCur().write_ && i == rb.to_rf_.GetCur().rd_ &&
             prev_begin_id == status_[i].GetCur() ? -1 : status_[i].GetCur();
  }
  return res;
}

void RegisterFile::Update() {
  for (auto &reg: value_) {
    reg.Update();
  }
  for (auto &reg_status: status_) {
    reg_status.Update();
  }
  wc_.Update();
}

void RegisterFile::Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const ReorderBuffer &rb,
                           const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, flush = rb.flush_.GetCur().flush_,
      stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), lsb.IsFull()), from_rb = rb.to_rf_.GetCur(),
      from_decoder = decoder.output_.GetCur(), rb_begin_id = rb.rb_.GetCur().BeginId(),
      rb_end_id = rb.rb_.GetCur().EndId()]() {
    if (flush) {
      Flush();
      return;
    }
    RemoveDependencyAndWrite(from_rb, rb_begin_id == 0 ? kRoBSize : rb_begin_id - 1);
    AddDependency(stall, from_decoder, rb_end_id);
  };
  wc_.Set(write_func, 1);
}

void RegisterFile::Write() {
  wc_.Write();
}

void RegisterFile::ForceWrite() {
  wc_.ForceWrite();
}

void RegisterFile::Flush() {
  for (int i = 0; i < kXLen; i++) {
    status_[i].Write(-1);
  }
}

void RegisterFile::AddDependency(bool stall, const DecoderOutput &from_decoder, int rob_new_inst_id) {
  if (!stall && from_decoder.get_inst_) {
    switch (from_decoder.inst_type_) {
      case kHALT:
      case kBEQ:
      case kBNE:
      case kBLT:
      case kBGE:
      case kBLTU:
      case kBGEU:
      case kSB:
      case kSH:
      case kSW:
        break;
      default:
        if (from_decoder.rd_ != 0) {
          status_[from_decoder.rd_].Write(rob_new_inst_id);
        }
        break;
    }
  }
}

void RegisterFile::RemoveDependencyAndWrite(const RobToRF &from_rb, int rob_commit_inst_id) {
  if (from_rb.write_) {
    if (from_rb.rd_ != 0) {
      value_[from_rb.rd_].Write(from_rb.val_);
      if (rob_commit_inst_id == status_[from_rb.rd_].GetCur()) {
        status_[from_rb.rd_].Write(-1);
      }
    }
  }
}

}