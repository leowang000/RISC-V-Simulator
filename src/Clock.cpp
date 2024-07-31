#include "Clock.h"

namespace bubble {

Clock::Clock() : cycle_(), is_running_(false) {}

void Clock::Run() {
  is_running_ = true;
}

void Clock::Stop() {
  is_running_ = false;
}

void Clock::Reset() {
  is_running_ = false;
  cycle_ = 0;
}

bool Clock::IsRunning() const {
  return is_running_;
}

void Clock::Tick() {
  if (!is_running_) {
    return;
  }
  cycle_++;
}

uint32_t Clock::GetCycleCount() const {
  return cycle_;
}


}