#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
WriteController::WriteController(const Clock &clock) :
    clock_(&clock), done_(INT32_MAX), write_func_([]() {}), busy_(false) {}
#else
WriteController::WriteController(const Clock &clock) :
    clock_(&clock), done_(INT32_MAX), write_func_(nullptr), busy_(false) {}
#endif

void WriteController::Update() {
  done_.Update();
  busy_ = done_.GetCur() != INT32_MAX && clock_->GetCycleCount() < done_.GetCur();
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

#ifdef _DEBUG
void WriteController::Set(const std::function<void()> &write_func, int cycle_cnt) {
  if (!clock_->IsRunning() || busy_) {
    return;
  }
  done_.Write(clock_->GetCycleCount() + cycle_cnt);
  write_func_ = write_func;
  busy_ = true;
}
#else

void WriteController::Set(
    void (*write_func)(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                       RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs), int cycle_cnt) {
  if (!clock_->IsRunning() || busy_) {
    return;
  }
  done_.Write(clock_->GetCycleCount() + cycle_cnt);
  write_func_ = write_func;
  busy_ = true;
}

#endif

void WriteController::Reset() {
  done_ = Register<uint32_t>(INT32_MAX);
}

#ifdef _DEBUG
void WriteController::Write() const {
  if (!clock_->IsRunning()) {
    return;
  }
  if (clock_->GetCycleCount() == done_.New() - 1) {
    write_func_();
  }
}
#else
void WriteController::Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                            RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) const {
  if (!clock_->IsRunning()) {
    return;
  }
  if (clock_->GetCycleCount() == done_.New() - 1) {
    write_func_(alu, decoder, iu, lsb, memory, rf, rb, rs);
  }
}
#endif

#ifdef _DEBUG
void WriteController::ForceWrite() const {
  write_func_();
}
#else
void WriteController::ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                                 RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) const {
  write_func_(alu, decoder, iu, lsb, memory, rf, rb, rs);
}
#endif

}