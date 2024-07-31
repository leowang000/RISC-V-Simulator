#ifndef RISC_V_SIMULATOR_CLOCK_H
#define RISC_V_SIMULATOR_CLOCK_H

#include <cstdint>

namespace bubble {

class Clock {
 public:
  Clock();

  void Run();
  void Stop();
  void Reset();
  bool IsRunning() const;
  void Tick();
  uint32_t GetCycleCount() const;

 private:
  uint32_t cycle_;
  bool is_running_;
};

}

#endif //RISC_V_SIMULATOR_CLOCK_H
