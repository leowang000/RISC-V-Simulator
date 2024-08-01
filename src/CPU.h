#ifndef RISC_V_SIMULATOR_CPU_H
#define RISC_V_SIMULATOR_CPU_H

#include <cstdint>
#include <string>

#include "ALU.h"
#include "BranchPredictor.h"
#include "Clock.h"
#include "Decoder.h"
#include "InstructionUnit.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "RegisterFile.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

class CPU {
 public:
  CPU();

  void Debug();
  void LoadMemory(const std::string &path);
  void Update();
  void Execute();
  void Write();
  bool ShouldHalt() const;
  uint32_t Halt();

  Clock clock_;
  BranchPredictor bp_;
  ALU alu_;
  Decoder decoder_;
  InstructionUnit iu_;
  LoadStoreBuffer lsb_;
  Memory memory_;
  RegisterFile rf_;
  ReorderBuffer rb_;
  ReservationStation rs_;

 private:
  void ForceWrite();
};

}

#endif //RISC_V_SIMULATOR_CPU_H
