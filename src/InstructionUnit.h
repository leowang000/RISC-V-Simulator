#ifndef RISC_V_SIMULATOR_INSTRUCTIONUNIT_H
#define RISC_V_SIMULATOR_INSTRUCTIONUNIT_H

#include <cstdint>

#include "utils/CircularQueue.h"
#include "utils/NumberOperation.h"
#include "utils/Register.h"
#include "utils/WriteController.h"

#include "BranchPredictor.h"
#include "Clock.h"
#include "TypeDef.h"

namespace bubble {

class Decoder;
class Memory;
class LoadStoreBuffer;
class ReorderBuffer;
class ReservationStation;

class InstructionUnit {
 public:
  InstructionUnit(const Clock &clock, const BranchPredictor &bp);

  void Update();
  void Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory, const ReorderBuffer &rb,
               const ReservationStation &rs);
  void Write();
  void ForceWrite();

  Register<uint32_t> pc_;
  Register<CircularQueue<InstQueueEntry, kInstQueueSize>> iq_;
  Register<bool> to_mem_;
  Register<IUToDecoder> to_decoder_;

 private:
  // Return whether the instruction is J-type or B-type (JAL and all branch instructions).
  // We don't predict JALR since we don't know the destination of the jump.
  static constexpr bool IsJumpInst(uint32_t inst) {
    return (inst & 0b1111111) == 0b1101111 || (inst & 0b1111111) == 0b1100011;
  }

  static constexpr uint32_t GetJumpDest(uint32_t inst, uint32_t pc) {
    return pc + ((inst & 0b1111111) ^ 0b1100011 ?
                 SignExtend((GetSub(inst, 31, 31) << 20) | (GetSub(inst, 19, 12) << 12) | (GetSub(inst, 20, 20) << 11) |
                            (GetSub(inst, 30, 25) << 5) | (GetSub(inst, 24, 21) << 1), 20) :
                 SignExtend((GetSub(inst, 31, 31) << 12) | (GetSub(inst, 7, 7) << 11) | (GetSub(inst, 30, 25) << 5) |
                            (GetSub(inst, 11, 8) << 1), 12));
  }

  void Flush(uint32_t pc, bool is_memory_ready);
  void WriteToDecoder(bool dequeue);
  void WriteToMemory(bool can_enqueue, bool is_mem_inst_ready, uint32_t inst);

  WriteController wc_;
  // Neglect next instruction if the current instruction loaded from memory is a branch instruction and the predictor
  // predicts that it will jump.
  Register<bool> neglect_;
  const BranchPredictor *bp_;
};

}

#endif //RISC_V_SIMULATOR_INSTRUCTIONUNIT_H
