#include "Decoder.h"
#include "InstructionUnit.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

InstructionUnit::InstructionUnit(const Clock &clock, const BranchPredictor &bp) :
    pc_(), iq_(), to_mem_(true), to_decoder_(), neglect_(), wc_(clock), bp_(&bp) {}

void InstructionUnit::Debug() const {
  std::cout << "Instruction Unit:\n";
  std::cout << "\tpc_ = " << pc_.GetCur() << "\n";
  std::cout << "\tiq_ = {\n";
  for (int i = iq_.GetCur().BeginId(); i != iq_.GetCur().EndId(); i = (i + 1) % (kInstQueueSize + 1)) {
    std::cout << "\t" << i << "\t" << iq_.GetCur()[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\tto_mem_ = " << to_mem_.GetCur() << "\n";
  std::cout << "\tto_decoder_ = " << to_decoder_.GetCur().ToString() << "\n\n";
}

void InstructionUnit::Update() {
  pc_.Update();
  iq_.Update();
  to_mem_.Update();
  to_decoder_.Update();
  neglect_.Update();
  wc_.Update();
}

void InstructionUnit::Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                              const ReorderBuffer &rb, const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, flush_info = rb.flush_.GetCur(), is_mem_inst_ready = memory.IsInstReady(),
      stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), lsb.IsFull()), inst = memory.to_iu_.GetCur()]() {
    bool flush = flush_info.flush_;
    if (flush) {
      Flush(flush_info.pc_, is_mem_inst_ready);
      return;
    }
    bool dequeue = !stall && !iq_.GetCur().IsEmpty();
    if (!stall) {
      WriteToDecoder(dequeue);
    }
    bool can_enqueue = !iq_.GetCur().IsFull() || dequeue;
    WriteToMemoryAndPCAndNeglect(can_enqueue, is_mem_inst_ready, inst);
  };
  wc_.Set(write_func, 1);
}

void InstructionUnit::Write() {
  wc_.Write();
}

void InstructionUnit::ForceWrite() {
  wc_.ForceWrite();
}

void InstructionUnit::Flush(uint32_t pc, bool is_memory_ready) {
  pc_.Write(pc);
  iq_.New().Clear();
  to_mem_.Write(true);
  to_decoder_.New().get_inst_ = false;
  if (is_memory_ready) {
    neglect_.Write(true);
  }
}

void InstructionUnit::WriteToDecoder(bool dequeue) {
  if (dequeue) {
    const InstQueueEntry &back = iq_.GetCur().Back();
    to_decoder_.Write(IUToDecoder(true, back.inst_, back.addr_, back.jump_));
    iq_.New().Dequeue();
  }
  else {
    to_decoder_.New().get_inst_ = false;
  }
}

void InstructionUnit::WriteToMemoryAndPCAndNeglect(bool can_enqueue, bool is_mem_inst_ready, uint32_t inst) {
  to_mem_.Write(can_enqueue);
  if (can_enqueue) {
    if (IsJAL(inst) || (IsBranchInst(inst) && bp_->Predict(pc_.GetCur() - 4))) {
      pc_.Write(GetJumpOrBranchDest(inst, pc_.GetCur() - 4));
    }
    else {
      pc_.Write(pc_.GetCur() + 4);
    }
  }
  if (is_mem_inst_ready && can_enqueue) {
    if (neglect_.GetCur()) {
      neglect_.Write(false);
    }
    else {
      if (IsBranchInst(inst)) {
        iq_.New().Enqueue(InstQueueEntry(inst, pc_.GetCur() - 4, bp_->Predict(pc_.GetCur() - 4)));
      }
      else {
        iq_.New().Enqueue(InstQueueEntry(inst, pc_.GetCur() - 4, false));
      }
      neglect_.Write((IsJAL(inst) || (IsBranchInst(inst) && bp_->Predict(pc_.GetCur() - 4))));
    }
  }
}

}