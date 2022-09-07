#pragma once

#include "input.h"
#include "affine-transform.h"
#include "clipped-relu.h"
#include "feature-set.h"
#include "halfkp.h"

namespace NNUE
{
  // Input features used in evaluation function
  using RawFeatures = FeatureSet<HalfKP<Side::kFriend>>;

  // Number of input feature dimensions after conversion
  constexpr std::uint32_t kTransformedFeatureDimensions = 256;

  using InputLayer = InputSlice<kTransformedFeatureDimensions * 2>;
  using HiddenLayer1 = ClippedReLU<AffineTransform<InputLayer, 32>>;
  using HiddenLayer2 = ClippedReLU<AffineTransform<HiddenLayer1, 32>>;
  using OutputLayer = AffineTransform<HiddenLayer2, 1>;
  using Network = OutputLayer;

} // namespace NNUE
