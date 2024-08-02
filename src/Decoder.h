#ifndef RISC_V_SIMULATOR_DECODER_H
#define RISC_V_SIMULATOR_DECODER_H

#include <cstdint>

#include "utils/Register.h"
#include "utils/WriteController.h"

#include "Clock.h"
#include "config.h"

namespace bubble {

class InstructionUnit;
class LoadStoreBuffer;
class ReorderBuffer;
class ReservationStation;

class Decoder {
 public:
  Decoder(const Clock &clock);

  void Debug() const;
  bool IsStallNeeded(bool is_rb_full, bool is_rs_full, bool is_lsb_full) const;
  void Update();
  void
  Execute(const InstructionUnit &iu, const LoadStoreBuffer &lsb, const ReorderBuffer &rb, const ReservationStation &rs);
  void Write();
  void ForceWrite();

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
