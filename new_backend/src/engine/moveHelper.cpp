#include "movehelper.h"
#include <sstream>
#include <iostream>

std::vector<BB> get_lsb_bb_array(BB bb)
{
  std::vector<BB> bbs;
  BB new_bb;
  while (bb)
  {
    int index = pop_lsb(bb);
    new_bb = 1 << index;
    bbs.push_back(new_bb);
  }
  return bbs;
}

std::vector<int> get_lsb_array(BB bb)
{
  std::vector<int> indexes;
  while (bb)
  {
    int index = pop_lsb(bb);
    indexes.push_back(index);
  }
  return indexes;
}

Move uciToMove(std::string uci) {
  unsigned char character, file, rank;
  std::istringstream ss(uci);
  // origin square
  ss >> character;
  file = (character - 'a');
  ss >> character;
  rank = (character - '1');
  int originSquare = (7 - file) + 8 * rank;
  // target square
  ss >> character;
  file = (character - 'a');
  ss >> character;
  rank = (character - '1');
  int targetSquare = (7 - file) + 8 * rank;

  if (!(originSquare || targetSquare))
  {
    throw "Invalid uci";
  }
  // possible promotion
  ss >> character;
  return Move(originSquare, targetSquare, isdigit(character) ? NORMAL : PROMOTION, isdigit(character) ? NO_PIECE : (Piece) character);
}