#include "ALU.h"
#include "Decoder.h"
#include "LoadStoreBuffer.h"
#include "Memory.h"
#include "ReorderBuffer.h"
#include "ReservationStation.h"

namespace bubble {

ReorderBuffer::ReorderBuffer(const Clock &clock, BranchPredictor &bp) :
    rb_(), to_rf_(), to_mem_(), flush_(), wc_(clock), bp_(&bp), halt_(false) {}

void ReorderBuffer::Debug(const Memory &memory, const ALU &alu) const {
  std::cout << "Reorder Buffer:\n";
  std::cout << "\trb_ = {\n";
  for (int i = rb_.GetCur().BeginId(); i != rb_.GetCur().EndId(); i = (i + 1) % (kRoBSize + 1)) {
    std::cout << "\t" << i << "\t" << rb_.GetCur()[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\trb_read = {\n";
  auto rb_queue = GetRB(memory, alu);
  for (int i = rb_queue.BeginId(); i != rb_queue.EndId(); i = (i + 1) % (kRoBSize + 1)) {
    std::cout << "\t" << i << "\t" << rb_queue[i].ToString() << "\n";
  }
  std::cout << "\t}\n";
  std::cout << "\tto_rf_ = " << to_rf_.GetCur().ToString() << "\n";
  std::cout << "\tto_mem_ = " << to_mem_.GetCur().ToString() << "\n";
  std::cout << "\tflush_ = " << flush_.GetCur().ToString() << "\n\n";
}

bool ReorderBuffer::IsFull() const {
  return rb_.GetCur().IsFull();
}

CircularQueue<RoBEntry, kRoBSize>
ReorderBuffer::GetRB(const Memory &memory, const ALU &alu) const {
  CircularQueue<RoBEntry, kRoBSize> res = rb_.GetCur();
  if (memory.output_.GetCur().done_) {
    res[memory.output_.GetCur().id_].val_ = memory.output_.GetCur().val_;
    res[memory.output_.GetCur().id_].done_ = true;
  }
  if (alu.output_.GetCur().done_) {
    if (res[alu.output_.GetCur().id_].inst_type_ != kJALR) {
      res[alu.output_.GetCur().id_].val_ = alu.output_.GetCur().val_;
    }
    res[alu.output_.GetCur().id_].done_ = true;
  }
  return res;
}

void ReorderBuffer::Update() {
  rb_.Update();
  to_rf_.Update();
  to_mem_.Update();
  flush_.Update();
  wc_.Update();
}

void ReorderBuffer::Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
                            const ReservationStation &rs) {
  if (wc_.IsBusy()) {
    return;
  }
  auto write_func = [this, is_mem_busy = memory.IsDataBusy(), from_mem = memory.output_.GetCur(),
      from_alu = alu.output_.GetCur(), from_decoder = decoder.output_.GetCur(), lsb_front = lsb.lsb_.GetCur().Front(),
      stall = decoder.IsStallNeeded(IsFull(), rs.IsFull(), lsb.IsFull())]() {
    if (flush_.GetCur().flush_) {
      Flush();
      return;
    }
    EnqueueInst(stall, from_decoder);
    UpdateDependencies(from_mem, from_alu, lsb_front);
    bool is_empty = rb_.GetCur().IsEmpty();
    bool is_front_store_inst = !is_empty &&
                               (rb_.GetCur().Front().inst_type_ == kSB || rb_.GetCur().Front().inst_type_ == kSH ||
                                rb_.GetCur().Front().inst_type_ == kSW);
    bool is_front_branch_inst = !is_empty &&
                                (rb_.GetCur().Front().inst_type_ == kBEQ || rb_.GetCur().Front().inst_type_ == kBNE ||
                                 rb_.GetCur().Front().inst_type_ == kBLT || rb_.GetCur().Front().inst_type_ == kBLTU ||
                                 rb_.GetCur().Front().inst_type_ == kBGE || rb_.GetCur().Front().inst_type_ == kBGEU);
    bool commit =
        !is_empty && rb_.GetCur().Front().done_ && !(is_front_store_inst && (is_mem_busy || to_mem_.GetCur().store_));
    WriteToRF(commit, is_front_store_inst || is_front_branch_inst, rb_.GetCur().Front());
    WriteToMem(commit, is_front_store_inst, rb_.GetCur().Front());
    if (commit) {
      halt_ = rb_.GetCur().Front().inst_type_ == kHALT;
      rb_.New().Dequeue();
    }
    bool flush = WriteFlush(commit, rb_.GetCur().Front());
    if (is_front_branch_inst && commit) {
      bp_->Update(rb_.GetCur().Front().addr_, rb_.GetCur().Front().val_, !flush);
    }
  };
  wc_.Set(write_func, 1);
}

void ReorderBuffer::Write() {
  wc_.Write();
}

void ReorderBuffer::ForceWrite() {
  wc_.ForceWrite();
}

void ReorderBuffer::Flush() {
  rb_.New().Clear();
  to_rf_.New().write_ = false;
  to_mem_.New().store_ = false;
  flush_.New().flush_ = false;
}

void ReorderBuffer::EnqueueInst(bool stall, const DecoderOutput &from_decoder) {
  if (stall || !from_decoder.get_inst_) {
    return;
  }
  RoBEntry rb_entry;
  rb_entry.inst_type_ = from_decoder.inst_type_;
  rb_entry.is_jump_predicted_ = from_decoder.is_jump_predicted_;
  rb_entry.rd_ = from_decoder.rd_;
  rb_entry.addr_ = from_decoder.addr_;
  switch (from_decoder.inst_type_) {
    case kLUI:
      rb_entry.done_ = true;
      rb_entry.val_ = from_decoder.imm_;
      break;
    case kAUIPC:
      rb_entry.done_ = true;
      rb_entry.val_ = from_decoder.addr_ + from_decoder.imm_;
      break;
    case kJAL:
      rb_entry.done_ = true;
      rb_entry.val_ = from_decoder.addr_ + 4;
      break;
    case kJALR:
      rb_entry.val_ = from_decoder.addr_ + 4;
      break;
    case kBEQ:
    case kBNE:
    case kBLT:
    case kBLTU:
    case kBGE:
    case kBGEU:
      rb_entry.dest_ = from_decoder.addr_ + from_decoder.imm_;
      break;
    case kHALT:
      rb_entry.done_ = true;
      break;
    default:
      break;
  }
  rb_.New().Enqueue(rb_entry);
}

void
ReorderBuffer::UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu, const LSBEntry &lsb_front) {
  if (from_mem.done_) {
    rb_.New()[from_mem.id_].val_ = from_mem.val_;
    rb_.New()[from_mem.id_].done_ = true;
  }
  if (from_alu.done_) {
    if (rb_.New()[from_alu.id_].inst_type_ == kJALR) {
      rb_.New()[from_alu.id_].dest_ = from_alu.val_;
    }
    else {
      rb_.New()[from_alu.id_].val_ = from_alu.val_;
    }
    rb_.New()[from_alu.id_].done_ = true;
  }
  if ((lsb_front.inst_type_ == kSB || lsb_front.inst_type_ == kSH || lsb_front.inst_type_ == kSW) &&
      lsb_front.Q1_ == -1 && lsb_front.Q2_ == -1) {
    rb_.New()[lsb_front.id_].dest_ = lsb_front.V1_;
    rb_.New()[lsb_front.id_].val_ = lsb_front.V2_;
    rb_.New()[lsb_front.id_].done_ = true;
  }
}

void ReorderBuffer::WriteToRF(bool commit, bool is_front_branch_or_store_inst, const RoBEntry &rb_entry) {
  if (!commit || is_front_branch_or_store_inst || rb_entry.inst_type_ == kHALT) {
    to_rf_.New().write_ = false;
    return;
  }
  to_rf_.Write(RobToRF(true, rb_entry.rd_, rb_entry.val_));
}

void ReorderBuffer::WriteToMem(bool commit, bool is_front_store_inst, const RoBEntry &rb_entry) {
  if (!(commit && is_front_store_inst)) {
    to_mem_.New().store_ = false;
    return;
  }
  to_mem_.Write(RobToMemory(true, rb_entry.inst_type_, rb_entry.dest_, rb_entry.val_));
}

bool ReorderBuffer::WriteFlush(bool commit, const RoBEntry &rb_entry) {
  if (!commit) {
    flush_.New().flush_ = false;
    return false;
  }
  bool flush;
  switch (rb_entry.inst_type_) {
    case kJALR:
      flush = true;
      break;
    case kBEQ:
    case kBNE:
    case kBLT:
    case kBGE:
    case kBLTU:
    case kBGEU:
      flush = rb_entry.is_jump_predicted_ == rb_entry.val_;
      break;
    default:
      flush = false;
  }
  flush_.Write(FlushInfo(flush, rb_entry.dest_));
  return flush;
}

}