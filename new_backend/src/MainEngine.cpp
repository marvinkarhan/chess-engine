#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/moveHelper.h"
#include <iostream>
#include <chrono>
#include <string>

int main(int argc, char *argv[])
{
  auto start = std::chrono::high_resolution_clock::now();
  initConstants();
  // Board board("1k6/8/8/8/6q1/8/8/R3K2R w KQ - 0 1");
  Board board;
  // std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  // MoveList<PSEUDO_LEGAL_MOVES> pseudoMoves(board, true);
  // std::cout << "size: " + std::to_string(pseudoMoves.size()) << std::endl;
  // for (Move move: pseudoMoves)
  // {
  //   std::cout << move.to_uci_string() + ", ";
  // }
  // std::cout << "\r\nLEGAL_MOVES" << std::endl;
  // MoveList<LEGAL_MOVES> legalMoves(board, true);
  // std::cout << "size: " + std::to_string(legalMoves.size()) << std::endl;
  // for (Move move: legalMoves)
  // {
  //   std::cout << move.to_uci_string() + ", ";
  // }
  board.makeMove(Move(1, 16));
  board.printBitboard(board.allPiecesBB());
  // std::cout << board.generatePseudoLegalMoves(true) << std::endl;
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}