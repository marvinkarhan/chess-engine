#include "board.h"
#include <string>
#include <iostream>

Board::Board(FenString fen /*=START_POS_FEN*/)
{
  resetBoard();

  /* TODO: Opening init */
  openingFinished = false;

  // parseFenString(fen);
}

void Board::resetBoard()
{
  pieces = {
      {WHITE_PAWN, 0},
      {WHITE_ROOK, 0},
      {WHITE_KNIGHT, 0},
      {WHITE_BISHOP, 0},
      {WHITE_QUEEN, 0},
      {WHITE_KING, 0},
      {BLACK_PAWN, 0},
      {BLACK_ROOK, 0},
      {BLACK_KNIGHT, 0},
      {BLACK_BISHOP, 0},
      {BLACK_QUEEN, 0},
      {BLACK_QUEEN, 0}};

  castleWhiteKingSide = true;
  castleWhiteQueenSide = true;
  castleBlackKingSide = true;
  castleBlackQueenSide = true;
  friendliesBB = BB(0);
  enemiesBB = BB(0);
  hashValue = 0ULL;

  // 0: black, 1: white
  activeSide = true;
  fullMoves = 0;
  halfMoves = 0;

  epSquareBB = BB(0);
}

void Board::printBitboard(BB bb)
{
  std::string result = "";
  for (int i = 0; i < 8; i++)
  {
    std::string tmpLine = "";
    for (int j = 0; j < 8; j++)
    {
      tmpLine.insert(0, " " + std::to_string(bb & 1ULL));
      bb >>= 1;
    }
    tmpLine.erase(0, 1);
    result.insert(0, tmpLine + "\r\n");
  }
  std::cout << result;
}