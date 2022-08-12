#pragma once

#include <string>
#include <optional>
#include "../board.h"

#define NNUEFileName "default.nnue"

namespace NNUE
{
  void init();
  
  bool loadWeights(std::string name, std::istream &stream);
  int evaluate(Board &board);
} // namespace NNUE