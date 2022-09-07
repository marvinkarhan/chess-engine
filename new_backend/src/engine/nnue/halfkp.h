#pragma once

#include "input.h"
#include "affine-transform.h"
#include "clipped-relu.h"
#include "nnue-helper.h"
#include "../constants.h"

namespace NNUE
{
  // Feature HalfKP: Combination of the position of own king
  // and the position of pieces other than kings
  class HalfKP
  {

  public:
    // Feature name
    static constexpr const char *kName = "HalfKP(Friend)";
    // Number of feature dimensions
    static constexpr std::uint32_t kDimensions =
        static_cast<std::uint32_t>(64) * static_cast<std::uint32_t>(PS_END);
    // Maximum number of simultaneously active features (Kings don't count)
    static constexpr std::uint32_t kMaxActiveDimensions = 30;
    using IndexList = ValueList<std::uint32_t, kMaxActiveDimensions>;

    // Get a list of indices for active features
    static void AppendActiveIndices(Board &board, bool perspective, IndexList &active);

    // Get a list of indices for recently changed features
    static bool AppendChangedIndices(Board &board, bool perspective, IndexList &removed, IndexList &added);

  private:
    // Index of a feature for a given king position and another piece on some square
    static std::uint32_t MakeIndex(bool perspective, Square s, Piece pc, Square sq_k);
  };

} // namespace NNUE
