#ifndef RISC_V_SIMULATOR_CLOCK_H
#define RISC_V_SIMULATOR_CLOCK_H

#include <cstdint>
#include <string>

namespace bubble {

class Clock {
 public:
  Clock();

  void Debug() const;
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
