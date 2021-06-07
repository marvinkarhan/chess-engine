#pragma once
#include "constants.h"
#include <string>

enum MoveType
{
  NORMAL,
  PROMOTION = 1 << 12,
  EN_PASSANT = 2 << 12,
  CASTLING = 3 << 12,
};

/*
  originSquare: bits 1-6 bits
  targetSquare: bits 7-12
  moveType: bits 13-14
  promotion: bits 15-17
*/
typedef int Move;

const int NONE_MOVE = 0;

constexpr Move createMove(int originSquare, int targetSquare)
{
  return Move(originSquare + (targetSquare << 6));
}

template <MoveType mt>
constexpr Move createMove(int originSquare, int targetSquare)
{
  return Move(originSquare + (targetSquare << 6) + mt);
}

template <MoveType mt>
constexpr Move createMove(int originSquare, int targetSquare, PieceType promotion)
{
  return Move(originSquare + (targetSquare << 6) + mt + (promotion << 14));
}

constexpr Move createMove(int originSquare, int targetSquare, PieceType promotion, MoveType mt)
{
  return Move(originSquare + (targetSquare << 6) + mt + (promotion << 14));
}

constexpr int originSquare(Move move)
{
  return move & 0x3f;
}

constexpr int targetSquare(Move move)
{
  return (move >> 6) & 0x3f;
}
constexpr MoveType moveType(Move move)
{
  return MoveType(move & 0x3000);
}

constexpr PieceType promotion(Move move)
{
  return PieceType(move >> 14);
}

std::string inline toUciString(Move move)
{
  return SQUARE_TO_ALGEBRAIC[originSquare(move)] + SQUARE_TO_ALGEBRAIC[targetSquare(move)] + (promotion(move) ? std::string(1, CharIndexToPiece[promotion(move)]) : "");
}
