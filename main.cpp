#include <fstream>
#include <iomanip>
#include <iostream>

#include "src/CPU.h"

int main() {
  uint32_t output;
  bubble::CPU cpu;
  /**/cpu.LoadMemory("/mnt/c/Users/leowa/CLionProjects/RISC-V-Simulator/testcases/naive.data");
  //cpu.LoadMemory();
  cpu.clock_.Run();
  /**/freopen("debug.txt", "w", stdout);
  std::cout << std::boolalpha;
  while (!cpu.ShouldHalt()) {
    if (cpu.clock_.GetCycleCount() == 19) {
      int stop;
      stop = 0;
    }
    cpu.Update();
    /**/cpu.Debug();
    cpu.Execute();
    cpu.Write();
    cpu.clock_.Tick();
  }
  output = cpu.Halt();
  std::cout << output;
  return 0;
}
