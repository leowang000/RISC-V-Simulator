#ifndef RISC_V_SIMULATOR_RESERVATIONSTATION_H
#define RISC_V_SIMULATOR_RESERVATIONSTATION_H

#include <array>

#include "utils/Register.h"
#include "utils/WriteController.h"

#include "Clock.h"
#include "config.h"

namespace bubble {

class ALU;
class Decoder;
class LoadStoreBuffer;
class Memory;
class RegisterFile;
class ReorderBuffer;

class ReservationStation {
 public:
  ReservationStation(const Clock &clock);

  void Debug() const;
  bool IsFull() const;
  void Update();
  void Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
               const ReorderBuffer &rb, const RegisterFile &rf);
  void Write();
  void ForceWrite();

  Register<std::array<RSEntry, kRSSize>> rs_;
  Register<RSToALU> to_alu_;

 private:
  void Flush();
  void InsertInst(bool stall, const DecoderOutput &from_decoder, const std::array<uint32_t, kXLen> &reg_value,
                  const std::array<int, kXLen> &reg_status, const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
  void InsertInst(bool stall, const DecoderOutput &from_decoder, const RegisterFile &rf, const ReorderBuffer &rb,
                  const Memory &memory, const ALU &alu);
  void UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu);
  int WriteToALU(const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
  int WriteToALU(const ReorderBuffer &rb, const Memory &memory, const ALU &alu);

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_RESERVATIONSTATION_H
