#include "constants.h"
#include "move.h"
#include "board.h"
#include <algorithm>
#include <vector>
#include <string>

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

/* Following Board Filling Functions are using the Kogge-Stone Algorithm */

inline BB north_occluded_attacks(BB bb, BB empty_bb)
{
  bb |= empty_bb & (bb << 8);
  empty_bb &= (empty_bb << 8);
  bb |= empty_bb & (bb << 16);
  empty_bb &= (empty_bb << 16);
  bb |= empty_bb & (bb << 32);
  return bb << 8 & FULL;
}

inline BB east_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_A;
  bb |= empty_bb & (bb >> 1);
  empty_bb &= (empty_bb >> 1);
  bb |= empty_bb & (bb >> 2);
  empty_bb &= (empty_bb >> 2);
  bb |= empty_bb & (bb >> 4);
  return bb >> 1 & ~FILE_A;
}

inline BB south_occluded_attacks(BB bb, BB empty_bb)
{
  bb |= empty_bb & (bb >> 8);
  empty_bb &= (empty_bb >> 8);
  bb |= empty_bb & (bb >> 16);
  empty_bb &= (empty_bb >> 16);
  bb |= empty_bb & (bb >> 32);
  return bb >> 8;
}

inline BB west_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_H;
  bb |= empty_bb & (bb << 1);
  empty_bb &= (empty_bb << 1);
  bb |= empty_bb & (bb << 2);
  empty_bb &= (empty_bb << 2);
  bb |= empty_bb & (bb << 4);
  return (bb << 1 & ~FILE_H) & FULL;
}

inline BB no_we_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_H;
  bb |= empty_bb & (bb << 9);
  empty_bb &= (empty_bb << 9);
  bb |= empty_bb & (bb << 18);
  empty_bb &= (empty_bb << 18);
  bb |= empty_bb & (bb << 36);
  return (bb << 9 & ~FILE_H) & FULL;
}

inline BB so_we_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_H;
  bb |= empty_bb & (bb >> 7);
  empty_bb &= (empty_bb >> 7);
  bb |= empty_bb & (bb >> 14);
  empty_bb &= (empty_bb >> 14);
  bb |= empty_bb & (bb >> 28);
  return bb >> 7 & ~FILE_H;
}

inline BB so_ea_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_A;
  bb |= empty_bb & (bb >> 9);
  empty_bb &= (empty_bb >> 9);
  bb |= empty_bb & (bb >> 18);
  empty_bb &= (empty_bb >> 18);
  bb |= empty_bb & (bb >> 36);
  return bb >> 9 & ~FILE_A;
}

inline BB no_ea_occluded_attacks(BB bb, BB empty_bb)
{
  empty_bb &= NOT_A;
  bb |= empty_bb & (bb << 7);
  empty_bb &= (empty_bb << 7);
  bb |= empty_bb & (bb << 14);
  empty_bb &= (empty_bb << 14);
  bb |= empty_bb & (bb << 28);
  return (bb << 7 & ~FILE_A) & FULL;
}

inline BB horizontal_vertical_moves(BB bb, BB empty_bb)
{
  BB acc = 0;
  acc |= north_occluded_attacks(bb, empty_bb);
  acc |= east_occluded_attacks(bb, empty_bb);
  acc |= south_occluded_attacks(bb, empty_bb);
  acc |= west_occluded_attacks(bb, empty_bb);
  return acc;
}

inline BB diagonal_moves(BB bb, BB empty_bb)
{
  BB acc = 0;
  acc |= no_ea_occluded_attacks(bb, empty_bb);
  acc |= so_ea_occluded_attacks(bb, empty_bb);
  acc |= so_we_occluded_attacks(bb, empty_bb);
  acc |= no_we_occluded_attacks(bb, empty_bb);
  return acc;
}

inline BB traverse_bb(BB bb, std::vector<Direction> directions, BB friendlies_bb, BB enemies_bb)
{
  BB result_bb = 0;
  for (Direction &dir : directions)
  {
    BB square_bb = bb;
    while (1)
    {
      square_bb = move(square_bb, dir);
      if (square_bb & friendlies_bb)
        break;
      result_bb |= square_bb;
      if (square_bb & enemies_bb || square_bb == 0)
        break;
    }
  }
  return result_bb;
}

inline BB rook_moves(BB bb, BB empties_bb, BB friendlies_bb)
{
  return horizontal_vertical_moves(bb, empties_bb) & ~friendlies_bb;
}

inline BB bishop_moves(BB bb, BB empties_bb, BB friendlies_bb)
{
  return diagonal_moves(bb, empties_bb) & ~friendlies_bb;
}

inline BB queen_moves(BB bb, BB empties_bb, BB friendlies_bb)
{
  return rook_moves(bb, empties_bb, friendlies_bb) | bishop_moves(bb, empties_bb, friendlies_bb);
}

inline BB king_moves(BB bb, BB friendlies_bb)
{
  BB moves_bb = BB(0);
  for (Direction &dir : DIRECTION_MOVES)
    moves_bb |= move(bb, dir);
  return moves_bb & ~friendlies_bb;
}

inline BB knight_moves(BB bb, BB friendlies_bb)
{
  BB moves_bb = 0;
  for (auto &moves : KNIGHT_MOVES)
  {
    BB moveAcc = bb;
    for (Direction &dir : moves)
      moveAcc = move(moveAcc, dir);
    moves_bb |= moveAcc;
  }
  return moves_bb & ~friendlies_bb;
}

inline BB pawn_attacks(BB bb, int active_side, BB friendlies_bb)
{
  if (active_side)
    return (move(bb, LEFT_UP) | move(bb, RIGHT_UP)) & ~friendlies_bb;
  return (move(bb, LEFT_DOWN) | move(bb, RIGHT_DOWN)) & ~friendlies_bb;
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

extern Square getPotentialEPSquareBB(int originSquare, int targetSquare, const Board &board);