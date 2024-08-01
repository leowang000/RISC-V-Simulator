#include <iomanip>
#include <string>
#include <sstream>

#include "utils/NumberOperation.h"

#include "InstructionUnit.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "ReorderBuffer.h"

namespace bubble {

Memory::Memory(const Clock &clock) :
    output_(), to_iu_(), memory_(), wc_data_(clock), wc_inst_(clock), is_load_(false) {}

void Memory::Debug() const {
  std::cout << "Memory:\n";
  std::cout << "\tto_iu_ = " << to_iu_.GetCur() << "\n";
  std::cout << "\toutput_ = " << output_.GetCur().ToString() << "\n\n";
}

void Memory::Init(std::istream &in) {
  std::string str;
  uint32_t now = 0;
  while (std::getline(in, str)) {
    if (str.empty()) {
      continue;
    }
    if (str.front() == '@') {
      now = std::stoi(str.substr(1), nullptr, 16);
      continue;
    }
    std::istringstream is(str);
    is >> std::hex;
    int num;
    while (is >> num) {
      StoreByte(now++, num);
    }
  }
}

bool Memory::IsDataBusy() const {
  return wc_data_.IsBusy();
}

bool Memory::IsInstReady() const {
  return wc_inst_.IsReady();
}

void Memory::Update() {
  output_.Update();
  to_iu_.Update();
  wc_data_.Update();
  wc_inst_.Update();
}

void Memory::Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb) {
  if (!wc_inst_.IsBusy() && iu.to_mem_.GetCur()) {
    auto write_func = [this, pc = iu.pc_.GetCur()]() {
      to_iu_.Write(LoadWord(pc));
    };
    wc_inst_.Set(write_func, 1);
  }
  if (wc_data_.IsBusy()) {
    if (rb.flush_.GetCur().flush_ && is_load_) {
      wc_data_.Reset();
      Flush();
      is_load_ = false;
    }
  }
  else {
    if (lsb.to_mem_.GetCur().load_ || rb.to_mem_.GetCur().store_) {
      auto write_func = [this, flush = rb.flush_.GetCur().flush_, from_lsb = lsb.to_mem_.GetCur(),
          from_rb = rb.to_mem_.GetCur()]() {
        WriteOutput(from_lsb, from_rb);
        is_load_ = false;
      };
      wc_data_.Set(write_func, 3);
      is_load_ = lsb.to_mem_.GetCur().load_;
    }
    else {
      output_.New().done_ = false;
    }
  }
}

void Memory::Write() {
  wc_data_.Write();
  wc_inst_.Write();
}

void Memory::ForceWrite() {
  wc_data_.ForceWrite();
}

uint32_t Memory::LoadWord(uint32_t addr) const {
  return (static_cast<uint32_t>(LoadByte(addr + 3)) << 24) | (static_cast<uint32_t>(LoadByte(addr + 2)) << 16) |
         (static_cast<uint32_t>(LoadByte(addr + 1)) << 8) | static_cast<uint32_t>(LoadByte(addr));
}

uint16_t Memory::LoadHalf(uint32_t addr) const {
  return (static_cast<uint16_t>(LoadByte(addr + 1)) << 8) | static_cast<uint16_t>(LoadByte(addr));
}

uint8_t Memory::LoadByte(uint32_t addr) const {
  if (memory_.count(addr / PageSize) == 0) {
    return 0;
  }
  return memory_.at(addr / PageSize)[addr % PageSize];
}

void Memory::StoreWord(uint32_t addr, uint32_t num) {
  StoreByte(addr, static_cast<uint8_t>(GetSub(num, 7, 0)));
  StoreByte(addr + 1, static_cast<uint8_t>(GetSub(num, 15, 8)));
  StoreByte(addr + 2, static_cast<uint8_t>(GetSub(num, 23, 16)));
  StoreByte(addr + 3, static_cast<uint8_t>(GetSub(num, 31, 24)));
}

void Memory::StoreHalf(uint32_t addr, uint16_t num) {
  StoreByte(addr, static_cast<uint8_t>(GetSub(num, 7, 0)));
  StoreByte(addr + 1, static_cast<uint8_t>(GetSub(num, 15, 8)));
}

void Memory::StoreByte(uint32_t addr, uint8_t num) {
  memory_[addr / PageSize][addr % PageSize] = num;
}

void Memory::Flush() {
  output_.New().done_ = false;
}

void Memory::WriteOutput(const LSBToMemory &from_lsb, const RobToMemory &from_rb) {
  output_.New().done_ = false;
  output_.New().id_ = from_lsb.id_;
  if (from_lsb.load_) {
    switch (from_lsb.inst_type_) {
      case kLB:
        output_.New().val_ = SignExtend(LoadByte(from_lsb.load_addr_), 7);
        break;
      case kLH:
        output_.New().val_ = SignExtend(LoadHalf(from_lsb.load_addr_), 15);
        break;
      case kLW:
        output_.New().val_ = LoadWord(from_lsb.load_addr_);
        break;
      case kLBU:
        output_.New().val_ = LoadByte(from_lsb.load_addr_);
        break;
      case kLHU:
        output_.New().val_ = LoadHalf(from_lsb.load_addr_);
        break;
      default:
        return;
    }
  }
  if (from_rb.store_) {
    switch (from_rb.inst_type_) {
      case kSB:
        StoreByte(from_rb.store_addr_, GetSub(from_rb.val_, 7, 0));
        break;
      case kSH:
        StoreHalf(from_rb.store_addr_, GetSub(from_rb.val_, 15, 0));
        break;
      case kSW:
        StoreWord(from_rb.store_addr_, from_rb.val_);
        break;
      default:
        return;
    }
  }
  output_.New().done_ = true;
  return;
}

}