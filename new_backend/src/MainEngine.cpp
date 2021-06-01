#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <chrono>
#include <string>
#include <cassert>

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

void perft(int depth, u64 result, std::string fen = START_POS_FEN)
{
  Board board(fen);
  u64 pertResult = board.perft(depth);
  std::cout << "Perft " << depth << " on " << fen << std::endl;
  std::cout << pertResult << ((pertResult == result) ? " == " : " !== ") << result;
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

  Board board("rnb1kbnr/pp1ppppp/2p5/q7/8/P2P4/1PP1PPPP/RNBQKBNR w KQkq - 1 3");
  // Board board;
  auto start = std::chrono::high_resolution_clock::now();
  // std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  // testMoveGen<PSEUDO_LEGAL_MOVES>(board);
  // std::cout << "\r\nLEGAL_MOVES" << std::endl;
  // testMoveGen<LEGAL_MOVES>(board);

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

  perft(5, 4865609ULL);

  // divide(5);
  // divide(4, "rnbqkbnr/pppppppp/8/8/8/3P4/PPP1PPPP/RNBQKBNR b KQkq - 0 1");
  // divide(3, "rnbqkbnr/pp1ppppp/2p5/8/8/3P4/PPP1PPPP/RNBQKBNR w KQkq - 0 2");
  // divide(2, "rnbqkbnr/pp1ppppp/2p5/8/8/P2P4/1PP1PPPP/RNBQKBNR b KQkq - 0 2");
  // divide(1, "rnb1kbnr/pp1ppppp/2p5/q7/8/P2P4/1PP1PPPP/RNBQKBNR w KQkq - 1 3");
  

  // testNegaMax(board, 6);

  // std::cout << std::to_string(board.evaluate()) << std::endl;

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}