#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <chrono>
#include <string>

void testNegaMax(Board &board, int depth)
{
  PVariation pVariation;
  int eval = board.negaMax(depth, -2000000, 2000000, &pVariation);
  std::cout << "evaluation: " << std::to_string(eval) << std::endl;
  std::cout << "moves: ";
  for (int i = 0; i < pVariation.len; i++)
    std::cout << pVariation.moves[i].to_uci_string() << " ";
  std::cout << std::endl;
}

template<MoveGenType moveType>
void testMoveGen(Board &board)
{
  MoveList<moveType> legalMoves(board, board.activeSide);
  std::cout << "size: " + std::to_string(legalMoves.size()) << std::endl;
  for (Move move: legalMoves)
  {
    std::cout << CharIndexToPiece[board.piecePos[move.originSquare]] << ": " << move.to_uci_string() + " ";
  }
}

int main(int argc, char *argv[])
{
  initConstants();

  Board board("k7/8/8/8/3q4/8/6K1/8 b - - 0 1");
  // Board board;
  auto start = std::chrono::high_resolution_clock::now();
  board.makeMove(uciToMove("d4g1"));
  std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  testMoveGen<PSEUDO_LEGAL_MOVES>(board);
  std::cout << "LEGAL_MOVES" << std::endl;
  testMoveGen<LEGAL_MOVES>(board);
  board.printBitboard(REY_BBS[9][1]);

  // board.makeMove(uciToMove("d7d5"));
  // bool isLegal = board.makeMove(uciToMove("g8f6"));
  // std::cout << "move is legal: " << isLegal << std::endl;

  // board.makeMove(uciToMove("g1f3"));
  // board.makeMove(uciToMove("e7e5"));
  // board.makeMove(uciToMove("d2d3"));
  // board.makeMove(uciToMove("d8e7"));
  // board.makeMove(uciToMove("d1d2"));
  // board.printBitboard(board.piecesByType[ALL_PIECES]);
  // std::cout << "fen: " << board.toFenString() << std::endl;
  // board.printEveryPiece();
  // board.printBitboard(board.allPiecesBB());

  // testNegaMax(board, 6);

  // std::cout << std::to_string(board.evaluate()) << std::endl;

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}