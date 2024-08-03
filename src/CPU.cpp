#include <cassert>
#include <fstream>
#include <algorithm>

#include "utils/NumberOperation.h"

#include "CPU.h"

namespace bubble {

CPU::CPU() :
    clock_(), bp_(), alu_(clock_), decoder_(clock_), iu_(clock_, bp_), lsb_(clock_), memory_(clock_), rf_(clock_), rb_(
    clock_, bp_), rs_(clock_) {}

CPU::CPU(const std::string &pc_file_name, const std::string pc_with_cycle_file_name) :
    clock_(), bp_(), alu_(clock_), decoder_(clock_), iu_(clock_, bp_), lsb_(clock_), memory_(clock_), rf_(clock_), rb_(
    clock_, bp_, pc_file_name, pc_with_cycle_file_name), rs_(clock_) {}

void CPU::Debug() {
  if (clock_.GetCycleCount() >= 100000) {
    return;
  }
  clock_.Debug();
  iu_.Debug();
  decoder_.Debug();
  memory_.Debug();
  alu_.Debug();
  rf_.Debug(rb_);
  rb_.Debug(memory_, alu_);
  rs_.Debug();
  lsb_.Debug();
  std::cout << "----------------------------------------------------------------------------------------------------\n";
}

void CPU::LoadMemory(const std::string &path) {
  std::ifstream in(path);
  memory_.Init(in);
}

void CPU::LoadMemory() {
  memory_.Init(std::cin);
}

void CPU::Update() {
  static int order[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  std::random_shuffle(order, order + 8);
  for (auto i : order) {
    switch (i) {
      case 0:
        alu_.Update();
        break;
      case 1:
        decoder_.Update();
        break;
      case 2:
        iu_.Update();
        break;
      case 3:
        lsb_.Update();
        break;
      case 4:
        memory_.Update();
        break;
      case 5:
        rf_.Update();
        break;
      case 6:
        rb_.Update();
        break;
      case 7:
        rs_.Update();
        break;
    }
  }
}

void CPU::Execute() {
  static int order[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  std::random_shuffle(order, order + 8);
  for (auto i : order) {
    switch (i) {
      case 0:
        alu_.Execute(rb_, rs_);
        break;
      case 1:
        decoder_.Execute(iu_, lsb_, rb_, rs_);
        break;
      case 2:
        iu_.Execute(decoder_, lsb_, memory_, rb_, rs_);
        break;
      case 3:
        lsb_.Execute(alu_, decoder_, memory_, rb_, rf_, rs_);
        break;
      case 4:
        memory_.Execute(iu_, lsb_, rb_);
        break;
      case 5:
        rf_.Execute(decoder_, lsb_, rb_, rs_);
        break;
      case 6:
        rb_.Execute(alu_, decoder_, lsb_, memory_, rs_);
        break;
      case 7:
        rs_.Execute(alu_, decoder_, lsb_, memory_, rb_, rf_);
        break;
    }
  }
}

void CPU::Write() {
  static int order[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  std::random_shuffle(order, order + 8);
#ifdef _DEBUG
  for (auto i : order) {
    switch (i) {
      case 0:
        alu_.Write();
        break;
      case 1:
        decoder_.Write();
        break;
      case 2:
        iu_.Write();
        break;
      case 3:
        lsb_.Write();
        break;
      case 4:
        memory_.Write();
        break;
      case 5:
        rf_.Write();
        break;
      case 6:
        rb_.Write();
        break;
      case 7:
        rs_.Write();
        break;
    }
  }
#else
  for (auto i : order) {
    switch (i) {
      case 0:
        alu_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 1:
        decoder_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 2:
        iu_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 3:
        lsb_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 4:
        memory_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 5:
        rf_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 6:
        rb_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
      case 7:
        rs_.Write(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
        break;
    }
  }
#endif
}

bool CPU::ShouldHalt() const {
  return rb_.halt_;
}

uint32_t CPU::Halt() {
#ifdef _DEBUG
  memory_.ForceWrite();
  rf_.ForceWrite();
#else
  memory_.ForceWrite(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
  rf_.ForceWrite(alu_, decoder_, iu_, lsb_, memory_, rf_, rb_, rs_);
#endif
  memory_.Update();
  rf_.Update();
  return GetSub(rf_.value_[10].GetCur(), 7, 0);
}

}