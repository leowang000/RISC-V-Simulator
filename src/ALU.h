#ifndef RISC_V_SIMULATOR_ALU_H
#define RISC_V_SIMULATOR_ALU_H

#include "utils/WriteController.h"

#include "Clock.h"
#include "config.h"

namespace bubble {

#ifdef _DEBUG
class ReorderBuffer;
class ReservationStation;
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

class ALU {
 public:
  ALU(const Clock &clock);

  void Debug() const;
  void Update();
  void Execute(const ReorderBuffer &rb, const ReservationStation &rs);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<ALUOutput> output_;

 private:
  void Flush();
  void WriteOutput(const RSToALU &from_rs);

  WriteController wc_;
};


}

#endif //RISC_V_SIMULATOR_ALU_H
