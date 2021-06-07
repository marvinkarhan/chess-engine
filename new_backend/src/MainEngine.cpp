#include "engine/constants.h"
#include "engine/move.h"
#include "engine/board.h"
#include "engine/movehelper.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

const std::string KIWI_PETE_POS_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const u64 KIWI_PETE_RESULTS[6] = {48, 2039, 97862, 4085603, 193690690, 8031647685};

void testNegaMax(Board &board, int depth)
{
  int eval = board.negaMax(depth, -2000000, 2000000);
  std::cout << "evaluation: " << std::to_string(eval) << std::endl;
  std::cout << "moves: ";
  for (Move move : board.getPV())
    std::cout << toUciString(move) << " ";
  std::cout << std::endl;
}

template <MoveGenType moveType>
void testMoveGen(Board &board, MoveGenCategory category = ALL)
{
  MoveList<moveType> legalMoves(board, board.activeSide, category);
  std::cout << "size: " + std::to_string(legalMoves.size()) << std::endl;
  for (Move move : legalMoves)
  {
    std::cout << CharIndexToPiece[board.piecePos[originSquare(move)]] << ": " << toUciString(move) + " ";
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
void benchmarkNegaMax()
{

  Board board;
  int maxMoves = 50;
  int depth = 4;
  cout << "Benchmark normal negaMax with " << maxMoves << " moves on depth: " << depth << endl;
  auto start = std::chrono::high_resolution_clock::now();
  double peak;
  FenString peakFen;
  for (int i = 0; i < maxMoves; i++)
  {
    auto newNegaMax = std::chrono::high_resolution_clock::now();
    int eval = board.negaMax(depth, -2000000, 2000000);
    auto newNegaMaxFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedNegaMax = newNegaMaxFinish - newNegaMax;
    Move nextMove = board.getPV()[0];
    board.makeMove(nextMove);
    if (elapsedNegaMax.count() > peak)
    {
      peak = elapsedNegaMax.count();
      peakFen = board.toFenString();
    }
    cout << "Move Nr.: " << right << std::setfill('0') << std::setw(5) << i << " move: " << toUciString(nextMove) << " fen: " << left << setfill('-') << std::setw(80) << board.toFenString() << " " << left << setfill('0') << setw(10) << elapsedNegaMax.count() << " seconds" << endl;
  }
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- BENCHMARK RUNTIME: " << elapsed.count() << " seconds ---" << std::endl;
  std::cout << "\r\n--- AVG RUNTIME PER CALL: " << elapsed.count() / maxMoves << " seconds ---" << std::endl;
  std::cout << "\r\n--- LONGEST NEGA MAX CALL: " << peak << " seconds, fen: " << peakFen << std::endl;
}
void testZobrist()
{
  // Test if ZOBRIST CONTAINS duplicates, if so you must change the seed value!
  for (int i = 0; i < ZOBRIST_ARRAY_LENGTH; i++)
  {
    u64 test = ZOBRIST_TABLE[i];
    for (int j = i + 1; j < ZOBRIST_ARRAY_LENGTH; j++)
    {
      u64 checkDuplicate = ZOBRIST_TABLE[j];
      if (test == checkDuplicate)
        cout << "Found duplicate: " << test << "=" << checkDuplicate << "!"
             << "with i: " << i << " and j: " << j << endl;
    }
  }
  cout << "Generate some zobrist keys from start!" << endl;
  Board board;
  int maxMoves = 70;
  int depth = 2;
  cout << "Make some moves with negaMax on depth: " << depth << endl;
  vector<FenString> fens;
  // Test values for checking if the testing works!!
  // fens.push_back("123");
  // fens.push_back("1234");
  // fens.push_back("123");
  // fens.push_back("12345");
  vector<u64> hashes;
  // hashes.push_back(123);
  // hashes.push_back(1);
  // hashes.push_back(12);
  // hashes.push_back(1);
  for (int i = 0; i < maxMoves; i++)
  {
    int eval = board.negaMax(depth, -2000000, 2000000);
    Move nextMove = board.getPV()[0];
    board.makeMove(nextMove);
    cout << "Move Nr.: " << std::setfill('0') << std::setw(5) << i << ", move: " << toUciString(nextMove) << ", fen: " << setfill('-') << std::setw(100) << board.toFenString() << ", hash: " << board.hashValue << endl;
    hashes.push_back(board.hashValue);
    fens.push_back(board.toFenString());
  }

  cout << "Check equal and unequal fens and their hashes" << endl;
  bool equal = true;
  bool different = true;
  for (int i = 0; i < fens.size(); i++)
  {
    for (int j = i + 1; j < fens.size(); j++)
    {
      if (fens[i] == fens[j])
      {
        if (hashes[i] != hashes[j])
        {
          cout << "ERROR: DIFFERENT HASHES BUT THEY SHOULD BE EQUAL" << endl;
          cout << "fen on " << i << ": " << fens[i] << " === " << fens[j] << " on: " << j << endl;
          cout << "hash on " << i << ": " << hashes[i] << " === " << hashes[j] << " on: " << j << endl;
          equal = false;
        }
      }
      else
      {
        if (hashes[i] == hashes[j])
        {
          cout << "ERROR: EQUAL HASHES BUT THEY SHOULD BE DIFFERENT" << endl;
          cout << "fen on " << i << ": " << fens[i] << " !== " << fens[j] << " on: " << j << endl;
          cout << "hash on " << i << ": " << hashes[i] << " !== " << hashes[j] << " on: " << j << endl;
          different = false;
        }
      }
    }
  }
  if (!equal)
  {
    cout << "Hashing is incorrect! At least one hash with the same fen position was different!!!" << endl;
  }
  if (!different)
  {
    cout << "Hashing is incorrect! At least one hash with different fen position was equal!!!" << endl;
  }
  if (equal && different)
  {
    cout << "HASHING WORKS CORRECTLY!!" << endl;
  }
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

  // Board board("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
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

  // std::cout << "Board hash:" << board.hashValue << std::endl;
  // Move move1 = uciToMove("g1f3", board);
  // board.makeMove(move1);
  // std::cout << "Board hash:" << board.hashValue << std::endl;
  // Move move2 = uciToMove("g8f6", board);
  // board.makeMove(move2);
  // std::cout << "Board hash:" << board.hashValue << std::endl;
  // board.unmakeMove(move2);
  // std::cout << "Board hash:" << board.hashValue << std::endl;
  // board.unmakeMove(move1);
  // std::cout << "Board hash:" << board.hashValue << std::endl;

  // perft(3, 8902);
  // perft(4, 197281);
  // perft(5, 4865609);
  // perft(5, KIWI_PETE_RESULTS[4], KIWI_PETE_POS_FEN);

  // divide(5);
  // divide(5, KIWI_PETE_POS_FEN);
  // divide(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/1R2K2R b Kkq - 1 1");
  // divide(3, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q2/PPPBBPpP/1R2K2R w Kkq - 0 2");
  // divide(2, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/1R2K2R b Kkq - 0 2");

  testNegaMax(board, 7);
  std::cout << "PV Node: ";
  for (Move move : board.getPV())
  {
    std::cout << toUciString(move) + " ";
  }
  std::cout << std::endl;
  std::cout << "Hash table size: " << board.hashTableSize << std::endl;
  std::cout << "Hash table hits: " << board.hashTableHits << std::endl;

  // testZobrist();
  // benchmarkNegaMax();

  // std::cout << std::to_string(board.evaluate()) << std::endl;

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}