#include "ALU.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

ALU::ALU(const Clock &clock) : output_(), wc_(clock) {}

void ALU::Update() {
  output_.Update();
  wc_.Update();
}

void ALU::Execute(const ReorderBuffer &rb, const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, from_rs = rs.to_alu_.GetCur(), flush = rb.flush_.GetCur().flush_]() {
    if (flush) {
      Flush();
      return;
    }
    WriteToOutput(from_rs);
  };
  wc_.Set(write_func, 1);
}

void ALU::Write() {
  wc_.Write();
}

void ALU::ForceWrite() {
  wc_.ForceWrite();
}

void ALU::Flush() {
  output_.New().done_ = false;
}

void ALU::WriteToOutput(const RSToALU &from_rs) {
  output_.New().done_ = false;
  if (!from_rs.execute_) {
    return;
  }
  output_.New().id_ = from_rs.id_;
  switch (from_rs.alu_op_type_) {
    case kAdd:
      output_.New().val_ = from_rs.in1_ + from_rs.in2_;
      break;
    case kSub:
      output_.New().val_ = from_rs.in1_ - from_rs.in2_;
      break;
    case kAnd:
      output_.New().val_ = from_rs.in1_ & from_rs.in2_;
      break;
    case kOr:
      output_.New().val_ = from_rs.in1_ | from_rs.in2_;
      break;
    case kXor:
      output_.New().val_ = from_rs.in1_ ^ from_rs.in2_;
      break;
    case kShiftLeftLogical:
      output_.New().val_ = from_rs.in1_ << from_rs.in2_;
      break;
    case kShiftRightLogical:
      output_.New().val_ = from_rs.in1_ >> from_rs.in2_;
      break;
    case kShiftRightArithmetic:
      output_.New().val_ = static_cast<int32_t>(from_rs.in1_) >> static_cast<int32_t>(from_rs.in2_);
      break;
    case kEqual:
      output_.New().val_ = (from_rs.in1_ == from_rs.in2_);
      break;
    case kNotEqual:
      output_.New().val_ = (from_rs.in1_ != from_rs.in2_);
      break;
    case kLessThan:
      output_.New().val_ = (static_cast<int32_t>(from_rs.in1_) < static_cast<int32_t>(from_rs.in2_));
      break;
    case kLessThanUnsigned:
      output_.New().val_ = (from_rs.in1_ < from_rs.in2_);
      break;
    case kGreaterOrEqual:
      output_.New().val_ = static_cast<int32_t>(from_rs.in1_) >= static_cast<int32_t>(from_rs.in2_);
      break;
    case kGreaterOrEqualUnsigned:
      output_.New().val_ = (from_rs.in1_ >= from_rs.in2_);
      break;
  }
  output_.New().done_ = true;
}

}