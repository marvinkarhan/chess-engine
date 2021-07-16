#pragma once
#include "constants.h"
#include "move.h"
#include "time.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "../vendor/include/nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

// stores partial information about a board usefull for unmake move
struct StoredBoard
{
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide;
  BB epSquareBB;
  int fullMoves, halfMoves;
  Piece capturedPiece;
  u64 hashValue;
  Move move;
  RepetitionType repetition;

  StoredBoard *oldBoard; // board state before store
};

// information stored in the TT Hash Table
struct HashEntry
{
  u64 key;
  int depth;
  HashEntryFlag flag;
  int score;
  Move bestMove;
};

class Board
{

private:
  bool tableContainsKey(string moveKey, json openingTable);
  string getRandomMove(json openingTable);

public:
  // 6 diffrent Piece types + one for all pieces
  BB piecesByType[7];
  BB piecesBySide[2];
  Piece piecePos[64];
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished, stopSearch;
  u64 epSquareBB, nodeCount, hashTableHits = 0;
  int fullMoves, halfMoves, openingMoves;
  time_t endTime = LLONG_MAX;
  nlohmann::json currentOpeningTable;
  // quadratic pv table
  Move pvTable[MAX_DEPTH][MAX_DEPTH];
  std::vector<Move> latestPv;
  int pvLength[MAX_DEPTH];
  int ply = 0;
  int latestScore = 0;
  // indexed by [piece][targetSquare] (#15 because king = 14)
  // int historyHeuristicTable[15][64] = {0};
  // killer moves
  ValuedMove killerMoves[MAX_DEPTH][2];
  /* Saves values of pieces on the board */
  int pieceValues = 0;
  int pieceSquareValues = 0;
  StoredBoard *state;
  int hashTableSize;
  HashEntry *hashTable;
  int calls;
  int overwrites;
  inline BB pieces(bool activeSide, PieceType pt = ALL_PIECES)
  {
    return piecesBySide[activeSide] & piecesByType[pt];
  }
  inline BB pieces(Piece piece)
  {
    return piecesBySide[getPieceSide(piece)] & piecesByType[getPieceType(piece)];
  }
  inline BB pieces(PieceType piece)
  {
    return piecesByType[piece];
  }
  int negaMax(int depth, int alpha, int beta, bool nullMoveAllowed =true);
  int iterativeDeepening(time_t timeInSeconds = LLONG_MAX, int maxDepth = MAX_DEPTH);
  int quiesce(int alpha, int beta, int depth = 0);
  int evaluate();
  int evaluteMobility();
  int evaluateNextMove();
  Board(FenString fen = START_POS_FEN);
  ~Board();
  void initHashTableSize(int sizeInMB = 32);
  u64 getHash();
  void resetBoard();
  void printBitboard(BB bb);
  void prettyPrint();
  FenString toFenString();
  void printEveryPiece();
  BB allPiecesBB();
  void parseFenString(FenString fen);
  BB pieceMoves(PieceType type, bool activeSide);
  BB potentialSlidingAttackers(int square, bool activeSide);
  BB attackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB blockers(int square, bool activeSide, BB occupied);
  std::vector<Move> getPV();
  ValuedMove *generatePseudoLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category);
  ValuedMove *generateLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category);
  bool moveIsLegal(const Move move, bool activeSide, BB blockers, int kingSquare);
  bool moveIsPseudoLegal(const Move move);
  inline bool isKingAttacked()
  {
    return bool(kingAttackers());
  }
  inline BB kingAttackers()
  {
    return attackers(DEBRUIJN_INDEX[((pieces(activeSide, KING) ^ (pieces(activeSide, KING) - 1)) * DEBRUIJN) >> 58], activeSide, piecesByType[ALL_PIECES]);
  }
  bool hasRepetitions();
  bool partialStalemate();
  bool stalemate();
  bool checkmate();
  auto getMovesTree(int depth);
  u64 perft(int depth);
  std::string divide(int depth);
  void store(Piece captuedPiece = NO_PIECE);
  void restore();
  int probeHash(int depth, int alpha, int beta, Move *bestMove);
  void storeHash(int depth, int score, Move move, HashEntryFlag hashFlag);
  void zobristToggleCastle(u64 &hashValue);
  // to simplify updating piece positions
  constexpr HashEntry *probeHash()
  {
    return &hashTable[state->hashValue % hashTableSize];
  }

  int countHashTableSize()
  {
    int count = 0;
    for (int i = 0; i < hashTableSize; i++)
    {
      if (hashTable[i].key)
      {
        count++;
      }
    }
    return count;
  }
  inline void createPiece(Piece piece, int targetSquare)
  {
    piecePos[targetSquare] = piece;
    BB targetBB = SQUARE_BBS[targetSquare];
    piecesByType[ALL_PIECES] |= targetBB;
    piecesByType[getPieceType(piece)] |= targetBB;
    piecesBySide[getPieceSide(piece)] |= targetBB;
    pieceValues += PieceValues[piece];
    pieceSquareValues += PIECE_SQUARE_TABLES[piece][63 - targetSquare] * (getPieceSide(piece) ? 1 : -1);
  }
  inline void updatePiece(int originSquare, int targetSquare)
  {
    Piece piece = piecePos[originSquare];
    piecePos[originSquare] = NO_PIECE;
    piecePos[targetSquare] = piece;
    BB bb = SQUARE_BBS[originSquare] | SQUARE_BBS[targetSquare];
    piecesByType[ALL_PIECES] ^= bb;
    piecesByType[getPieceType(piece)] ^= bb;
    piecesBySide[getPieceSide(piece)] ^= bb;
    pieceSquareValues += (PIECE_SQUARE_TABLES[piece][63 - targetSquare] * (getPieceSide(piece) ? 1 : -1)) - (PIECE_SQUARE_TABLES[piece][63 - originSquare] * (getPieceSide(piece) ? 1 : -1));
  }
  inline void deletePiece(int targetSquare)
  {
    Piece piece = piecePos[targetSquare];
    piecePos[targetSquare] = NO_PIECE;
    BB targetBB = SQUARE_BBS[targetSquare];
    piecesByType[ALL_PIECES] ^= targetBB;
    piecesByType[getPieceType(piece)] ^= targetBB;
    piecesBySide[getPieceSide(piece)] ^= targetBB;
    pieceValues -= PieceValues[piece];
    pieceSquareValues -= PIECE_SQUARE_TABLES[piece][63 - targetSquare] * (getPieceSide(piece) ? 1 : -1);
  }
  bool makeMove(const Move &newMove);
  void unmakeMove(const Move &oldMove);
  void makeNullMove();
  void unmakeNullMove();
  /* for debugging */
  void printStateHistory()
  {
    StoredBoard *currState = state;
    int total = 0;
    while (currState)
    {
      currState = currState->oldBoard;
      total++;
    }
    std::cout << "Current states stored in history: " << std::to_string(total) << std::endl;
  }
  int getStateHistory()
  {
    StoredBoard *currState = state;
    int total = 0;
    while (currState)
    {
      currState = currState->oldBoard;
      total++;
    }
    return total;
  }
};

template <MoveGenType moveType>
struct MoveList
{
  ValuedMove moves[MAX_MOVES], *last;
  MoveList(Board &board, bool activeSide, MoveGenCategory category = ALL)
  {
    if constexpr (moveType == PSEUDO_LEGAL_MOVES)
      last = board.generatePseudoLegalMoves(moves, activeSide, category);
    else if constexpr (moveType == LEGAL_MOVES) {
      last = board.generateLegalMoves(moves, activeSide, category);
    }
  }
  // implement iterator pattern
  const ValuedMove *begin() const { return moves; }
  const ValuedMove *end() const { return last; }
  size_t size() const { return last - moves; }
};