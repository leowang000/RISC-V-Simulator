#ifndef RISC_V_SIMULATOR_MEMORY_H
#define RISC_V_SIMULATOR_MEMORY_H

#include <array>
#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "utils/Register.h"

#include "Clock.h"
#include "config.h"
#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
class InstructionUnit;
class LoadStoreBuffer;
class ReorderBuffer;
#else
class ALU;
class Decoder;
class InstructionUnit;
class LoadStoreBuffer;
class Memory;
class RegisterFile;
class ReorderBuffer;
class ReservationStation;
#endif

class Memory {
 public:
  explicit Memory(const Clock &clock);

  void Debug() const;
  void Init(std::istream &in);
  bool IsDataBusy() const;
  bool IsInstReady() const;
  void Update();
  void Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<MemoryToIU> to_iu_;
  Register<MemoryOutput> output_;

 private:
  static constexpr int PageSize = 4096;

  uint32_t LoadWord(uint32_t addr) const;
  uint16_t LoadHalf(uint32_t addr) const;
  uint8_t LoadByte(uint32_t addr) const;
  void StoreWord(uint32_t addr, uint32_t num);
  void StoreHalf(uint32_t addr, uint16_t num);
  void StoreByte(uint32_t addr, uint8_t num);
  void Flush();
  void WriteOutput(const LSBToMemory &from_lsb, const RobToMemory &from_rb);

  std::unordered_map<uint32_t, std::array<uint8_t, PageSize>> memory_;
  WriteController wc_data_, wc_inst_;
  bool is_load_;
#ifndef _DEBUG
  LSBToMemory from_lsb_;
  RobToMemory from_rb_;
#endif
};

}

#endif //RISC_V_SIMULATOR_MEMORY_H
