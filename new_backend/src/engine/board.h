#pragma once
#include "constants.h"
#include <string>
#include <map>
#include "move.h"
#include "../vendor/include/nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

struct Evaluation
{
  int evaluation;
  vector<string> moves;
};

struct StoredBoard
{
  std::map<Piece, BB> pieces;
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide;
  BB friendliesBB, enemiesBB, epSquareBB;
  int fullMoves, halfMoves;
};


class Board
{

private:
  bool tableContainsKey(string moveKey, json openingTable);
  string getRandomMove(json openingTable);
public:
  std::map<Piece, BB> pieces;
  /** current_opening_table **/
  bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
  u64 friendliesBB, enemiesBB, epSquareBB, hashValue;
  int fullMoves, halfMoves, openingMoves;
  nlohmann::json currentOpeningTable;

  Evaluation negaMax(int depth, int alpha, int beta);
  int evaluate();
  Evaluation evaluateNextMove(int depth, string lastMove);
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
  void restore(StoredBoard &board);
  void hash();
  bool makeMove(const Move &move);
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