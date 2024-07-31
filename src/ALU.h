#ifndef RISC_V_SIMULATOR_ALU_H
#define RISC_V_SIMULATOR_ALU_H

#include "utils/WriteController.h"

#include "Clock.h"
#include "TypeDef.h"

namespace bubble {

class ReorderBuffer;
class ReservationStation;

class ALU {
 public:
  ALU(const Clock &clock);

  void Update();
  void Execute(const ReorderBuffer &rb, const ReservationStation &rs);
  void Write();
  void ForceWrite();

  Register<ALUOutput> output_;

 private:
  void Flush();
  void WriteToOutput(const RSToALU &from_rs);

  WriteController wc_;
};


}

#endif //RISC_V_SIMULATOR_ALU_H
