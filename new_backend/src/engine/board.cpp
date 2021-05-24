#include "constants.h"
#include "board.h"

int pieces[12][1];
/** current_opening_table **/
bool castleWhiteKingSide, castleWhiteQueenSide, castleBlackKingSide, castleBlackQueenSide, activeSide, openingFinished;
u64 friendlieBB, enemiesBB, epSquareBB, hashValue;
int fullMoves, halfMoves, openingMoves;

int Board::evaluate() {
  int sideMultiplier = activeSide? 1 : -1;
  int score = 0;
  return 1;
}

FenString Board::toFenString() {
  FenString fen = "";
  BB bb = 1ULL << 63;
  char piece;
  int counter = 0;
  for(int row = 0; row < 8; row++) {
    for(int column = 0; column < 8; column++) {
      piece = getPieceOnSquare(bb);
      if(piece) {
        if(counter > 0) {
          fen += counter;
          counter = 0;
        }
        fen += piece;
      } else {
        counter += 1;
      }
      bb >= 1;
    }
    if(counter > 0)
      fen += counter;
    if(row < 7)
      fen += "/";
  }
  fen +=  activeSide? " w" : " b";
  string castleRights = "";
  castleRights += castleWhiteKingSide? "K" : "";
  castleRights += castleWhiteQueenSide? "Q" : "";
  castleRights += castleBlackKingSide ? "k" : "";
  castleRights += castleBlackQueenSide? "q" : "";
  if(castleRights == "")
    castleRights += "-";
  fen += castleRights + " ";
  if(epSquareBB == 0)
    fen += "- ";
  else 
    fen += SQUARE_TO_ALGEBRAIC[epSquareBB] + " ";
  fen += halfMoves + " ";
  fen += fullMoves;
  return fen;
}

Evaluation Board::negaMax(int depth, int alpha, int beta)
{
  int score;
  StoredBoard backup;
  Evaluation bestEvaluation;
  bestEvaluation.evaluation = alpha;
  bestEvaluation.moves = new string[depth];
  if (depth == 0)
  {
    bestEvaluation.evaluation = evaluate();
    bestEvaluation.moves = {};
    return bestEvaluation;
  }
  Move *move;
  while ((move = legalMovesGenerator(activeSide)) != nullptr)
  {
    backup = store();
    if(makeMove(*move)) {
      Evaluation newEvaluation = negaMax(depth -1, -beta, -alpha);
      score = -newEvaluation.evaluation;
      restore(backup);
      if(score >= beta) {
        bestEvaluation.evaluation = beta;
        addMovesToList(bestEvaluation, newEvaluation, depth, *move);
        delete &newEvaluation;
        return bestEvaluation;
      }
      if(score > bestEvaluation.evaluation) {
        bestEvaluation.evaluation = score;
        addMovesToList(bestEvaluation, newEvaluation, depth, *move);
        delete &newEvaluation;
      }
    }
  }
  return bestEvaluation;
}


#include "board.h"
#include <string>
#include <iostream>

Board::Board(FenString fen /*=START_POS_FEN*/)
{
  resetBoard();

  /* TODO: Opening init */
  openingFinished = false;

  // parseFenString(fen);
}

void Board::resetBoard()
{
  pieces = {
      {WHITE_PAWN, 0},
      {WHITE_ROOK, 0},
      {WHITE_KNIGHT, 0},
      {WHITE_BISHOP, 0},
      {WHITE_QUEEN, 0},
      {WHITE_KING, 0},
      {BLACK_PAWN, 0},
      {BLACK_ROOK, 0},
      {BLACK_KNIGHT, 0},
      {BLACK_BISHOP, 0},
      {BLACK_QUEEN, 0},
      {BLACK_QUEEN, 0}};

  castleWhiteKingSide = true;
  castleWhiteQueenSide = true;
  castleBlackKingSide = true;
  castleBlackQueenSide = true;
  friendliesBB = BB(0);
  enemiesBB = BB(0);
  hashValue = 0ULL;

  // 0: black, 1: white
  activeSide = true;
  fullMoves = 0;
  halfMoves = 0;

  epSquareBB = BB(0);
}

void Board::printBitboard(BB bb)
{
  std::string result = "";
  for (int i = 0; i < 8; i++)
  {
    std::string tmpLine = "";
    for (int j = 0; j < 8; j++)
    {
      tmpLine.insert(0, " " + std::to_string(bb & 1ULL));
      bb >>= 1;
    }
    tmpLine.erase(0, 1);
    result.insert(0, tmpLine + "\r\n");
  }
  std::cout << result;
}
