#pragma once
#include "constants.h"
#include <string>

enum MoveType
{
  NORMAL,
  PROMOTION,
  EN_PASSANT,
  CASTLING
};

class Move
{
public:
  int originSquare;
  int targetSquare;
  Piece promotion;
  MoveType type;
  Move();
  Move(int originSquare, int targetSquare, MoveType type = NORMAL, Piece promotion = NO_PIECE);
  bool operator==(const Move &other);
  std::string to_uci_string();
};