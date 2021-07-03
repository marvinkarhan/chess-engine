#include "engine/movepicker.h"
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

std::vector<std::string> positions{
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
    "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
    "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
    "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
    "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
    "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
    "4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
    "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
    "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
    "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
    "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
    "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
    "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
    "6k1/6p1/6Pp/ppp5/3pn2P/1P3K2/1PP2P2/3N4 b - - 0 1",
    "3b4/5kp1/1p1p1p1p/pP1PpP1P/P1P1P3/3KN3/8/8 w - - 0 1",
    "2K5/p7/7P/5pR1/8/5k2/r7/8 w - - 0 1",
    "8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
    "7k/3p2pp/4q3/8/4Q3/5Kp1/P6b/8 w - - 0 1",
    "8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
    "8/1p3pp1/7p/5P1P/2k3P1/8/2K2P2/8 w - - 0 1",
    "8/pp2r1k1/2p1p3/3pP2p/1P1P1P1P/P5KR/8/8 w - - 0 1",
    "8/3p4/p1bk3p/Pp6/1Kp1PpPp/2P2P1P/2P5/5B2 b - - 0 1",
    "5k2/7R/4P2p/5K2/p1r2P1p/8/8/8 b - - 0 1",
    "6k1/6p1/P6p/r1N5/5p2/7P/1b3PP1/4R1K1 w - - 0 1",
    "1r3k2/4q3/2Pp3b/3Bp3/2Q2p2/1p1P2P1/1P2KP2/3N4 w - - 0 1",
    "6k1/4pp1p/3p2p1/P1pPb3/R7/1r2P1PP/3B1P2/6K1 w - - 0 1",
    "8/3p3B/5p2/5P2/p7/PP5b/k7/6K1 w - - 0 1",
    "5rk1/q6p/2p3bR/1pPp1rP1/1P1Pp3/P3B1Q1/1K3P2/R7 w - - 93 90",
    "4rrk1/1p1nq3/p7/2p1P1pp/3P2bp/3Q1Bn1/PPPB4/1K2R1NR w - - 40 21",
    "r3k2r/3nnpbp/q2pp1p1/p7/Pp1PPPP1/4BNN1/1P5P/R2Q1RK1 w kq - 0 16",
    "3Qb1k1/1r2ppb1/pN1n2q1/Pp1Pp1Pr/4P2p/4BP2/4B1R1/1R5K b - - 11 40",
    "4k3/3q1r2/1N2r1b1/3ppN2/2nPP3/1B1R2n1/2R1Q3/3K4 w - - 5 1",
};

void testNegaMax(Board &board, int depth)
{
  // int eval = board.negaMax(depth, -2000000, 2000000);
  int eval = board.iterativeDeepening(0, depth);
  std::cout << "evaluation: " << std::to_string(eval) << std::endl;
  std::cout << "moves: ";
  for (Move move : board.getPV())
    std::cout << toUciString(move) << " ";
  std::cout << std::endl;
  board.makeMove(board.getPV()[0]);
}

template <MoveGenType moveType>
void testMoveGen(Board &board, MoveGenCategory category = ALL)
{
  MoveList<moveType> legalMoves(board, board.activeSide, category);
  std::cout << "size: " + std::to_string(legalMoves.size()) << std::endl;
  for (ValuedMove move : legalMoves)
  {
    std::cout << CharIndexToPiece[board.piecePos[originSquare(move)]] << ": " << toUciString(move) << " ";
  }
  std::cout << std::endl;
}

void testMovePicker(Board &board, bool attacksOnly = false)
{
  MovePicker movePicker(board, NONE_MOVE, attacksOnly);
  Move move;
  int count = 0;
  while ((move = movePicker.nextMove()) != NONE_MOVE)
  {
    std::cout << CharIndexToPiece[board.piecePos[originSquare(move)]] << ": " << toUciString(move) << " ";
    count++;
  }
  std::cout << std::endl
            << "size: " << count << std::endl;
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
void benchmarkNegaMax(int depth, int maxMoves)
{
  Board board;
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
    Move nextMove = board.hashTable[board.hashValue % board.hashTableSize].bestMove;
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
  for (size_t i = 0; i < fens.size(); i++)
  {
    for (size_t j = i + 1; j < fens.size(); j++)
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

void testPopCount()
{
  cout << "TEST POP COUNT" << endl;
  Board board;
  BB testBoard = 1ULL;
  board.printBitboard(testBoard);
  cout << "GREATER THAN 1 ?" << pop_last_bb(testBoard) << endl;
  testBoard <<= 63;
  board.printBitboard(testBoard);
  cout << "GREATER THAN 1 ? " << pop_last_bb(testBoard) << endl;
  testBoard |= 1ULL;
  board.printBitboard(testBoard);
  cout << "GREATER THAN 1 ? " << pop_last_bb(testBoard) << endl;
  testBoard |= Mask::FULL;
  board.printBitboard(testBoard);
  cout << "GREATER THAN 1 ? " << pop_last_bb(testBoard) << endl;
}

void divide(int depth, std::string fen = START_POS_FEN)
{
  Board board(fen);
  std::cout << "Divide " << depth << " on " << fen << std::endl;
  std::cout << board.divide(depth);
}

void effectiveBranchingFactor()
{
  int depth = 6;
  Board board;
  // nodecount index by ply
  float totalBranchingFactor = 0;
  u64 totalNodes = 0;
  for (auto position : positions)
  {
    board.parseFenString(position);
    std::vector<int> iterationNodeCounts;
    for (size_t i = 1; i <= depth; i++)
    {
      board.nodeCount = 0;
      board.negaMax(i, MIN_ALPHA, MIN_BETA);
      iterationNodeCounts.push_back(board.nodeCount);
      totalNodes += board.nodeCount;
    }
    // average out branching factor
    float branchingFactor = 0;
    for (size_t i = 0; i < depth - 1; i++)
    {
      branchingFactor += iterationNodeCounts[i + 1] / iterationNodeCounts[i];
    }
    branchingFactor /= depth;
    std::cout << "EBF: " << branchingFactor << ", on: " << position << std::endl;
    totalBranchingFactor += branchingFactor;
  }
  totalBranchingFactor /= positions.size();
  std::cout << std::endl
            << "--- total nodes: " << totalNodes << " ---" << std::endl;
  std::cout << std::endl
            << "--- total branching factor for: " << totalBranchingFactor << " ---" << std::endl;
}

int main(int argc, char *argv[])
{
  initConstants();

  // Board board("rnbqkb1r/ppNppppp/5n2/8/8/1P6/P1PPPPPP/R1BQKBNR b KQkq - 0 4");
  // Board board(KIWI_PETE_POS_FEN);
  Board board;
  auto start = std::chrono::high_resolution_clock::now();

  // BB attackFields = board.pieceMoves(PieceType::KNIGHT,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::BISHOP,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::ROOK,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::QUEEN,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::KING,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::PAWN,board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::BISHOP,!board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::ROOK,!board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::QUEEN,!board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::KING,!board.activeSide);
  // board.printBitboard(attackFields);
  // attackFields = board.pieceMoves(PieceType::PAWN,!board.activeSide);
  // board.printBitboard(attackFields);
  // std::cout << "PSEUDO_LEGAL_MOVES" << std::endl;
  // testMoveGen<PSEUDO_LEGAL_MOVES>(board);
  // std::cout << "LEGAL_MOVES" << std::endl;
  // testMoveGen<LEGAL_MOVES>(board);
  // std::cout << "PSEUDO_LEGAL_MOVES - EVASIONS" << std::endl;
  // testMoveGen<PSEUDO_LEGAL_MOVES>(board, EVASIONS);
  // std::cout << "LEGAL_MOVES - EVASIONS" << std::endl;
  // testMoveGen<LEGAL_MOVES>(board, EVASIONS);

  // std::cout << "MovePicker" << std::endl;
  // testMovePicker(board);
  // std::cout << "MovePicker attacksOnly (only good/equal attacks)" << std::endl;
  // testMovePicker(board, true);

  // testNegaMax(board, 7);
  // cout << toUciString(board.hashTable[board.hashValue % board.hashTableSize].bestMove) << endl;
  // std::cout << "Hash table size: " << board.hashTableSize << std::endl;
  // std::cout << "Hash table fill: " << board.countHashTableSize() << std::endl;
  // std::cout << "Hash table fill: " << (float)board.countHashTableSize() / board.hashTableSize * 100 << "%" << std::endl;
  // std::cout << "Hash table overwrites: " << board.overwrites << std::endl;
  // std::cout << "Hash table hits: " << board.hashTableHits << std::endl;

  // testZobrist();
  // perft(6, KIWI_PETE_RESULTS[5], KIWI_PETE_POS_FEN);
  // perft(6, 119060324, START_POS_FEN);

  // divide(5);
  // divide(5, KIWI_PETE_POS_FEN);

  // effectiveBranchingFactor();
  

  // testZobrist();
  benchmarkNegaMax(7, 50);

  // std::cout << "fen: " << board.toFenString() << std::endl;
  // std::cout << "eval: " << std::to_string(board.evaluate()) << std::endl;

  // testPopCount();

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
  return 0;
}