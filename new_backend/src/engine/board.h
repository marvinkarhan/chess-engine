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

  StoredBoard *oldBoard; // board state before store
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
  u64 epSquareBB, hashValue;
  int fullMoves, halfMoves, openingMoves;
  nlohmann::json currentOpeningTable;
  /* Saves values of pieces on the board */
  int pieceValues = 0;
  int pieceSquareValues = 0;
  StoredBoard *state;
  inline BB pieces(bool activeSide, PieceType pt = ALL_PIECES)
  {
    return piecesBySide[activeSide] & piecesByType[pt];
  }
  inline BB pieces(Piece piece)
  {
    return piecesBySide[getPieceSide(piece)] & piecesByType[getPieceType(piece)];
  }
  int negaMax(int depth, int alpha, int beta, PVariation *pVariation);
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
  Move *generatePseudoLegalMoves(Move *moveList, bool activeSide, bool onlyEvasions = false);
  Move *generateLegalMoves(Move *moveList, bool activeSide);
  bool moveIsLegal(const Move &move, bool activeSide, BB blockers, BB kingAttackersBB, int kingSquare, BB occupied);
  bool stalemate();
  bool checkmate();
  auto getMovesTree(int depth);
  u64 perft(int depth);
  std::string divide(int depth);
  void store(Piece captuedPiece = NO_PIECE);
  void restore();
  void hash();
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
  /* for debugging */
  void printStateHistory()
  {
    StoredBoard *currState = state;
    int total = 0;
    while(currState)
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
  MoveList(Board &board, bool activeSide, bool onlyEvasions = false)
  {
    if (moveType == PSEUDO_LEGAL_MOVES)
      last = board.generatePseudoLegalMoves(moves, activeSide, onlyEvasions);
    else if (moveType == LEGAL_MOVES)
      last = board.generateLegalMoves(moves, activeSide);
  }
  // implement iterator pattern
  const Move *begin() const { return moves; }
  const Move *end() const { return last; }
  size_t size() const { return last - moves; }
};