#ifndef RISC_V_SIMULATOR_RESERVATIONSTATION_H
#define RISC_V_SIMULATOR_RESERVATIONSTATION_H

#include <array>

#include "utils/Register.h"

#include "Clock.h"
#include "config.h"
#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
class ALU;
class Decoder;
class LoadStoreBuffer;
class Memory;
class RegisterFile;
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

class ReservationStation {
 public:
  ReservationStation(const Clock &clock);

  void Debug() const;
  bool IsFull() const;
  void Update();
  void Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
               const ReorderBuffer &rb, const RegisterFile &rf);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<std::array<RSEntry, kRSSize>> rs_;
  Register<RSToALU> to_alu_;

 private:
  void Flush();
#ifdef _DEBUG
  void InsertInst(bool stall, const DecoderOutput &from_decoder, const std::array<uint32_t, kXLen> &reg_value,
                  const std::array<int, kXLen> &reg_status, const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
#else
  void InsertInst(bool stall, const DecoderOutput &from_decoder, const RegisterFile &rf, const ReorderBuffer &rb,
                  const Memory &memory, const ALU &alu);
#endif
  void UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu);
#ifdef _DEBUG
  int WriteToALU(const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
#else
  int WriteToALU(const ReorderBuffer &rb, const Memory &memory, const ALU &alu);
#endif

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_RESERVATIONSTATION_H
