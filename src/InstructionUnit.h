#ifndef RISC_V_SIMULATOR_INSTRUCTIONUNIT_H
#define RISC_V_SIMULATOR_INSTRUCTIONUNIT_H

#include <cstdint>

#include "utils/CircularQueue.h"
#include "utils/NumberOperation.h"
#include "utils/Register.h"
#include "utils/WriteController.h"

#include "BranchPredictor.h"
#include "Clock.h"
#include "config.h"

namespace bubble {

class Decoder;
class Memory;
class LoadStoreBuffer;
class ReorderBuffer;
class ReservationStation;

class InstructionUnit {
 public:
  InstructionUnit(const Clock &clock, const BranchPredictor &bp);

  void Debug() const;
  void Update();
  void Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory, const ReorderBuffer &rb,
               const ReservationStation &rs);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<uint32_t> pc_;
  Register<CircularQueue<InstQueueEntry, kInstQueueSize>> iq_;
  Register<IUToMemory> to_mem_;
  Register<IUToDecoder> to_decoder_;

 private:
  static constexpr bool IsBranchInst(uint32_t inst) {
    return (inst & 0b1111111) == 0b1100011;
  }

  static constexpr bool IsJAL(uint32_t inst) {
    return (inst & 0b1111111) == 0b1101111;
  }

  static constexpr uint32_t GetJumpOrBranchDest(uint32_t inst, uint32_t pc) {
    return pc + ((inst & 0b1111111) ^ 0b1100011 ?
                 SignExtend((GetSub(inst, 31, 31) << 20) | (GetSub(inst, 19, 12) << 12) | (GetSub(inst, 20, 20) << 11) |
                            (GetSub(inst, 30, 25) << 5) | (GetSub(inst, 24, 21) << 1), 20) :
                 SignExtend((GetSub(inst, 31, 31) << 12) | (GetSub(inst, 7, 7) << 11) | (GetSub(inst, 30, 25) << 5) |
                            (GetSub(inst, 11, 8) << 1), 12));
  }

  void Flush(uint32_t pc);
  void WriteToDecoder(bool dequeue);
  void WriteOthers(const MemoryToIU &from_mem);

  WriteController wc_;
  // Neglect next instruction if the current instruction loaded from memory is JAL, or is a branch instruction and that
  // the predictor predicts that it will jump.
  Register<bool> neglect_;
  const BranchPredictor *bp_;
};

}

#endif //RISC_V_SIMULATOR_INSTRUCTIONUNIT_H
