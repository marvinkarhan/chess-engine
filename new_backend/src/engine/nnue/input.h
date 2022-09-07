#pragma once

#include <iostream>

#include "nnue-helper.h"

namespace NNUE
{

  // Input layer
  template <std::uint32_t OutputDimensions, std::uint32_t Offset = 0>
  class InputSlice
  {
  public:
    // Need to maintain alignment
    static_assert(Offset % kMaxSimdWidth == 0, "");

    // Output type
    using OutputType = std::uint8_t;

    // Output dimensionality
    static constexpr std::uint32_t kOutputDimensions = OutputDimensions;

    // Size of forward propagation buffer used from the input layer to this layer
    static constexpr std::size_t kBufferSize = 0;

    // Read network parameters
    bool readParameters(std::istream & /*stream*/)
    {
      return true;
    }

    // Forward propagation
    const OutputType *Propagate(
        const std::uint8_t *transformed_features,
        char * /*buffer*/) const
    {
      return transformed_features + Offset;
    }
  };

} // namespace NNUE
