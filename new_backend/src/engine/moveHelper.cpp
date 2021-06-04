#include "movehelper.h"
#include "board.h"
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

Move uciToMove(std::string uci, const Board &board)
{
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

  // u can still pass Invalid moves, but not everything is checked
  if (!(originSquare || targetSquare) || (board.piecePos[originSquare] == NO_PIECE))
  {
    std::cout << "Invalid uci move"  << std::endl;
    throw;
  }
  // possible promotion
  ss >> character;
  return Move(originSquare, targetSquare, getMoveType(originSquare, targetSquare, isdigit(character), board), isdigit(character) ? NO_PIECE : Piece(CharIndexToPiece.find(character)));
}

MoveType getMoveType(int originSquare, int targetSquare, bool isDigit, const Board &board)
{
  PieceType movedPieceType = getPieceType(board.piecePos[originSquare]);
  if (movedPieceType == PAWN)
  {
    if (!isDigit)
      return PROMOTION; // found a promotion letter
    if (board.epSquareBB == SQUARE_BBS[targetSquare])
      return EN_PASSANT; // found en passant square
  }
  if (movedPieceType == KING && REY_BBS[originSquare][targetSquare] > 0)
  {
    return CASTLING; // king moves more than 1 square
  }
  return NORMAL;
}

BB getPotentialEPSquareBB(int originSquare, int targetSquare, const Board &board)
{
  BB originSquareBB = SQUARE_BBS[originSquare];
  BB targetSquareBB = SQUARE_BBS[targetSquare];
  BB movedUpx2 = originSquareBB & RANK_2 && targetSquareBB & RANK_4;
  BB movedDownx2 = originSquareBB & RANK_7 && targetSquareBB & RANK_5;
  if (movedUpx2 || movedDownx2)
  {
    Piece leftSquarePiece = board.piecePos[bitScanForward(move(targetSquareBB, LEFT))];
    Piece rightSquarePiece = board.piecePos[bitScanForward(move(targetSquareBB, RIGHT))];
    Piece enemyPawnKey = board.activeSide ? BLACK_PAWN : WHITE_PAWN;
    if (leftSquarePiece == enemyPawnKey || rightSquarePiece == enemyPawnKey)
      return move(targetSquareBB & RANK_4, DOWN) | move(targetSquareBB & RANK_5, UP);
  }
  return 0ULL;
}