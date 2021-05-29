#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <chrono>
#include <string>

void testNegaMax(Board &board, int depth)
{
  Evaluation eval = board.negaMax(depth, INT_MIN, INT_MAX);
  std::cout << "evaluation: " << std::to_string(eval.evaluation) << std::endl;
  std::cout << "moves: ";
  for (int i = 0; i < depth; i++)
  {
    std::cout << eval.moves[i] << " ";
  }
  std::cout << std::endl;
}

template<MoveGenType moveType>
void testMoveGen(Board &board)
{
  MoveList<moveType> legalMoves(board, board.activeSide);
  std::cout << "size: " + std::to_string(legalMoves.size()) << std::endl;
  for (Move move: legalMoves)
  {
    std::cout << move.to_uci_string() + " ";
  }
}

int main(int argc, char *argv[])
{
  initConstants();

  Board board("rnbqkb1r/1ppppppp/p4n2/4P3/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3");
  // Board board;
  
  auto start = std::chrono::high_resolution_clock::now();
  // std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  // testMoveGen<PSEUDO_LEGAL_MOVES>(board);
  // std::cout << "LEGAL_MOVES" << std::endl;
  // testMoveGen<LEGAL_MOVES>(board);

  // board.makeMove(uciToMove("g1f3"));
  // board.makeMove(uciToMove("e7e5"));
  // board.makeMove(uciToMove("d2d3"));
  // board.makeMove(uciToMove("d8e7"));
  // board.makeMove(uciToMove("d1d2"));
  // board.printBitboard(board.piecesByType[ALL_PIECES]);
  // std::cout << "fen: " << board.toFenString() << std::endl;
  // board.printEveryPiece();
  // board.printBitboard(board.allPiecesBB());

  testNegaMax(board, 5);

  // std::cout << std::to_string(board.evaluate()) << std::endl;

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}