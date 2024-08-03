#ifndef RISC_V_SIMULATOR_REGISTERFILE_H
#define RISC_V_SIMULATOR_REGISTERFILE_H

#include <array>
#include <cstdint>

#include "utils/Register.h"

#include "Clock.h"
#include "config.h"
#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
class Decoder;
class LoadStoreBuffer;
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

class RegisterFile {
 public:
  RegisterFile(const Clock &clock);

  void Debug(const ReorderBuffer &rb) const;
  std::array<uint32_t, kXLen> GetRegisterValue(const ReorderBuffer &rb) const;
  uint32_t GetRegisterValue(uint8_t i, const ReorderBuffer &rb) const;
  std::array<int, kXLen> GetRegisterStatus(const ReorderBuffer &rb) const;
  int GetRegisterStatus(uint8_t i, const ReorderBuffer &rb) const;
  void Update();
  void
  Execute(const Decoder &decoder, const LoadStoreBuffer &lsb, const ReorderBuffer &rb, const ReservationStation &rs);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<uint32_t> value_[kXLen];
  Register<int> status_[kXLen];

 private:
  void Flush();
  void AddDependency(bool stall, const DecoderOutput &from_decoder, int rob_new_inst_id);
  void RemoveDependencyAndWrite(const RobToRF &from_rb, int rob_commit_inst_id);

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_REGISTERFILE_H
