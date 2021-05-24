#ifndef BOARDEVENTS_HPP
#define BOARDEVENTS_HPP
#include <string>

enum BoardEvents
{
  NEW_BOARD,
  MAKE_MOVE
};
const std::string BOARD_EVENTS_NAMES[2] = {"new_board", "make_move"};

#endif