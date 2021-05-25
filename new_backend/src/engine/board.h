#pragma once
#include "constants.h"
#include <string>
#include <map>
#include "move.h"

using namespace std;

struct Evaluation
{
  int evaluation;
  string *moves;
};

struct StoredBoard
{
  std::map<Piece, BB> pieces;
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide;
  BB friendlies, enemies, epSquare;
  int fullMoves, halfMoves;
};

inline void addMovesToList(Evaluation &bestEvaluation, Evaluation &newEvaluation, int depth, Move move)
{
  //Add moves from the depth -1 array
  for (int i = 0; i < depth - 1; i++)
  {
    bestEvaluation.moves[i + 1] = newEvaluation.moves[i];
  }
  //Add the current move to the front
  bestEvaluation.moves[0] = move.to_uci_string();
}

class Board
{
public:
  std::map<Piece, BB> pieces;
  /** current_opening_table **/
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
  u64 friendliesBB, enemiesBB, epSquareBB, hashValue;
  int fullMoves, halfMoves, openingMoves;
  Evaluation negaMax(int depth, int alpha, int beta);
  int evaluate();

public:
  Board(FenString fen = START_POS_FEN);
  u64 getHash();
  void resetBoard();
  void printBitboard(BB bb);
  Evaluation evaluateMoves(int depth, string lastMove);
  FenString toFenString();
  void printEveryPiece();
  BB whitePiecesBB();
  BB blackPiecesBB();
  BB allPiecesBB();
  BB *getActivePieces(bool activeSide);
  template <PieceType pt>
  BB getPieceForSide(bool activeSide);
  Piece getPieceOnSquare(BB bb);
  void parseFenString(FenString fen);
  BB potentialAttackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB attackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB blockers(int square, bool activeSide, BB occupied);
  Move *generatePseudoLegalMoves(Move *moveList, bool activeSide, bool onlyEvasions = false);
  Move *generateLegalMoves(Move *moveList, bool activeSide);
  bool moveIsLegal(const Move &move, bool activeSide, BB blockers, BB kingAttackersBB, int kingSquare, BB occupied);
  bool stalemate();
  bool checkmate();
  auto getMovesTree(int depth);
  u64 perft(int depth);
  StoredBoard store();
  void restore(StoredBoard board);
  void hash();
  bool makeMove(Move move);
};

template<MoveGenType moveType>
struct MoveList
{
  Move moves[MAX_MOVES], *last;
  MoveList(Board& board, bool activeSide, bool onlyEvasions = false) {
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