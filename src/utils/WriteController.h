#undef _DEBUG

#ifndef RISC_V_SIMULATOR_WRITECONTROLLER_H
#define RISC_V_SIMULATOR_WRITECONTROLLER_H

#include <cstdint>
#include <functional>
#include <utility>

#include "Register.h"

#include "../Clock.h"

namespace bubble {

#ifndef _DEBUG
class ALU;
class Decoder;
class InstructionUnit;
class LoadStoreBuffer;
class Memory;
class RegisterFile;
class ReorderBuffer;
class ReservationStation;
#endif

/*
 * Use WriteController to control when to write and whether the output is ready.
 * At the beginning of each cycle, use Update() to update the controller.
 * During the first cycle of each execution, use Set(write_func_, cycle_cnt_) to simulate the process that the execution
 * will last cycle_cnt_ cycles and behave like write_func_. Function write_func should capture all inputs by value.
 * At the end of each cycle, use Write() to write the output.
 */
class WriteController {
 public:
  WriteController(const Clock &clock);

  void Update();
  bool IsReady() const;
  bool IsBusy() const;
#ifdef _DEBUG
  void Set(const std::function<void()> &write_func, int cycle_cnt);
  void Write() const;
  void ForceWrite() const;
#else
  void Set(void (*write_func)(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                              RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs), int cycle_cnt);
  void Write(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
             RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) const;
  void ForceWrite(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                  RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs) const;
#endif
  void Reset();

 private:
 public:
  const Clock *clock_;
 private:
  Register<uint32_t> done_;
#ifdef _DEBUG
  std::function<void()> write_func_;
#else
  void (*write_func_)(ALU &alu, Decoder &decoder, InstructionUnit &iu, LoadStoreBuffer &lsb, Memory &memory,
                      RegisterFile &rf, ReorderBuffer &rb, ReservationStation &rs);
#endif
  bool busy_;
};

}

#endif //RISC_V_SIMULATOR_WRITECONTROLLER_H
