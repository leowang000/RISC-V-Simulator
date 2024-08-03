#ifndef RISC_V_SIMULATOR_LOADSTOREBUFFER_H
#define RISC_V_SIMULATOR_LOADSTOREBUFFER_H

#include <array>

#include "utils/CircularQueue.h"
#include "utils/Register.h"

#include "Clock.h"
#include "config.h"
#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
class ALU;
class Decoder;
class Memory;
class RegisterFile;
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

class LoadStoreBuffer {
 public:
  LoadStoreBuffer(const Clock &clock);

  void Debug() const;
  bool IsFull() const;
  void Update();
  void Execute(const ALU &alu, const Decoder &decoder, const Memory &memory, const ReorderBuffer &rb,
               const RegisterFile &rf, const ReservationStation &rs);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<CircularQueue<LSBEntry, kLSBSize>> lsb_;
  Register<LSBToMemory> to_mem_;

 private:
  void Flush();
  void EnqueueInst(bool stall, bool is_new_inst_store, bool is_new_inst_load, const DecoderOutput &from_decoder,
                   const std::array<uint32_t, kXLen> &reg_value, const std::array<int, kXLen> &reg_status,
                   const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
  void EnqueueInst(bool stall, bool is_new_inst_store, bool is_new_inst_load, const DecoderOutput &from_decoder,
                   const RegisterFile &rf, const ReorderBuffer &rb, const Memory &memory, const ALU &alu);
  void UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu);
  bool WriteToMemory(bool is_front_load, bool is_mem_busy);

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_LOADSTOREBUFFER_H
