#include "TypeDef.h"

namespace bubble {

InstQueueEntry::InstQueueEntry(uint32_t inst, uint32_t pc, bool jump) :
    inst_(inst), addr_(pc), jump_(jump) {}

IUToMemory::IUToMemory(bool load, uint32_t pc) : load_(load), pc_(pc) {}

IUToDecoder::IUToDecoder(bool get_inst, uint32_t inst, uint32_t pc, bool jump) :
    get_inst_(get_inst), inst_(inst), addr_(pc), is_jump_predicted_(jump) {}

RobToRF::RobToRF(bool write, uint8_t rd, uint32_t val) :
    write_(write), rd_(rd), val_(val) {}

RobToMemory::RobToMemory(bool store, InstType inst_type, uint32_t addr, uint32_t val) :
    store_(store), inst_type_(inst_type), store_addr_(addr), val_(val) {}

FlushInfo::FlushInfo(bool flush, uint32_t pc) : flush_(flush), pc_(pc) {}

MemoryToIU::MemoryToIU(uint32_t inst, uint32_t pc) : inst_(inst), pc_(pc) {}

}