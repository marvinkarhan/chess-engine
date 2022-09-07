#include "halfkp.h"
#include "../constants.h"
#include "../board.h"
#include "../movehelper.h"

namespace NNUE
{

  // Orient a square according to perspective (rotates by 180 for black)
  inline Square orient(bool perspective, Square s)
  {
    return Square(int(s) ^ (!bool(perspective) * 63));
  }

  // Find the index of the feature quantity from the king position and PieceSquare
  inline std::uint32_t HalfKP::MakeIndex(
      bool perspective, Square s, Piece pc, Square ksq)
  {
    return static_cast<std::uint32_t>(orient(perspective, s) + kpp_board_index[pc][perspective] + PS_END * ksq);
  }

  // Get a list of indices for active features
  void HalfKP::AppendActiveIndices(Board &board, bool perspective, IndexList &active)
  {
    Square ksq = orient(perspective, toNNUESquare(bitScanForward(board.pieces(perspective, KING))));
    BB bb = board.pieces(ALL_PIECES) & ~board.pieces(KING);
    while (bb)
    {
      int s = pop_lsb(bb);
      active.push_back(MakeIndex(perspective, toNNUESquare(s), board.piecePos[s], ksq));
    }
  }

  // Get a list of indices for recently changed features
  bool HalfKP::AppendChangedIndices(Board &board, bool perspective, IndexList &removed, IndexList &added)
  {
    const auto &dp = board.state->dirtyPiece;
    if (dp.dirty_num == 0)
      return false;

    bool reset = false;
    reset = dp.piece[0] == makePiece(perspective, KING);
    if (reset) {
      AppendActiveIndices(board, perspective, added);
    } else {
      Square ksq = orient(perspective, toNNUESquare(bitScanForward(board.pieces(perspective, KING))));
      const auto &dp = board.state->dirtyPiece;
      for (int i = 0; i < dp.dirty_num; ++i)
      {
        Piece pc = dp.piece[i];
        if (getPieceType(pc) == KING)
          continue;
        if (dp.from[i] >= 0 && dp.from[i] < 64)
          removed.push_back(MakeIndex(perspective, dp.from[i], pc, ksq));
        if (dp.to[i] >= 0 && dp.to[i] < 64)
          added.push_back(MakeIndex(perspective, dp.to[i], pc, ksq));
      }
    }
    return reset;
  }
} // namespace NNUE
