#undef _DEBUG

#ifndef RISC_V_SIMULATOR_CONFIG_H
#define RISC_V_SIMULATOR_CONFIG_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace bubble {

constexpr int kXLen = 32;

#ifdef _DEBUG
constexpr int kInstQueueSize = 3;
constexpr int kRoBSize = 2;
constexpr int kRSSize = 2;
constexpr int kLSBSize = 2;
#else
constexpr int kInstQueueSize = 32;
constexpr int kRoBSize = 32;
constexpr int kRSSize = 32;
constexpr int kLSBSize = 32;
#endif

enum InstType {
  kLUI, kAUIPC, kJAL, kJALR, kBEQ, kBNE, kBLT, kBGE, kBLTU, kBGEU, kLB, kLH, kLW, kLBU, kLHU, kSB, kSH, kSW, kADDI,
  kSLTI, kSLTIU, kXORI, kORI, kANDI, kSLLI, kSRLI, kSRAI, kADD, kSUB, kSLL, kSLT, kSLTU, kXOR, kSRL, kSRA, kOR, kAND,
  kHALT
};

const std::unordered_map<InstType, std::string> inst_map = {{kLUI,   "kLUI"},
                                                            {kAUIPC, "kAUIPC"},
                                                            {kJAL,   "kJAL"},
                                                            {kJALR,  "kJALR"},
                                                            {kBEQ,   "kBEQ"},
                                                            {kBNE,   "kBNE"},
                                                            {kBLT,   "kBLT"},
                                                            {kBGE,   "kBGE"},
                                                            {kBLTU,  "kBLTU"},
                                                            {kBGEU,  "kBGEU"},
                                                            {kLB,    "kLB"},
                                                            {kLH,    "kLH"},
                                                            {kLW,    "kLW"},
                                                            {kLBU,   "kLBU"},
                                                            {kLHU,   "kLHU"},
                                                            {kSB,    "kSB"},
                                                            {kSH,    "kSH"},
                                                            {kSW,    "kSW"},
                                                            {kADDI,  "kADDI"},
                                                            {kSLTI,  "kSLTI"},
                                                            {kSLTIU, "kSLTIU"},
                                                            {kXORI,  "kXORI"},
                                                            {kORI,   "kORI"},
                                                            {kANDI,  "kANDI"},
                                                            {kSLLI,  "kSLLI"},
                                                            {kSRLI,  "kSRLI"},
                                                            {kSRAI,  "kSRAI"},
                                                            {kADD,   "kADD"},
                                                            {kSUB,   "kSUB"},
                                                            {kSLL,   "kSLL"},
                                                            {kSLT,   "kSLT"},
                                                            {kSLTU,  "kSLTU"},
                                                            {kXOR,   "kXOR"},
                                                            {kSRL,   "kSRL"},
                                                            {kSRA,   "kSRA"},
                                                            {kOR,    "kOR"},
                                                            {kAND,   "kAND"},
                                                            {kHALT,  "kHALT"}};

enum ALUOpType {
  kAdd, kSub, kAnd, kOr, kXor, kShiftLeftLogical, kShiftRightLogical, kShiftRightArithmetic, kEqual, kNotEqual,
  kLessThan, kLessThanUnsigned, kGreaterOrEqual, kGreaterOrEqualUnsigned
};

const std::unordered_map<ALUOpType, std::string> alu_map = {{kAdd,                    "kAdd"},
                                                            {kSub,                    "kSub"},
                                                            {kAnd,                    "kAnd"},
                                                            {kOr,                     "kOr"},
                                                            {kXor,                    "kXor"},
                                                            {kShiftLeftLogical,       "kShiftLeftLogical"},
                                                            {kShiftRightLogical,      "kShiftRightLogical"},
                                                            {kShiftRightArithmetic,   "kShiftRightArithmetic"},
                                                            {kEqual,                  "kEqual"},
                                                            {kNotEqual,               "kNotEqual"},
                                                            {kLessThan,               "kLessThan"},
                                                            {kLessThanUnsigned,       "kLessThanUnsigned"},
                                                            {kGreaterOrEqual,         "kGreaterOrEqual"},
                                                            {kGreaterOrEqualUnsigned, "kGreaterOrEqualUnsigned"}};

struct InstQueueEntry {
  uint32_t inst_ = 0, addr_ = 0;
  bool jump_ = false;

  InstQueueEntry() = default;
  InstQueueEntry(uint32_t inst, uint32_t pc, bool jump);
  InstQueueEntry &operator=(const InstQueueEntry &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ inst_ = " << inst_ << ", addr_ = " << addr_ << ", jump_ = " << jump_ << " }";
    return sstr.str();
  }
};

struct IUToMemory {
  bool load_ = false;
  uint32_t pc_ = 0;

  IUToMemory() = default;
  IUToMemory(bool load, uint32_t pc);
  IUToMemory &operator=(const IUToMemory &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ load_ = " << load_ << ", pc_ = " << pc_ << " }";
    return sstr.str();
  }
};

struct IUToDecoder {
  bool get_inst_ = false, is_jump_predicted_ = false;
  uint32_t inst_ = 0, addr_ = 0;

  IUToDecoder() = default;
  IUToDecoder(bool get_inst, uint32_t inst, uint32_t pc, bool jump);
  IUToDecoder &operator=(const IUToDecoder &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ get_inst_ = " << get_inst_ << ", inst_ = " << inst_ << ", addr_ = " << addr_ << ", is_jump_predicted_ = "
         << is_jump_predicted_ << " }";
    return sstr.str();
  }
};

struct DecoderOutput {
  InstType inst_type_ = kLUI;
  uint8_t rd_ = 0, rs1_ = 0, rs2_ = 0;
  uint32_t imm_ = 0, addr_ = 0;
  bool is_jump_predicted_ = false, get_inst_ = false;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    std::string inst_type_str = inst_map.count(inst_type_) ? inst_map.at(inst_type_) : "No Inst Type";
    sstr << "{ get_inst_ = " << get_inst_ << ", inst_type_ = " << inst_type_str << ", addr_ = " << addr_
         << ", rs1_ = " << (int) rs1_ << ", rs2_ = " << (int) rs2_ << ", rd_ = " << (int) rd_ << ", imm_ = " << imm_
         << ", is_jump_predicted_ = " << is_jump_predicted_ << " }";
    return sstr.str();
  }
};

// store instruction: dest_ = address to store; jump instruction: dest_ = destination of the jump
struct RoBEntry {
  InstType inst_type_ = kLUI;
  bool done_ = false, is_jump_predicted_ = false;
  uint8_t rd_ = 0;
  uint32_t val_ = 0, dest_ = 0, addr_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    std::string inst_type_str = inst_map.count(inst_type_) ? inst_map.at(inst_type_) : "No Inst Type";
    sstr << "{ inst_type_ = " << inst_type_str << ", done_ = " << done_ << ", rd_ = " << (int) rd_
         << ", val_ = " << val_ << ", is_jump_predicted_ = " << is_jump_predicted_ << ", dest_ = " << dest_
         << ", addr_ = " << addr_ << " }";
    return sstr.str();
  }
};

struct RobToRF {
  bool write_ = false;
  uint8_t rd_ = 0;
  uint32_t val_ = 0;

  RobToRF() = default;
  RobToRF(bool write, uint8_t rd, uint32_t val);
  RobToRF &operator=(const RobToRF &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ write_ = " << write_ << ", rd_ = " << (int) rd_ << ", val_ = " << val_ << " }";
    return sstr.str();
  }
};

struct RobToMemory {
  InstType inst_type_ = kLUI;
  bool store_ = false;
  uint32_t store_addr_ = 0, val_ = 0;

  RobToMemory() = default;
  RobToMemory(bool store, InstType inst_type, uint32_t addr, uint32_t val);
  RobToMemory &operator=(const RobToMemory &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    std::string inst_type_str = inst_map.count(inst_type_) ? inst_map.at(inst_type_) : "No Inst Type";
    sstr << "{ store_ = " << store_ << ", inst_type_ = " << inst_type_str << ", store_addr_ = " << store_addr_
         << ", val_ = " << val_ << " }";
    return sstr.str();
  }
};

struct FlushInfo {
  bool flush_ = false;
  uint32_t pc_ = 0;

  FlushInfo() = default;
  FlushInfo(bool flush, uint32_t pc);
  FlushInfo &operator=(const FlushInfo &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ flush_ = " << flush_ << ", pc_ = " << pc_ << " }";
    return sstr.str();
  }
};

// If Q1_ == -1, V1_ is the address to load or store. Otherwise, V1_ is the offset.
struct LSBEntry {
  InstType inst_type_ = kLUI;
  int id_ = 0, Q1_ = -1, Q2_ = -1;
  uint32_t V1_ = 0, V2_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    std::string inst_type_str = inst_map.count(inst_type_) ? inst_map.at(inst_type_) : "No Inst Type";
    sstr << "{ inst_type_ = " << inst_type_str << ", id_ = " << id_ << ", Q1_ = " << Q1_ << ", V1_ = " << V1_
         << ", Q2_ = " << Q2_ << ", V2_ = " << V2_ << " }";
    return sstr.str();
  }
};

struct LSBToMemory {
  InstType inst_type_ = kLUI;
  bool load_ = false;
  uint32_t load_addr_ = 0;
  int id_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    std::string inst_type_str = inst_map.count(inst_type_) ? inst_map.at(inst_type_) : "No Inst Type";
    sstr << "{ load_ = " << load_ << ", id_ = " << id_ << ", load_addr_ = " << load_addr_ << ", inst_type_ = "
         << inst_type_str << " }";
    return sstr.str();
  }
};

struct RSEntry {
  bool busy_ = false;
  int Q1_ = -1, Q2_ = -1, id_ = 0;
  uint32_t V1_ = 0, V2_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ busy_ = " << busy_ << ", id_ = " << id_ << ", Q1_ = " << Q1_ << ", Q2_ = " << Q2_ << ", V1_ = " << V1_
         << ", V2_ = " << V2_ << " }";
    return sstr.str();
  }
};

struct RSToALU {
  bool execute_ = false;
  ALUOpType alu_op_type_ = kAdd;
  uint32_t in1_ = 0, in2_ = 0;
  int id_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ execute_ = " << execute_ << ", alu_op_type_ = " << alu_map.at(alu_op_type_) << ", id_ = " << id_
         << ", in1_ = " << in1_ << ", in2_ = " << in2_ << " }";
    return sstr.str();
  }
};

struct MemoryToIU {
  uint32_t inst_ = 0, pc_ = 0;

  MemoryToIU() = default;
  MemoryToIU(uint32_t inst, uint32_t pc);
  MemoryToIU &operator=(const MemoryToIU &other) = default;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ inst_ = " << inst_ << ", pc_ = " << pc_ << " }";
    return sstr.str();
  };
};

struct MemoryOutput {
  bool done_ = false;
  uint32_t val_ = 0;
  int id_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ done_ = " << done_ << ", id_ = " << id_ << ", val_ = " << val_ << " }";
    return sstr.str();
  }
};

struct ALUOutput {
  bool done_ = false;
  uint32_t val_ = 0;
  int id_ = 0;

  std::string ToString() const {
    std::stringstream sstr;
    sstr << std::boolalpha;
    sstr << "{ done_ = " << done_ << ", id_ = " << id_ << ", val_ = " << val_ << " }";
    return sstr.str();
  }
};

}

#endif //RISC_V_SIMULATOR_CONFIG_H
