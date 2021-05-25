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
  // Board board("k7/7P/8/8/8/8/8/7K w - - 0 1");
  Board board;
  // board.printBitboard(board.blockers(35, true, board.friendliesBB | board.enemiesBB));
  // board.printBitboard(board.friendliesBB);
  MoveList<PSEUDO_LEGAL_MOVES> moves(board, true);
  std::cout << "size: " + std::to_string(moves.size()) << std::endl;
  for (Move move: moves)
  {
    std::cout << move.to_uci_string() + ", ";
  }
  // std::cout << board.pseudoLegalMovesGenerator(true) << std::endl;
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}