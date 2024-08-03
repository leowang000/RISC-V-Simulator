#include "Decoder.h"
#include "InstructionUnit.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

InstructionUnit::InstructionUnit(const Clock &clock, const BranchPredictor &bp) :
    pc_(), iq_(), to_mem_(), to_decoder_(), neglect_(), wc_(clock), bp_(&bp) {}

void InstructionUnit::Debug() const {
  std::cout << "Instruction Unit:\n";
  std::cout << "\tpc_ = " << pc_.GetCur() << "\n";
  std::cout << "\tiq_ = {\n";
  for (int i = iq_.GetCur().BeginId(); i != iq_.GetCur().EndId(); i = (i + 1) % (kInstQueueSize + 1)) {
    std::cout << "\t" << i << "\t" << iq_.GetCur()[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\tto_mem_ = " << to_mem_.GetCur().ToString() << "\n";
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

#ifdef _DEBUG
void InstructionUnit::Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                              const ReorderBuffer &rb, const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, flush_info = rb.flush_.GetCur(), from_mem = memory.to_iu_.GetCur(),
      stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), lsb.IsFull())]() {
    if (flush_info.flush_) {
      Flush(flush_info.pc_);
      return;
    }
    bool dequeue = !stall && !iq_.GetCur().IsEmpty();
    if (!stall) {
      WriteToDecoder(dequeue);
    }
    WriteOthers(from_mem);
  };
  wc_.Set(write_func, 1);
}
#else
void InstructionUnit::Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                              const ReorderBuffer &rb, const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, &rb, &memory, stall = decoder.IsStallNeeded(rb.IsFull(), rs.IsFull(), lsb.IsFull())]() {
    if (rb.flush_.GetCur().flush_) {
      Flush(rb.flush_.GetCur().pc_);
      return;
    }
    bool dequeue = !stall && !iq_.GetCur().IsEmpty();
    if (!stall) {
      WriteToDecoder(dequeue);
    }
    WriteOthers(memory.to_iu_.GetCur());
  };
  wc_.Set(write_func, 1);
}
#endif

void InstructionUnit::Write() {
  wc_.Write();
}

void InstructionUnit::ForceWrite() {
  wc_.ForceWrite();
}

void InstructionUnit::Flush(uint32_t pc) {
  pc_.Write(pc);
  iq_.New().Clear();
  to_mem_.Write(IUToMemory(false, 0));
  to_decoder_.New().get_inst_ = false;
  neglect_.Write(false);
}

void InstructionUnit::WriteToDecoder(bool dequeue) {
  if (dequeue) {
    const InstQueueEntry &front = iq_.GetCur().Front();
    to_decoder_.Write(IUToDecoder(true, front.inst_, front.addr_, front.jump_));
    iq_.New().Dequeue();
  }
  else {
    to_decoder_.New().get_inst_ = false;
  }
}

void InstructionUnit::WriteOthers(const MemoryToIU &from_mem) {
  if (neglect_.GetCur()) {
    to_mem_.Write(IUToMemory());
    neglect_.Write(false);
    return;
  }
  bool load_from_mem = (iq_.GetCur().Size() <= kInstQueueSize - 3);
  if (load_from_mem) {
    pc_.Write(pc_.GetCur() + 4);
  }
  to_mem_.Write(IUToMemory(load_from_mem, pc_.GetCur()));
  if (from_mem.inst_ != 0) {
    bool jump = IsJAL(from_mem.inst_) || (IsBranchInst(from_mem.inst_) && bp_->Predict(from_mem.pc_));
    iq_.New().Enqueue(InstQueueEntry(from_mem.inst_, from_mem.pc_, jump));
    if (jump) {
      pc_.Write(GetJumpOrBranchDest(from_mem.inst_, from_mem.pc_));
      to_mem_.Write(IUToMemory());
      neglect_.Write(true);
    }
  }
}

}