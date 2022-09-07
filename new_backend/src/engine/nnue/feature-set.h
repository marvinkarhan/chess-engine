#pragma once

#include <array>
#include <assert.h>

#include "nnue-helper.h"
#include "../constants.h"

namespace NNUE
{
  class IndexList;

  template <typename... FeatureTypes>
  class FeatureSet;

  enum class Side
  {
    kFriend // side to move
  };

  // Base class of feature set
  template <typename Derived>
  class FeatureSetBase
  {

  public:
    // Get a list of indices for active features
    template <typename IndexListType>
    static void AppendActiveIndices(Board &board, IndexListType active[2])
    {

      for (bool perspective : {WHITE, BLACK})
      {
        Derived::CollectActiveIndices(board, perspective, &active[perspective]);
      }
    }

    // Get a list of indices for recently changed features
    template <typename IndexListType>
    static void AppendChangedIndices(Board &board, IndexListType removed[2], IndexListType added[2], bool reset[2])
    {

      const auto &dp = board.state->dirtyPiece;
      if (dp.dirty_num == 0)
        return;

      for (bool perspective : {WHITE, BLACK})
      {
        reset[perspective] = false;
        reset[perspective] = dp.piece[0] == makePiece(perspective, KING);
        if (reset[perspective])
        {
          Derived::CollectActiveIndices(board, perspective, &added[perspective]);
        }
        else
        {
          Derived::CollectChangedIndices(board, perspective, &removed[perspective], &added[perspective]);
        }
      }
    }
  };

  // Class template that represents the feature set
  template <typename FeatureType>
  class FeatureSet<FeatureType> : public FeatureSetBase<FeatureSet<FeatureType>>
  {

  public:
    // Hash value embedded in the evaluation file
    static constexpr std::uint32_t kHashValue = FeatureType::kHashValue;
    // Number of feature dimensions
    static constexpr std::uint32_t kDimensions = FeatureType::kDimensions;
    // Maximum number of simultaneously active features
    static constexpr std::uint32_t kMaxActiveDimensions = FeatureType::kMaxActiveDimensions;

  private:
    // Get a list of indices for active features
    static void CollectActiveIndices(Board &board, const bool perspective, IndexList *const active)
    {
      FeatureType::AppendActiveIndices(board, perspective, active);
    }

    // Get a list of indices for recently changed features
    static void CollectChangedIndices(Board &board, const bool perspective, IndexList *const removed, IndexList *const added)
    {
      FeatureType::AppendChangedIndices(board, perspective, removed, added);
    }

    // Make the base class and the class template that recursively uses itself a friend
    friend class FeatureSetBase<FeatureSet>;
    template <typename... FeatureTypes>
    friend class FeatureSet;
  };

} // namespace NNUE
