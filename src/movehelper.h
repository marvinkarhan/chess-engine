#include "constants.h"
#include "move.h"
#include "board.h"
#include "Hash_Fancy.hpp"
#include <algorithm>
#include <vector>
#include <string>

constexpr int squareRank(Square sq) { return sq >> 3; }

constexpr int squareFile(Square sq) { return sq & 7; }

constexpr uint8_t diagonalOf(Square sq) {
  return 7 + squareRank(sq) - squareFile(sq);
}
constexpr uint8_t antiDiagonalOf(Square sq) {
  return squareRank(sq) + squareFile(sq);
}

static auto init_squares_between = []() constexpr
{
  // initialize squares between table
  std::array<std::array<BB, 64>, 64> squares_between_bb{};
  BB sqs = 0;
  for (Square sq1 = H1; sq1 <= A8; ++sq1)
  {
    for (Square sq2 = H1; sq2 <= A8; ++sq2)
    {
      sqs = (1ULL << sq1) | (1ULL << sq2);
      if (sq1 == sq2)
        squares_between_bb[sq1][sq2] = 0ull;
      else if (squareFile(sq1) == squareFile(sq2) || squareRank(sq1) == squareRank(sq2))
        squares_between_bb[sq1][sq2] = Chess_Lookup::Fancy::GetRookAttacks(sq1, sqs) & Chess_Lookup::Fancy::GetRookAttacks(sq2, sqs);
      else if (diagonalOf(sq1) == diagonalOf(sq2) ||
               antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
        squares_between_bb[sq1][sq2] =
            Chess_Lookup::Fancy::GetBishopAttacks(sq1, sqs) & Chess_Lookup::Fancy::GetBishopAttacks(sq2, sqs);
    }
  }
  return squares_between_bb;
};

static const std::array<std::array<BB, 64>, 64> REY_BBS = init_squares_between();

enum Direction : int
{
  LEFT,
  RIGHT,
  UP,
  DOWN,
  LEFT_UP,
  LEFT_DOWN,
  RIGHT_UP,
  RIGHT_DOWN
};

static std::vector<Direction> HORIZONTAL_MOVES{LEFT, RIGHT};
static std::vector<Direction> VERTICAL_MOVES{UP, DOWN};
static std::vector<Direction> HORIZONTAL_VERTICAL_MOVES{LEFT, RIGHT, UP, DOWN};
static std::vector<Direction> DIAGONALS_MOVES{LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
static std::vector<Direction> DIRECTION_MOVES{LEFT, RIGHT, UP, DOWN, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
static std::vector<std::vector<Direction>> KNIGHT_MOVES{
    {LEFT, LEFT, UP},
    {LEFT, LEFT, DOWN},
    {UP, UP, LEFT},
    {UP, UP, RIGHT},
    {RIGHT, RIGHT, UP},
    {RIGHT, RIGHT, DOWN},
    {DOWN, DOWN, LEFT},
    {DOWN, DOWN, RIGHT}};

constexpr int bitScanForward(BB bb)
{
  /* OLD (NOT C++ Way) */
  // """
  //   bitScanForward
  //   @author Kim Walisch (2012)
  //   @param bb bitboard to scan
  //   @precondition bb != 0
  //   @return index (0..63) of least significant one bit
  // """
  return DEBRUIJN_INDEX[((bb ^ (bb - 1)) * DEBRUIJN) >> 58];
  //   unsigned long index;
  //   _BitScanForward64(&index, bb);
  //   return index;
}

inline int pop_lsb(BB &bb)
{
  const int index = bitScanForward(bb);
  bb &= bb - 1;
  return index;
}

constexpr BB pop_last_bb(BB bb)
{
  bb &= bb - 1;
  return bb;
}

inline int popCount(BB bb)
{
  // split bb into four parts
  union
  {
    BB bb;
    uint16_t parts[4];
  } split = {bb};
  return STATIC_POP_COUNT_16[split.parts[0]] +
         STATIC_POP_COUNT_16[split.parts[1]] +
         STATIC_POP_COUNT_16[split.parts[2]] +
         STATIC_POP_COUNT_16[split.parts[3]];
};

// """
// following functions are helper functions
// they take a bitboard (bb)
// the bb must only contain one piece
// the piece (bit) is then shifted
// if a shift results in a piece wrapping the board (going from line A to H) it gets returned as 0
// """

inline BB move(BB bb, Direction dir)
{
  switch (dir)
  {
  case LEFT:
    return (bb << 1 & ~FILE_H) & FULL;
    break;
  case RIGHT:
    return bb >> 1 & ~FILE_A;
    break;
  case UP:
    return bb << 8 & FULL;
    break;
  case DOWN:
    return bb >> 8;
    break;
  case LEFT_UP:
    return (bb << 9 & ~FILE_H) & FULL;
    break;
  case LEFT_DOWN:
    return bb >> 7 & ~FILE_H;
    break;
  case RIGHT_UP:
    return (bb << 7 & ~FILE_A) & FULL;
    break;
  case RIGHT_DOWN:
    return bb >> 9 & ~FILE_A;
    break;
  }
  return 0;
}

inline BB rook_moves(int square, BB occupied_bb)
{
  return Chess_Lookup::Fancy::GetRookAttacks(square, occupied_bb);
}

inline BB bishop_moves(int sq, BB occupied_bb)
{
  return  Chess_Lookup::Fancy::GetBishopAttacks(sq, occupied_bb);
}

inline BB queen_moves(int sq, BB occupied_bb)
{
  return  Chess_Lookup::Fancy::Queen(sq, occupied_bb);
}

inline BB pawn_left_attacks(BB bb, int active_side)
{
  return (active_side ? move(bb, LEFT_UP) : move(bb, RIGHT_DOWN));
}

inline BB pawn_right_attacks(BB bb, int active_side)
{
  return (active_side ? move(bb, RIGHT_UP) : move(bb, LEFT_DOWN));
}

inline BB pawn_attacks(BB bb, int active_side, BB friendlies_bb)
{
  return (pawn_left_attacks(bb, active_side) |
          pawn_right_attacks(bb, active_side)) &
         ~friendlies_bb;
}

inline BB in_between(int origin, int target)
{
  return REY_BBS[origin][target];
}

inline BB may_move(int origin, int target, BB occupied_bb)
{
  return !(in_between(origin, target) & occupied_bb);
}

extern Move uciToMove(std::string uci, const Board &board);

MoveType getMoveType(int originSquare, int targetSquare, bool isDigit, const Board &board);

extern BB getPotentialEPSquareBB(int originSquare, int targetSquare, const Board &board);
