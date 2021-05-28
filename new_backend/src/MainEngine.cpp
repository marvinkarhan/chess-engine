#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <chrono>
#include <string>

int main(int argc, char *argv[])
{
  auto start = std::chrono::high_resolution_clock::now();
  initConstants();
  // Board board("8/7P/k7/8/8/8/8/7K w - - 0 1");
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
  // std::cout << uciToMove("g1h3").to_uci_string() << std::endl;
  board.makeMove(uciToMove("e2e4"));
  board.printBitboard(board.allPiecesBB());
  // #Evaluation eval = board.negaMax(1, -2000, 20000);
  // std::cout << eval.evaluation << ", " << eval.moves[0] << " "<< eval.moves[1] << " " << eval.moves[2] << " " << eval.moves[3] << endl;
  // std::cout << board.generatePseudoLegalMoves(true) << std::endl;
  // Evaluation eval = board.evaluateNextMove(3, "e2e4");
  // std::cout << eval.evaluation << "," << eval.moves[0] << endl;
  // eval = board.evaluateNextMove(3, "d2d4");
  // std::cout << eval.evaluation << "," << eval.moves[0] << endl;
  // board.printBitboard(board.blockers(35, true, board.friendliesBB | board.enemiesBB));
  // board.printBitboard(board.friendliesBB);
  // std::cout << getPieceForSide<QUEEN>(false) << std::endl;
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}