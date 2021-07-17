#ifndef BOARDEVENTS_HPP
#define BOARDEVENTS_HPP
#include <string>

enum BoardEvents
{
  NEW_BOARD,
  MAKE_MOVE,
  UNMAKE_MOVE,
  NEW_ENGINE_MOVE,
};
const char* BOARD_EVENTS_NAMES[] = {"new_board", "make_move", "unmake_move", "new_engine_move"};
#endif