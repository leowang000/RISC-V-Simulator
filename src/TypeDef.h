#ifndef RISC_V_SIMULATOR_TYPEDEF_H
#define RISC_V_SIMULATOR_TYPEDEF_H

#include <cstdint>

namespace bubble {

constexpr int kInstQueueSize = 4;
constexpr int kXLen = 32;
constexpr int kRoBSize = 6;
constexpr int kRSSize = 5;
constexpr int kLSBSize = 4;

enum InstType {
  kLUI, kAUIPC, kJAL, kJALR, kBEQ, kBNE, kBLT, kBGE, kBLTU, kBGEU, kLB, kLH, kLW, kLBU, kLHU, kSB, kSH, kSW, kADDI,
  kSLTI, kSLTIU, kXORI, kORI, kANDI, kSLLI, kSRLI, kSRAI, kADD, kSUB, kSLL, kSLT, kSLTU, kXOR, kSRL, kSRA, kOR, kAND,
  kHALT
};

enum ALUOpType {
  kAdd, kSub, kAnd, kOr, kXor, kShiftLeftLogical, kShiftRightLogical, kShiftRightArithmetic, kEqual, kNotEqual,
  kLessThan, kLessThanUnsigned, kGreaterOrEqual, kGreaterOrEqualUnsigned
};

struct InstQueueEntry {
  uint32_t inst_, pc_;
  bool jump_;

  InstQueueEntry() = default;
  InstQueueEntry(uint32_t inst, uint32_t pc, bool jump);
  InstQueueEntry &operator=(const InstQueueEntry &other) = default;
};

struct IUToDecoder {
  bool get_inst_, is_jump_predicted_;
  uint32_t inst_, addr_;

  IUToDecoder() = default;
  IUToDecoder(bool get_inst, uint32_t inst, uint32_t pc, bool jump);
  IUToDecoder &operator=(const IUToDecoder &other) = default;
};

struct DecoderOutput {
  InstType inst_type_;
  uint8_t rd_, rs1_, rs2_;
  uint32_t imm_, addr_;
  bool is_jump_predicted_, get_inst_;
};

// store instruction: dest_ = address to store; jump instruction: dest_ = destination of the jump
struct RoBEntry {
  InstType inst_type_;
  bool done_, is_jump_predicted_;
  uint8_t rd_;
  uint32_t val_, dest_, addr_;
};

struct RobToRF {
  bool write_;
  uint8_t rd_;
  uint32_t val_;

  RobToRF() = default;
  RobToRF(bool write, uint8_t rd, uint32_t val);
  RobToRF &operator=(const RobToRF &other) = default;
};

struct RobToMemory {
  InstType inst_type_;
  bool store_;
  uint32_t store_addr_, val_;

  RobToMemory() = default;
  RobToMemory(bool store, InstType inst_type, uint32_t addr, uint32_t val);
  RobToMemory &operator=(const RobToMemory &other) = default;
};

struct FlushInfo {
  bool flush_;
  uint32_t pc_;

  FlushInfo() = default;
  FlushInfo(bool flush, uint32_t pc);
  FlushInfo &operator=(const FlushInfo &other) = default;
};

// If Q_ == -1, V_ is the address to load or store. Otherwise, V_ is the offset.
struct LSBEntry {
  InstType inst_type_;
  int id_, Q_;
  uint32_t V_;
};

struct LSBToMemory {
  InstType inst_type_;
  bool load_;
  uint32_t load_addr_;
  int id_;
};

struct RSEntry {
  bool busy_;
  int Q1_, Q2_, id_;
  uint32_t V1_, V2_;
};

struct RSToALU {
  bool execute_;
  ALUOpType alu_op_type_;
  uint32_t in1_, in2_;
  int id_;
};

struct MemoryOutput {
  bool done_;
  uint32_t val_;
  int id_;
};

struct ALUOutput {
  bool done_;
  uint32_t val_;
  int id_;
};

}

#endif //RISC_V_SIMULATOR_TYPEDEF_H
