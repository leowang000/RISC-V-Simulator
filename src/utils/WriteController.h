#ifndef RISC_V_SIMULATOR_WRITECONTROLLER_H
#define RISC_V_SIMULATOR_WRITECONTROLLER_H

#include <cstdint>
#include <functional>
#include <utility>

#include "Register.h"

#include "../Clock.h"

namespace bubble {

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
  void Set(const std::function<void()> &write_func, int cycle_cnt);
  void Reset();
  void Write() const;
  void ForceWrite() const;

 private:
  const Clock *clock_;
  Register<uint32_t> done_;
  std::function<void()> write_func_;
  bool busy_;
};

}

#endif //RISC_V_SIMULATOR_WRITECONTROLLER_H
