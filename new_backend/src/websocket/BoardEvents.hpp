#ifndef BOARDEVENTS_HPP
#define BOARDEVENTS_HPP
#include <string>

enum BoardEvents
{
  NEW_BOARD,
  MAKE_MOVE,
  UNMAKE_MOVE,
};
const char* BOARD_EVENTS_NAMES[3] = {"new_board", "make_move", "unmake_move"};
#endif