#include <iomanip>
#include <iostream>

#include "src/CPU.h"

int main() {
  uint32_t output;

#ifdef _DEBUG
  bubble::CPU cpu("pc", "pc_with_cycle");
  cpu.LoadMemory("/mnt/c/Users/leowa/CLionProjects/RISC-V-Simulator/testcases/multiarray.data");
  std::streambuf *cout_buf = std::cout.rdbuf();
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
  std::cout.rdbuf(cout_buf);
  std::cout << "output: " << output << "\n";
  std::cout << "clock cycle count: " << cpu.clock_.GetCycleCount() << "\n";
  std::cout << "accuracy of branch prediction: " << cpu.bp_.GetAccuracy() << "\n";
#else
  std::cout << output;
#endif
  return 0;
}
