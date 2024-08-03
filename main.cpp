#include <iomanip>
#include <iostream>

#include "src/CPU.h"

int main() {
  uint32_t output;
#ifdef _DEBUG
  bubble::CPU cpu("pc.txt", "pc_with_cycle.txt");
  cpu.LoadMemory("../testcases/multiarray.data");
  freopen("debug.txt", "w", stdout);
  std::cout << std::boolalpha;
#else
  bubble::CPU cpu;
  cpu.LoadMemory();
#endif
  cpu.clock_.Run();
  while (!cpu.ShouldHalt()) {
    cpu.Update();
#ifdef _DEBUG
    cpu.Debug();
#endif
    cpu.Execute();
    cpu.Write();
    cpu.clock_.Tick();
  }
  output = cpu.Halt();
#ifdef _DEBUG
  freopen("/dev/tty", "w", stdout);
  std::cout << "output: " << output << "\n";
  std::cout << "clock cycle count: " << cpu.clock_.GetCycleCount() << "\n";
  std::cout << "accuracy of branch prediction: " << cpu.bp_.GetAccuracy() << "\n";
#else
  std::cout << output;
#endif
  return 0;
}
