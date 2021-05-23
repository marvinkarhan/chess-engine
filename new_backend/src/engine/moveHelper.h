#include "constants.h"
#include <algorithm>

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

static Direction HORIZONTAL_MOVES[4] = {LEFT, RIGHT};
static Direction VERTICAL_MOVES[4] = {UP, DOWN};
static Direction HORIZONTAL_VERTICAL_MOVES[4] = {LEFT, RIGHT, UP, DOWN};
static Direction DIAGONALS_MOVES[4] = {LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
static Direction DIRECTION_MOVES[8] = {LEFT, RIGHT, UP, DOWN, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
static Direction KNIGHT_MOVES[8][3] = {
  {LEFT, LEFT, UP},
  {LEFT, LEFT, DOWN},
  {UP, UP, LEFT},
  {UP, UP, RIGHT},
  {RIGHT, RIGHT, UP},
  {RIGHT, RIGHT, DOWN},
  {DOWN, DOWN, LEFT},
  {DOWN, DOWN, LEFT}
};

BB traverse_bb(BB bb, Direction directions[], BB friendlies_bb, BB enemies_bb);

BB rook_moves(BB bb, BB empties_bb, BB friendlies_bb);

BB bishop_moves(BB bb, BB empties_bb, BB friendlies_bb);

BB queen_moves(BB bb, BB empties_bb, BB friendlies_bb);

BB king_moves(BB bb, BB friendlies_bb);

BB knight_moves(BB bb, BB friendlies_bb);

BB pawn_attacks(BB bb, int active_side, BB friendlies_bb);