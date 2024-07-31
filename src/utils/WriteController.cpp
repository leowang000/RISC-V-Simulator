#include "WriteController.h"

namespace bubble {

WriteController::WriteController(const Clock &clock) : clock_(&clock), done_(INT32_MAX), write_func_(), busy_(false) {}

void WriteController::Update() {
  done_.Update();
  busy_ = clock_->GetCycleCount() >= done_.GetCur();
}

bool WriteController::IsReady() const {
  if (!clock_->IsRunning()) {
    return false;
  }
  return clock_->GetCycleCount() >= done_.GetCur();
}

bool WriteController::IsBusy() const {
  return busy_;
}

void WriteController::Set(const std::function<void()> &write_func, int cycle_cnt) {
  if (!clock_->IsRunning() || busy_) {
    return;
  }
  done_.Write(clock_->GetCycleCount() + cycle_cnt);
  write_func_ = write_func;
  busy_ = true;
}

void WriteController::Reset() {
  done_ = Register<uint32_t>(INT32_MAX);
}

void WriteController::Write() const {
  if (!clock_->IsRunning()) {
    return;
  }
  if (clock_->GetCycleCount() == done_.GetCur() - 1) {
    write_func_();
  }
}

void WriteController::ForceWrite() const {
  write_func_();
}

}