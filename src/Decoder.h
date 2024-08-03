#ifndef RISC_V_SIMULATOR_DECODER_H
#define RISC_V_SIMULATOR_DECODER_H

#include <cstdint>

#include "utils/Register.h"

#include "Clock.h"
#include "config.h"
#include "WriteController.h"

namespace bubble {

#ifdef _DEBUG
class InstructionUnit;
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

class Decoder {
 public:
  Decoder(const Clock &clock);

  void Debug() const;
  bool IsStallNeeded(bool is_rb_full, bool is_rs_full, bool is_lsb_full) const;
  void Update();
  void
  Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb, const ReservationStation &rs);
#ifdef _DEBUG
  void Write();
  void ForceWrite();
#else
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif

  Register<DecoderOutput> output_;

 private:
  static bool GetOperands(DecoderOutput &out, uint32_t inst);
  static bool GetInstType(DecoderOutput &out, uint32_t inst);
  void Flush();
  void WriteOutput(const IUToDecoder &from_iu);

  WriteController wc_;
};

}

#endif //RISC_V_SIMULATOR_DECODER_H
