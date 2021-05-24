#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/moveHelper.h"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
  auto start = std::chrono::high_resolution_clock::now();
  initConstants();
  Board board("k7/2b3b1/3P4/4R3/8/2b3b1/8/7K w - - 0 1");
  board.printBitboard(board.blockers(35, true, board.friendliesBB | board.enemiesBB));
  // board.printBitboard(board.friendliesBB);
  // std::cout << getPieceForSide<QUEEN>(false) << std::endl;
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}