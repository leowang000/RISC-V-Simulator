#ifndef RISC_V_SIMULATOR_REORDERBUFFER_H
#define RISC_V_SIMULATOR_REORDERBUFFER_H

#include <array>
#include <cstdint>
#include <fstream>

#include "utils/CircularQueue.h"
#include "utils/Register.h"
#include "utils/WriteController.h"

#include "BranchPredictor.h"
#include "Clock.h"
#include "config.h"

namespace bubble {

class ALU;
class Decoder;
class LoadStoreBuffer;
class Memory;
class ReservationStation;

class ReorderBuffer {
 public:
  ReorderBuffer(const Clock &clock, BranchPredictor &bp);
  ReorderBuffer(const Clock &clock, BranchPredictor &bp, const std::string &pc_file_name,
                const std::string pc_with_cycle_file_name);

  void Debug(const Memory &memory, const ALU &alu) const;
  bool IsFull() const;
  CircularQueue<RoBEntry, kRoBSize> GetRB(const Memory &memory, const ALU &alu) const;
  void Update();
  void Execute(const ALU &alu, const Decoder &decoder, const LoadStoreBuffer &lsb, const Memory &memory,
               const ReservationStation &rs);
  void Write();
  void ForceWrite();

  Register<CircularQueue<RoBEntry, kRoBSize>> rb_;
  Register<RobToRF> to_rf_;
  Register<RobToMemory> to_mem_;
  Register<FlushInfo> flush_;
  bool halt_;

 private:
  void Flush();
  void EnqueueInst(bool stall, const DecoderOutput &from_decoder);
  void UpdateDependencies(const MemoryOutput &from_mem, const ALUOutput &from_alu, bool is_lsb_empty,
                          const LSBEntry &lsb_front);
  void WriteToRF(bool commit, bool is_front_branch_or_store_inst, const RoBEntry &rb_entry);
  void WriteToMem(bool commit, bool is_front_store_inst, const RoBEntry &rb_entry);
  bool WriteFlush(bool commit, const RoBEntry &rb_entry);

  WriteController wc_;
  BranchPredictor *bp_;
  std::ofstream pc_f_, pc_with_cycle_cnt_f_;
};

}

#endif //RISC_V_SIMULATOR_REORDERBUFFER_H
