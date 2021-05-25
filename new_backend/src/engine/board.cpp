#include "constants.h"
#include "board.h"

#include "board.h"
#include "moveHelper.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "../vendor/include/nlohmann/json.hpp"
#include <fstream>
#include "enviroment.h"

using namespace nlohmann;
Board::Board(FenString fen /*=START_POS_FEN*/)
{
  /* TODO: Opening init */
  openingFinished = false;
  fullMoves = 0;
  openingMoves = 5;
  parseFenString(fen);
  std::ifstream fileStream(enviroment::__OPENING_JSON__);
  fileStream >> currentOpeningTable;
}

Evaluation Board::evaluateNextMove(int depth, string lastMove)
{
  Evaluation eval;
  eval.evaluation = 0;
  eval.moves = new string[1];
  auto currentKeys = currentOpeningTable;
  if (fullMoves * 2 < openingMoves && tableContainsKey(lastMove, currentOpeningTable) && !openingFinished)
  {
    json newJson = currentOpeningTable[lastMove];
    string nextMove = getRandomMove(newJson);
    currentOpeningTable = currentOpeningTable[lastMove][nextMove];
    eval.moves[0] = nextMove;
    return eval;
  }
}

bool Board::tableContainsKey(string moveKey, json openingTable)
{
  for (auto &[key, val] : openingTable.items())
  {
    if (moveKey == key)
      return true;
  }
  return false;
}

string Board::getRandomMove(json openingTable)
{
  int randomKeyNumber = rand() % openingTable.size();
  int i = 0;
  for (auto &[key, val] : openingTable.items())
  {
    if (i == randomKeyNumber)
      return key;
    i++;
  }
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

  castleWhiteKingSide = false;
  castleWhiteQueenSide = false;
  castleBlackKingSide = false;
  castleBlackQueenSide = false;
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
      tmpLine.insert(0, " " + std::to_string(bb & 1));
      bb >>= 1;
    }
    tmpLine.erase(0, 1);
    result.insert(0, tmpLine + "\r\n");
  }
  std::cout << result;
}

int Board::evaluate()
{
  int sideMultiplier = activeSide ? 1 : -1;
  int score = 0;
  return 1;
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
    if (makeMove(*move))
    {
      Evaluation newEvaluation = negaMax(depth - 1, -beta, -alpha);
      score = -newEvaluation.evaluation;
      restore(backup);
      if (score >= beta)
      {
        bestEvaluation.evaluation = beta;
        addMovesToList(bestEvaluation, newEvaluation, depth, *move);
        delete &newEvaluation;
        return bestEvaluation;
      }
      if (score > bestEvaluation.evaluation)
      {
        bestEvaluation.evaluation = score;
        addMovesToList(bestEvaluation, newEvaluation, depth, *move);
        delete &newEvaluation;
      }
    }
  }
  return bestEvaluation;
}

FenString Board::toFenString()
{
  FenString fen = "";
  BB bb = BB(1) << 63;
  for (int row = 0; row < 8; row++)
  {
    int counter = 0;
    for (int i = 0; i < 8; i++)
    {
      Piece piece = getPieceOnSquare(bb);
      if (piece)
      {
        if (counter > 0)
        {
          fen += std::to_string(counter);
          counter = 0;
        }
        fen += piece;
      }
      else
        counter++;
      bb >>= 1;
    }
    if (counter > 0)
      fen += std::to_string(counter);
    if (row < 7)
      fen += "/";
  }
  fen += activeSide ? " w " : " b ";
  std::string castleRights = "";
  castleRights += castleWhiteKingSide ? "K" : "";
  castleRights += castleWhiteQueenSide ? "Q" : "";
  castleRights += castleBlackKingSide ? "k" : "";
  castleRights += castleBlackQueenSide ? "q" : "";
  if (castleRights == "")
    castleRights += "-";
  fen += castleRights + " ";
  if (epSquareBB == BB(0))
    fen += "- ";
  else
    fen += SQUARE_TO_ALGEBRAIC[bitScanForward(epSquareBB)] + " ";
  fen += std::to_string(halfMoves) + " ";
  fen += std::to_string(fullMoves);
  return fen;
}

void Board::printEveryPiece()
{
  for (auto [k, v] : pieces)
  {
    std::string key(1, k);
    std::cout << key + ":" << std::endl;
    printBitboard(v);
    std::cout << std::endl;
  }
}

BB Board::whitePiecesBB()
{
  BB *whitePieces = getActivePieces(true);
  BB bb = BB(0);
  for (int i = 0; i < 6; i++)
  {
    bb |= whitePieces[i];
  }
  return bb;
}

BB Board::blackPiecesBB()
{
  BB *blackPieces = getActivePieces(false);
  BB bb = BB(0);
  for (int i = 0; i < 6; i++)
  {
    bb |= blackPieces[i];
  }
  return bb;
}

BB Board::allPiecesBB()
{
  BB bb = BB(0);
  for (auto const &[k, v] : pieces)
  {
    bb |= v;
  }
  return bb;
}

BB *Board::getActivePieces(bool activeSide)
{
  if (activeSide)
  {
    static BB whitePieces[6]{pieces[WHITE_PAWN], pieces[WHITE_ROOK], pieces[WHITE_KNIGHT], pieces[WHITE_BISHOP], pieces[WHITE_QUEEN], pieces[WHITE_KING]};
    return whitePieces;
  }
  else
  {
    static BB blackPieces[6]{pieces[BLACK_PAWN], pieces[BLACK_ROOK], pieces[BLACK_KNIGHT], pieces[BLACK_BISHOP], pieces[BLACK_QUEEN], pieces[BLACK_KING]};
    return blackPieces;
  }
}

template <PieceType pt>
BB Board::getPieceForSide(bool activeSide)
{
  return pieces[(Piece)(pt + (activeSide ? 0 : 32))];
}

Piece Board::getPieceOnSquare(BB bb)
{
  for (auto const &[k, v] : pieces)
  {
    if (bb & v)
      return k;
  }
  return NO_PIECE;
}

void Board::parseFenString(FenString fen)
{
  resetBoard();
  unsigned char character;
  BB placementMask = BB(1) << 63;
  std::istringstream ss(fen);
  ss >> std::noskipws; // don't default on skiping white space
  // placement
  while ((ss >> character) && !isspace(character))
  {
    if (isdigit(character))
      placementMask >>= character - '0';
    else if (character >= 'A' && character <= 'Z' || character >= 'a' && character <= 'z')
    {
      pieces[(Piece)character] |= placementMask;
      placementMask >>= 1;
    }
  }
  // active side
  ss >> character;
  activeSide = character == 'w';
  ss >> character;
  // castling
  while ((ss >> character) && !isspace(character))
  {
    if (character == 'K')
      castleWhiteKingSide = true;
    else if (character == 'Q')
      castleWhiteQueenSide = true;
    else if (character == 'k')
      castleBlackKingSide = true;
    else if (character == 'q')
      castleBlackQueenSide = true;
  }
  // en passant
  ss >> character;
  if (character == '-')
    epSquareBB = BB(0);
  else
  {
    unsigned char file = (character - 'a');
    ss >> character;
    unsigned char rank = (character - '1');
    epSquareBB = SQUARE_BBS[(7 - file) + 7 * rank];
  }
  ss >> character;
  // half moves
  ss >> character;
  halfMoves = character - '0';
  ss >> character;
  ss >> character;
  fullMoves = character - '0';
  // init some helper variables
  friendliesBB = activeSide ? whitePiecesBB() : blackPiecesBB();
  enemiesBB = activeSide ? blackPiecesBB() : whitePiecesBB();
  /* TODO Hash */
  // hashValue = hash();
}

BB Board::potentialAttackers(int square, bool activeSide, BB occupied, bool onlySliders /*= false*/, bool excludeSliders /*= false*/)
{
  BB *pieces = getActivePieces(!activeSide);
  BB pawn_bb = pieces[0], rook_bb = pieces[1], knight_bb = pieces[2], bishop_bb = pieces[3], queen_bb = pieces[4], king_bb = pieces[5];
  BB potentialAttackersBB = BB(0);
  if (!excludeSliders)
  {
    potentialAttackersBB |= BISHOP_MOVE_BBS[square] & (bishop_bb | queen_bb);
    potentialAttackersBB |= ROOK_MOVE_BBS[square] & (rook_bb | queen_bb);
  }
  if (!onlySliders)
  {
    potentialAttackersBB |= KNIGHT_MOVE_BBS[square] & knight_bb;
    potentialAttackersBB |= PAWN_ATTACKS_BBS[square][activeSide] & pawn_bb;
    potentialAttackersBB |= KING_MOVES_BBS[square] & king_bb;
  }
  return potentialAttackersBB;
}

BB Board::attackers(int square, bool activeSide, BB occupied, bool onlySliders /*= false*/, bool excludeSliders /*= false*/)
{
  BB potentialAttackersBB = potentialAttackers(square, activeSide, occupied, onlySliders, excludeSliders);

  BB attackersBB = BB(0);
  while (potentialAttackersBB)
  {
    int moveSquare = pop_lsb(potentialAttackersBB);
    if (may_move(moveSquare, square, occupied))
      attackersBB |= SQUARE_BBS[moveSquare];
  }
  return attackersBB;
}
BB Board::blockers(int square, bool activeSide, BB occupied)
{
  BB blockersBB = BB(0);

  BB potentialAttackersBB = potentialAttackers(square, activeSide, occupied, true);
  occupied ^= potentialAttackersBB;

  while (potentialAttackersBB)
  {
    BB potentialPinnedBB = in_between(square, pop_lsb(potentialAttackersBB)) & occupied;
    // if there ist at most one piece between king and a potential pinned piece it is pinned
    if (potentialPinnedBB && !pop_last_bb(potentialPinnedBB))
      blockersBB |= potentialPinnedBB;
  }
  return blockersBB;
}
auto Board::pseudoLegalMovesGenerator(bool activeSide, bool onlyEvasions /*= false*/) {}
Move *Board::legalMovesGenerator(bool activeSide /*= 0*/) {}

bool Board::moveIsLegal(Move move, bool activeSide, u64 blockers, int kingSquare, u64 occupied) {}
bool Board::stalemate() {}
bool Board::checkmate() {}
auto Board::getMovesTree(int depth) {}
u64 Board::perft(int depth) {}
StoredBoard Board::store() {}
void Board::restore(StoredBoard board) {}
void Board::hash() {}
bool Board::makeMove(Move move) {}