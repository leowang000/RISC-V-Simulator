#ifndef RISC_V_SIMULATOR_MEMORY_H
#define RISC_V_SIMULATOR_MEMORY_H

#include <array>
#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "utils/Register.h"
#include "utils/WriteController.h"

#include "Clock.h"
#include "TypeDef.h"

namespace bubble {

class InstructionUnit;
class LoadStoreBuffer;
class ReorderBuffer;

class Memory {
 public:
  explicit Memory(const Clock &clock);

  void Init(std::istream &in);
  bool IsDataReady() const;
  bool IsDataBusy() const;
  bool IsInstReady() const;
  bool IsInstBusy() const;
  void Update();
  void Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb);
  void Write();
  void ForceWrite();

  Register<uint32_t> to_iu_;
  Register<MemoryOutput> output_;

 private:
  static constexpr int PageSize = 4096;

  uint32_t LoadWord(uint32_t addr) const;
  uint16_t LoadHalf(uint32_t addr) const;
  uint8_t LoadByte(uint32_t addr) const;
  void StoreWord(uint32_t addr, uint32_t num);
  void StoreHalf(uint32_t addr, uint16_t num);
  void StoreByte(uint32_t addr, uint8_t num);
  void WriteToOutput(const LSBToMemory &from_lsb, const RobToMemory &from_rb);

  std::unordered_map<uint32_t, std::array<uint8_t, PageSize>> memory_;
  WriteController wc_data_, wc_inst_;
};

}

#endif //RISC_V_SIMULATOR_MEMORY_H
