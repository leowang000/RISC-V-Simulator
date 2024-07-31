#include "BranchPredictor.h"

namespace bubble {

BranchPredictor::BranchPredictor() {
  for (auto &item : predictor_) {
    item = 0b01;
  }
}

bool BranchPredictor::Predict(uint32_t pc) const {
  return predictor_[GetHash(pc)] & 0b10;
}

void BranchPredictor::Update(uint32_t pc, bool jump, bool correct) {
  total_[GetHash(pc)]++;
  correct_[GetHash(pc)] += (correct ? 1 : 0);
  uint8_t &pred = predictor_[GetHash(pc)];
  if (jump && pred != 0b11) {
    pred++;
  }
  if (!jump && pred != 0b00) {
    pred--;
  }
}

double BranchPredictor::GetAccuracy() const {
  int total = 0, correct = 0;
  for (auto item : total_) {
    total += item;
  }
  for (auto item : correct_) {
    correct += item;
  }
  return static_cast<double>(correct) / total;
}

}