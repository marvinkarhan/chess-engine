#ifndef MOVE_HELPER_H
#define MOVE_HELPER_H

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

Direction HORIZONTAL_MOVES[4] = {LEFT, RIGHT};
Direction VERTICAL_MOVES[4] = {UP, DOWN};
Direction HORIZONTAL_VERTICAL_MOVES[4] = {LEFT, RIGHT, UP, DOWN};
Direction DIAGONALS_MOVES[4] = {LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
Direction DIRECTION_MOVES[8] = {LEFT, RIGHT, UP, DOWN, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
Direction KNIGHT_MOVES[8][3] = {
  {LEFT, LEFT, UP},
  {LEFT, LEFT, DOWN},
  {UP, UP, LEFT},
  {UP, UP, RIGHT},
  {RIGHT, RIGHT, UP},
  {RIGHT, RIGHT, DOWN},
  {DOWN, DOWN, LEFT},
  {DOWN, DOWN, LEFT}
};

inline BB traverse_bb(BB bb, Direction directions[], BB friendlies_bb, BB enemies_bb);

#endif /* MOVE_HELPER_H */