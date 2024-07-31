#ifndef RISC_V_SIMULATOR_LOADSTOREBUFFER_H
#define RISC_V_SIMULATOR_LOADSTOREBUFFER_H

#include <array>

#include "utils/CircularQueue.h"
#include "utils/Register.h"
#include "utils/WriteController.h"

#include "Clock.h"
#include "TypeDef.h"

namespace bubble {

class ALU;
class Decoder;
class Memory;
class RegisterFile;
class ReorderBuffer;
class ReservationStation;

class LoadStoreBuffer {
 public:
  LoadStoreBuffer(const Clock &clock);

  bool IsFull() const;
  void Update();
  void Execute(const ALU &alu, const Decoder &decoder, const Memory &memory, const ReorderBuffer &rb,
               const RegisterFile &rf, const ReservationStation &rs);
  void Write();
  void ForceWrite();

  Register<CircularQueue<LSBEntry, kLSBSize>> lsb_;
  Register<LSBToMemory> to_mem_;

 private:
  void Flush();
  void EnqueueInst(bool stall, bool is_store_or_load_inst, const DecoderOutput &from_decoder,
                   const std::array<uint32_t, kXLen> &reg_value, const std::array<int, kXLen> &reg_status,
                   const CircularQueue<RoBEntry, kRoBSize> &rb_queue);
  void UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu);
  bool WriteToToMemory(bool is_load_inst, bool is_mem_busy);

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_LOADSTOREBUFFER_H
