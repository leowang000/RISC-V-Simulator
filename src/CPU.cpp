#include <fstream>

#include "utils/NumberOperation.h"

#include "CPU.h"

namespace bubble {

CPU::CPU() :
    clock_(), bp_(), alu_(clock_), decoder_(clock_), iu_(clock_, bp_), lsb_(clock_), memory_(clock_), rf_(clock_), rb_(
    clock_, bp_), rs_(clock_) {}

void CPU::LoadMemory(const std::string &path) {
  std::ifstream in(path);
  memory_.Init(in);
}

void CPU::RunClock() {
  clock_.Run();
}

void CPU::Update() {
  alu_.Update();
  decoder_.Update();
  iu_.Update();
  lsb_.Update();
  memory_.Update();
  rf_.Update();
  rb_.Update();
  rs_.Update();
}

void CPU::Execute() {
  alu_.Execute(rb_, rs_);
  decoder_.Execute(iu_, lsb_, rb_, rs_);
  iu_.Execute(decoder_, lsb_, memory_, rb_, rs_);
  lsb_.Execute(alu_, decoder_, memory_, rb_, rf_, rs_);
  memory_.Execute(iu_, lsb_, rb_);
  rf_.Execute(decoder_, lsb_, rb_, rs_);
  rb_.Execute(alu_, decoder_, lsb_, memory_, rs_);
  rs_.Execute(alu_, decoder_, lsb_, memory_, rb_, rf_);
}

void CPU::Write() {
  alu_.Write();
  decoder_.Write();
  iu_.Write();
  lsb_.Write();
  memory_.Write();
  rf_.Write();
  rb_.Write();
  rs_.Write();
}

uint8_t CPU::Halt() {
  ForceWrite();
  clock_.Tick();
  Update();
  return GetSub(rf_.value_[10].GetCur(), 7, 0);
}

void CPU::ForceWrite() {
  alu_.ForceWrite();
  decoder_.ForceWrite();
  iu_.ForceWrite();
  lsb_.ForceWrite();
  memory_.ForceWrite();
  rf_.ForceWrite();
  rb_.ForceWrite();
  rs_.ForceWrite();
}

}