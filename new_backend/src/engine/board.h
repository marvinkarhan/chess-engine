#pragma once
#include "constants.h"
#include "move.h"
#include <iostream>
#include <string>
#include "../vendor/include/nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

struct Evaluation
{
  int evaluation;
  vector<string> moves;
};

struct PVariation
{
  int len = 0;
  Move moves[MAX_MOVES];
};

// stores partial information about a board usefull for unmake move
struct StoredBoard
{
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide;
  BB epSquareBB;
  int fullMoves, halfMoves;
  Piece capturedPiece;
  u64 hashValue;

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
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
  u64 epSquareBB, hashValue, hashTableHits = 0;
  int fullMoves, halfMoves, openingMoves;
  nlohmann::json currentOpeningTable;
  /* Saves values of pieces on the board */
  int pieceValues = 0;
  int pieceSquareValues = 0;
  PVariation mateMoves;
  StoredBoard *state;
  int hashTableSize;
  HashEntry *hashTable;
  void initHashTableSize(int sizeInMB = 32);
  inline BB pieces(bool activeSide, PieceType pt = ALL_PIECES)
  {
    return piecesBySide[activeSide] & piecesByType[pt];
  }
  inline BB pieces(Piece piece)
  {
    return piecesBySide[getPieceSide(piece)] & piecesByType[getPieceType(piece)];
  }
  int negaMax(int depth, int alpha, int beta, PVariation *pVariation);
  int iterativeDeepening(int timeInSeconds, PVariation *pVariation);
  int quiesce(int alpha, int beta, PVariation *pVariation, int depth = 0);
  int evaluate();
  int evaluateNextMove(int depth, string lastMove, PVariation *pVariation);
  Board(FenString fen = START_POS_FEN);
  ~Board();
  u64 getHash();
  void resetBoard();
  void printBitboard(BB bb);
  Evaluation evaluateMoves(int depth, string lastMove);
  FenString toFenString();
  void printEveryPiece();
  BB allPiecesBB();
  void parseFenString(FenString fen);
  BB potentialSlidingAttackers(int square, bool activeSide);
  BB attackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB blockers(int square, bool activeSide, BB occupied);
  Move *generatePseudoLegalMoves(Move *moveList, bool activeSide, MoveGenCategory category);
  Move *generateLegalMoves(Move *moveList, bool activeSide, MoveGenCategory category);
  bool moveIsLegal(const Move &move, bool activeSide, BB blockers, BB kingAttackersBB, int kingSquare, BB occupied);
  bool stalemate();
  bool stalemate(int moveListSize);
  bool checkmate();
  bool checkmate(int moveListSize);
  auto getMovesTree(int depth);
  u64 perft(int depth);
  std::string divide(int depth);
  void store(Piece captuedPiece = NO_PIECE);
  void restore();
  int probeHash(int depth, int alpha, int beta, Move *bestMove);
  void storeHash(int depth, int score, Move move, HashEntryFlag hashFlag);
  void zobristToggleCastle();
  // to simplify updating piece positions
  inline void createPiece(Piece piece, int targetSquare)
  {
    piecePos[targetSquare] = piece;
    BB targetBB = SQUARE_BBS[targetSquare];
    piecesByType[ALL_PIECES] |= targetBB;
    piecesByType[getPieceType(piece)] |= targetBB;
    piecesBySide[getPieceSide(piece)] |= targetBB;
    pieceValues += PieceValues[piece];
    pieceSquareValues += PIECE_SQUARE_TABLES[piece][63 - targetSquare] * (getPieceSide(piece) ? 1 : -1);
    hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[piece] + targetSquare];
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
    hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[piece] + originSquare] | ZOBRIST_TABLE[ZobristPieceOffset[piece] + targetSquare];
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
    hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[piece] + targetSquare];
  }
  bool makeMove(const Move &newMove);
  void unmakeMove(const Move &oldMove);
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
};

template <MoveGenType moveType>
struct MoveList
{
  Move moves[MAX_MOVES], *last;
  MoveList(Board &board, bool activeSide, MoveGenCategory category = ALL, Move bestMove = NONE_MOVE)
  {
    last = moves;
    if (bestMove != NONE_MOVE)
    {
      *last++ = bestMove;
    }
    if (moveType == PSEUDO_LEGAL_MOVES)
      last = board.generatePseudoLegalMoves(last, activeSide, category);
    else if (moveType == LEGAL_MOVES)
      last = board.generateLegalMoves(last, activeSide, category);
  }
  // implement iterator pattern
  const Move *begin() const { return moves; }
  const Move *end() const { return last; }
  size_t size() const { return last - moves; }
};