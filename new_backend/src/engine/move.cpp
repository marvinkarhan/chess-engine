#include "move.h"
#include <string>
#include <iostream>

Move::Move() {}

Move::Move(int originSquare, int targetSquare, MoveType type /*=NORMAL*/, Piece promotion /*=NO_PIECE*/)
{
  this->originSquare = originSquare;
  this->targetSquare = targetSquare;
  this->promotion = promotion;
  this->type = type;
}

bool Move::operator==(const Move &other)
{
  return (originSquare == other.originSquare &&
          targetSquare == other.targetSquare &&
          promotion == other.promotion);
}

std::string Move::to_uci_string() const
{
  return SQUARE_TO_ALGEBRAIC[originSquare] + SQUARE_TO_ALGEBRAIC[targetSquare] + (promotion ? std::string(1, CharIndexToPiece[promotion]) : "");
}
