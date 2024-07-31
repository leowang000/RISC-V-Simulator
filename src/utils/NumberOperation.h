#ifndef RISC_V_SIMULATOR_NUMBEROPERATION_H
#define RISC_V_SIMULATOR_NUMBEROPERATION_H

#include <cstdint>

namespace bubble {

constexpr uint32_t GetSub(uint8_t n, int start, int end) {
  return n << (static_cast<uint8_t>(7) - start) >> (static_cast<uint8_t>(7) - start + end);
}

constexpr uint32_t GetSub(uint16_t n, int start, int end) {
  return n << (static_cast<uint16_t>(15) - start) >> (static_cast<uint16_t>(15) - start + end);
}

constexpr uint32_t GetSub(uint32_t n, int start, int end) {
  return n << (31u - start) >> (31u - start + end);
}

constexpr uint32_t SignExtend(uint32_t n, int start) {
  return (n >> (start + 1)) ? (n | static_cast<uint32_t>(-1) << (start + 1)) : n;
}

}

#endif //RISC_V_SIMULATOR_NUMBEROPERATION_H
