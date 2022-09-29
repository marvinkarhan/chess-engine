#pragma once

#include "nnue-helper.h"
#include "architecture.h"

namespace NNUE
{

  // Class that holds the result of affine transformation of input features
  struct alignas(kCacheLineSize) Accumulator
  {
    std::int16_t accumulation[2][kTransformedFeatureDimensions];
    bool computed_accumulation;
  };

} // namespace NNUE
