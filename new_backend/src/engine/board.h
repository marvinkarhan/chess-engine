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
  BB *getActivePieces(bool activeSide);
  template<PieceType pt>
  BB getPieceForSide(bool activeSide);
  Piece getPieceOnSquare(BB bb);
  void parseFenString(FenString fen);
  BB potentialAttackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB attackers(int square, bool activeSide, BB occupied, bool onlySliders = false, bool excludeSliders = false);
  BB blockers(int square, bool activeSide, BB occupied);
  auto pseudoLegalMovesGenerator(bool activeSide, bool onlyEvasions = false);
  Move *legalMovesGenerator(bool activeSide = 0);
  bool moveIsLegal(Move move, bool activeSide, u64 blockers, int kingSquare, u64 occupied);
  bool stalemate();
  bool checkmate();
  auto getMovesTree(int depth);
  u64 perft(int depth);
  StoredBoard store();
  void restore(StoredBoard board);
  void hash();
  bool makeMove(Move move);
};