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

  // Trigger to perform full calculations instead of difference only
  enum class TriggerEvent
  {
    kFriendKingMoved // calculate full evaluation when own king moves
  };

  enum class Side
  {
    kFriend // side to move
  };

  // Class template that represents a list of values
  template <typename T, T... Values>
  struct CompileTimeList;

  template <typename T, T First, T... Remaining>
  struct CompileTimeList<T, First, Remaining...>
  {
    static constexpr bool Contains(T value)
    {
      return value == First || CompileTimeList<T, Remaining...>::Contains(value);
    }
    static constexpr std::array<T, sizeof...(Remaining) + 1>
        kValues = {{First, Remaining...}};
  };

  // Base class of feature set
  template <typename Derived>
  class FeatureSetBase
  {

  public:
    // Get a list of indices for active features
    template <typename IndexListType>
    static void AppendActiveIndices(
        Board &board, TriggerEvent trigger, IndexListType active[2])
    {

      for (bool perspective : {WHITE, BLACK})
      {
        Derived::CollectActiveIndices(
            board, trigger, perspective, &active[perspective]);
      }
    }

    // Get a list of indices for recently changed features
    template <typename IndexListType>
    static void AppendChangedIndices(
        Board &board, TriggerEvent trigger,
        IndexListType removed[2], IndexListType added[2], bool reset[2])
    {

      const auto &dp = board.state->dirtyPiece;
      if (dp.dirty_num == 0)
        return;

      for (bool perspective : {WHITE, BLACK})
      {
        reset[perspective] = false;
        switch (trigger)
        {
        case TriggerEvent::kFriendKingMoved:
          reset[perspective] = dp.piece[0] == makePiece(perspective, KING);
          break;
        default:
          assert(false);
          break;
        }
        if (reset[perspective])
        {
          Derived::CollectActiveIndices(
              board, trigger, perspective, &added[perspective]);
        }
        else
        {
          Derived::CollectChangedIndices(
              board, trigger, perspective,
              &removed[perspective], &added[perspective]);
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
    static constexpr std::uint32_t kMaxActiveDimensions =
        FeatureType::kMaxActiveDimensions;
    // Trigger for full calculation instead of difference calculation
    using SortedTriggerSet =
        CompileTimeList<TriggerEvent, FeatureType::kRefreshTrigger>;
    static constexpr auto kRefreshTriggers = SortedTriggerSet::kValues;

  private:
    // Get a list of indices for active features
    static void CollectActiveIndices(
        Board &board, const TriggerEvent trigger, const bool perspective,
        IndexList *const active)
    {
      if (FeatureType::kRefreshTrigger == trigger)
      {
        FeatureType::AppendActiveIndices(board, perspective, active);
      }
    }

    // Get a list of indices for recently changed features
    static void CollectChangedIndices(
        Board &board, const TriggerEvent trigger, const bool perspective,
        IndexList *const removed, IndexList *const added)
    {

      if (FeatureType::kRefreshTrigger == trigger)
      {
        FeatureType::AppendChangedIndices(board, perspective, removed, added);
      }
    }

    // Make the base class and the class template that recursively uses itself a friend
    friend class FeatureSetBase<FeatureSet>;
    template <typename... FeatureTypes>
    friend class FeatureSet;
  };

} // namespace NNUE
