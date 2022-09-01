#pragma once

#include <string>
#include <optional>
#include "../board.h"

#define NNUEFileName "default.nnue"

namespace NNUE
{
  void init();
  void loadFile(std::string fileName);
  
  bool loadWeights(std::istream &stream);
  int evaluate(Board &board);
} // namespace NNUE