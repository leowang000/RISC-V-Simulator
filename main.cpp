#include <cstdint>
#include <iostream>

#include "src/CPU.h"

int main() {
  uint8_t output;
  bubble::CPU cpu;
  cpu.LoadMemory("/../testcases/naive.data");
  try {
    while (true) {
      cpu.Update();
      cpu.Execute();
      cpu.Write();
      cpu.clock_.Tick();
    }
  }catch (const std::string & err) {
    output = cpu.Halt();
  }
  std::cout << output;
  return 0;
}
