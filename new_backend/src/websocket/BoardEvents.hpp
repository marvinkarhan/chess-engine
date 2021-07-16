#ifndef BOARDEVENTS_HPP
#define BOARDEVENTS_HPP
#include <string>

enum BoardEvents
{
  NEW_BOARD,
  MAKE_MOVE,
  UNMAKE_MOVE,
  SWAP_BOARD,
};
const char* BOARD_EVENTS_NAMES[] = {"new_board", "make_move", "unmake_move", "swap_board"};
#endif