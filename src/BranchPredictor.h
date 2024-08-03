#ifndef RISC_V_SIMULATOR_BRANCHPREDICTOR_H
#define RISC_V_SIMULATOR_BRANCHPREDICTOR_H

#include <cstdint>

namespace bubble {

class BranchPredictor {
 public:
  BranchPredictor();

  bool Predict(uint32_t pc) const;
  void Update(uint32_t pc, bool jump, bool correct);
  double GetAccuracy() const;

 private:
  static constexpr int kMod = 128;

  static constexpr int GetHash(uint32_t pc) {
    return pc % kMod;
  }

  uint8_t predictor_[kMod];
  unsigned long long total_[kMod], correct_[kMod];
};

}

#endif //RISC_V_SIMULATOR_BRANCHPREDICTOR_H
