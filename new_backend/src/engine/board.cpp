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
}

FenString Board::toFenString() {
  FenString fen = "";
  BB bb = 1 << 63;
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
    if(makeMove(move)) {
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


