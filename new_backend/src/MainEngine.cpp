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
  Board board;
  // board.printBitboard(board.whitePiecesBB());
  std::cout << (board.toFenString() == START_POS_FEN) << std::endl;
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}