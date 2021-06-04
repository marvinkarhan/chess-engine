#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <chrono>
#include <string>
#include <cassert>

const std::string KIWI_PETE_POS_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const u64 KIWI_PETE_RESULTS[6] = {48, 2039, 97862, 4085603, 193690690, 8031647685};

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
  std::cout << std::endl;
}

void perft(int depth, u64 result, std::string fen = START_POS_FEN)
{
  Board board(fen);
  u64 pertResult = board.perft(depth);
  std::cout << "Perft " << depth << " on " << fen << std::endl;
  if (pertResult == result)
    std::cout << "Is equal" << std::endl;
  else
    std::cout << pertResult << " !== " << result << std::endl;
}

void divide(int depth, std::string fen = START_POS_FEN)
{
  Board board(fen);
  std::cout << "Divide " << depth << " on " << fen << std::endl;
  std::cout << board.divide(depth);
}

int main(int argc, char *argv[])
{
  initConstants();

  // Board board("k7/8/8/8/8/8/8/4K2R w K - 0 1");
  // Board board(KIWI_PETE_POS_FEN);
  Board board;
  auto start = std::chrono::high_resolution_clock::now();
  // std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  // testMoveGen<PSEUDO_LEGAL_MOVES>(board);
  // std::cout << "LEGAL_MOVES" << std::endl;
  // testMoveGen<LEGAL_MOVES>(board);
  
  // board.makeMove(uciToMove("e7e5", board));
  // board.makeMove(uciToMove("d2d3", board));
  // board.makeMove(uciToMove("d8e7", board));
  // board.makeMove(uciToMove("d1d2", board));
  // board.printBitboard(board.piecesByType[ALL_PIECES]);
  // std::cout << "fen: " << board.toFenString() << std::endl;
  // board.printEveryPiece();
  // board.printBitboard(board.allPiecesBB());

  
  // Move move1 = uciToMove("e1d1", board);
  // board.makeMove(move1);
  // Move move2 = uciToMove("a8a7", board);
  // board.makeMove(move2);
  // board.unmakeMove(move2);
  // board.makeMove(move2);
  // testMoveGen<LEGAL_MOVES>(board);

  // perft(3, 8902);
  // perft(4, 197281);
  // perft(5, 4865609);
  // perft(5, KIWI_PETE_RESULTS[4], KIWI_PETE_POS_FEN);

  // divide(5);
  // divide(5, KIWI_PETE_POS_FEN);
  // divide(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/1R2K2R b Kkq - 1 1");
  // divide(3, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q2/PPPBBPpP/1R2K2R w Kkq - 0 2");
  // divide(2, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/1R2K2R b Kkq - 0 2");


  testNegaMax(board, 6);

  // std::cout << std::to_string(board.evaluate()) << std::endl;

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}