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
#include "environment.h"
#include "time.h"
#include "evaluation.h"

#include <bitset>
#include <map>

/* TODO: remove this, when replacing old pieces system with new one */
Piece getPieceByTypeAndColor(bool activeSide, PieceType pt)
{
  return (Piece)(pt + (activeSide ? 0 : 32));
}

using namespace nlohmann;
Board::Board(FenString fen /*=START_POS_FEN*/)
{
  /* TODO: Opening init */
  openingFinished = false;
  fullMoves = 0;
  openingMoves = 5;
  parseFenString(fen);
  std::ifstream fileStream(environment::__OPENING_JSON__);
  fileStream >> currentOpeningTable;
}

Evaluation Board::evaluateNextMove(int depth, string lastMove)
{
  if (fullMoves * 2 < openingMoves && tableContainsKey(lastMove, currentOpeningTable) && !openingFinished)
  {
    Evaluation eval;
    eval.evaluation = 0;
    eval.moves = new string[1];
    json newJson = currentOpeningTable[lastMove];
    string nextMove = getRandomMove(newJson);
    currentOpeningTable = currentOpeningTable[lastMove][nextMove];
    eval.moves[0] = nextMove;
    return eval;
  }
  return negaMax(depth, INT_MIN, INT_MAX);
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
  // Seeding. Turn it off if you want the same random moves on startup.
  srand(time(0));
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
  for (auto [piece, bb] : pieces)
  {
    score += PieceValues[piece] * std::bitset<64>(bb).count();
    while (bb)
    {
      score += PIECE_SQUARE_TABLES[piece][pop_lsb(bb)] * ((int)piece < 'a' ? 1 : -1);
    }
  }
  return score * sideMultiplier;
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
  for (Move move : MoveList<LEGAL_MOVES>(*this, activeSide))
  {
    backup = store();
    if (makeMove(move))
    {
      Evaluation newEvaluation = negaMax(depth - 1, -beta, -alpha);
      score = -newEvaluation.evaluation;

      restore(backup);
      if (score >= beta)
      {
        bestEvaluation.evaluation = beta;
        addMovesToList(bestEvaluation, newEvaluation, depth, move);
        return bestEvaluation;
      }
      if (score > bestEvaluation.evaluation)
      {
        bestEvaluation.evaluation = score;
        addMovesToList(bestEvaluation, newEvaluation, depth, move);
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
  BB whitePieces[6]{pieces[getPieceByTypeAndColor(true, PAWN)], pieces[getPieceByTypeAndColor(true, KNIGHT)], pieces[getPieceByTypeAndColor(true, BISHOP)], pieces[getPieceByTypeAndColor(true, ROOK)], pieces[getPieceByTypeAndColor(true, QUEEN)], pieces[getPieceByTypeAndColor(true, KING)]};
  BB bb = BB(0);
  for (int i = 0; i < 6; i++)
  {
    bb |= whitePieces[i];
  }
  return bb;
}

BB Board::blackPiecesBB()
{
  BB blackPieces[6]{pieces[getPieceByTypeAndColor(false, PAWN)], pieces[getPieceByTypeAndColor(false, KNIGHT)], pieces[getPieceByTypeAndColor(false, BISHOP)], pieces[getPieceByTypeAndColor(false, ROOK)], pieces[getPieceByTypeAndColor(false, QUEEN)], pieces[getPieceByTypeAndColor(false, KING)]};
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

template <PieceType pt>
BB Board::getPieceForSide(bool activeSide)
{
  return pieces[getPieceByTypeAndColor(activeSide, pt)];
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
    epSquareBB = SQUARE_BBS[(7 - file) + 8 * rank];
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
  BB pawnBB = pieces[getPieceByTypeAndColor(!activeSide, PAWN)], rookBB = pieces[getPieceByTypeAndColor(!activeSide, ROOK)], knightBB = pieces[getPieceByTypeAndColor(!activeSide, KNIGHT)], bishopBB = pieces[getPieceByTypeAndColor(!activeSide, BISHOP)], queenBB = pieces[getPieceByTypeAndColor(!activeSide, QUEEN)], kingBB = pieces[getPieceByTypeAndColor(!activeSide, KING)];
  BB potentialAttackersBB = BB(0);
  if (!excludeSliders)
  {
    potentialAttackersBB |= BISHOP_MOVE_BBS[square] & (bishopBB | queenBB);
    potentialAttackersBB |= ROOK_MOVE_BBS[square] & (rookBB | queenBB);
  }
  if (!onlySliders)
  {
    potentialAttackersBB |= KNIGHT_MOVE_BBS[square] & knightBB;
    potentialAttackersBB |= PAWN_ATTACKS_BBS[square][activeSide] & pawnBB;
    potentialAttackersBB |= KING_MOVES_BBS[square] & kingBB;
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
Move *Board::generatePseudoLegalMoves(Move *moveList, bool activeSide, bool onlyEvasions /*= false*/)
{
  BB pawnBB = pieces[getPieceByTypeAndColor(activeSide, PAWN)], rookBB = pieces[getPieceByTypeAndColor(activeSide, ROOK)], knightBB = pieces[getPieceByTypeAndColor(activeSide, KNIGHT)], bishopBB = pieces[getPieceByTypeAndColor(activeSide, BISHOP)], queenBB = pieces[getPieceByTypeAndColor(activeSide, QUEEN)], kingBB = pieces[getPieceByTypeAndColor(activeSide, KING)];
  int kingSquare = bitScanForward(kingBB);
  BB friendliesBB = activeSide ? this->friendliesBB : this->enemiesBB;
  BB notFriendliesBB = ~friendliesBB;
  BB enemiesBB = activeSide ? this->enemiesBB : this->friendliesBB;
  BB occupiedBB = friendliesBB | enemiesBB;
  BB emptyBB = ~occupiedBB;
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  BB evasionBB = FULL;
  BB bb, bb2;
  int originSquare, targetSquare;
  if (onlyEvasions)
  {
    int fistKingAttackerSquare = bitScanForward(kingAttackersBB);
    evasionBB = in_between(kingSquare, fistKingAttackerSquare) | SQUARE_BBS[fistKingAttackerSquare];
  }
  // more than one king attacker = only king moves possible
  if (!pop_last_bb(kingAttackersBB))
  {
    // pawn moves
    // generate vars to handle pawn colors easier
    BB R2orR7 = activeSide ? RANK_2 : RANK_7;
    BB R3orR6 = activeSide ? RANK_3 : RANK_6;
    BB promotionRank = activeSide ? RANK_7 : RANK_2;
    Direction moveDirection = activeSide ? UP : DOWN;
    int directionFactor = activeSide ? 1 : -1;
    // normal moves
    // pawns that can move one/two
    BB pawnsNotOnPromotionRank = pawnBB & ~promotionRank;
    bb = move(pawnsNotOnPromotionRank, moveDirection) & emptyBB & evasionBB;
    bb2 = move(R3orR6 & bb, moveDirection) & emptyBB & evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = Move(targetSquare - 8 * directionFactor, targetSquare);
    }
    while (bb2)
    {
      targetSquare = pop_lsb(bb2);
      *moveList++ = Move(targetSquare - (8 << 1) * directionFactor, targetSquare);
    }
    // promotions by move and by capture
    BB pawnsOnPromotionRank = pawnBB & promotionRank;
    if (pawnsOnPromotionRank)
    {
      bb = move(pawnsOnPromotionRank, moveDirection) & emptyBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (Piece promotion : activeSide ? PROMOTION_OPTIONS_WHITE : PROMOTION_OPTIONS_BLACK)
          *moveList++ = Move(targetSquare - 8 * directionFactor, targetSquare, PROMOTION, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), LEFT) & emptyBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (Piece promotion : activeSide ? PROMOTION_OPTIONS_WHITE : PROMOTION_OPTIONS_BLACK)
          *moveList++ = Move(targetSquare - 9 * directionFactor, targetSquare, PROMOTION, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), RIGHT) & emptyBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (Piece promotion : activeSide ? PROMOTION_OPTIONS_WHITE : PROMOTION_OPTIONS_BLACK)
          *moveList++ = Move(targetSquare - 7 * directionFactor, targetSquare, PROMOTION, promotion);
      }
    }
    // pawn captures
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), LEFT) & enemiesBB & evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = Move(targetSquare - 9 * directionFactor, targetSquare);
    }
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), RIGHT) & enemiesBB & evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = Move(targetSquare - 7 * directionFactor, targetSquare);
    }
    // en passant
    if (epSquareBB && !onlyEvasions)
    {
      int epSquare = bitScanForward(epSquareBB);
      bb = pawnsNotOnPromotionRank & PAWN_ATTACKS_BBS[epSquare][!activeSide];
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = Move(targetSquare, epSquare, EN_PASSANT);
      }
    }
    // rook moves
    while (rookBB)
    {
      originSquare = pop_lsb(rookBB);
      bb = rook_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = Move(originSquare, targetSquare);
      }
    }
    // bishop moves
    while (bishopBB)
    {
      originSquare = pop_lsb(bishopBB);
      bb = bishop_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = Move(originSquare, targetSquare);
      }
    }
    // bishop moves
    while (queenBB)
    {
      originSquare = pop_lsb(queenBB);
      bb = queen_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = Move(originSquare, targetSquare);
      }
    }
    // knight moves
    while (knightBB)
    {
      originSquare = pop_lsb(knightBB);
      bb = KNIGHT_MOVE_BBS[originSquare] & notFriendliesBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = Move(originSquare, targetSquare);
      }
    }
  }
  // king moves
  bb = KING_MOVES_BBS[kingSquare] & ~friendliesBB;
  while (bb)
  {
    targetSquare = pop_lsb(bb);
    *moveList++ = Move(kingSquare, targetSquare);
  }
  // castle
  if (activeSide)
  {
    // check if sth is in the way, dont check if is legal to castle
    if (castleWhiteKingSide && !(WHITE_KING_SIDE_WAY & occupiedBB))
      *moveList++ = Move(kingSquare, WHITE_KING_SIDE_SQUARE, CASTLING);
    if (castleWhiteQueenSide && !(WHITE_QUEEN_SIDE_WAY & occupiedBB))
      *moveList++ = Move(kingSquare, WHITE_QUEEN_SIDE_SQUARE, CASTLING);
  }
  else
  {
    if (castleBlackKingSide && !(BLACK_KING_SIDE_WAY & occupiedBB))
      *moveList++ = Move(kingSquare, BLACK_KING_SIDE_SQUARE, CASTLING);
    if (castleBlackQueenSide && !(BLACK_QUEEN_SIDE_WAY & occupiedBB))
      *moveList++ = Move(kingSquare, BLACK_QUEEN_SIDE_SQUARE, CASTLING);
  }
  return moveList;
}

Move *Board::generateLegalMoves(Move *moveList, bool activeSide)
{
  BB kingBB = getPieceForSide<KING>(activeSide);
  int kingSquare = bitScanForward(kingBB);
  BB occupiedBB = friendliesBB | enemiesBB;
  BB blockersBB = blockers(kingSquare, activeSide, occupiedBB);
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  bool onlyEvasions = (bool)kingAttackersBB;
  for (const Move &move : MoveList<PSEUDO_LEGAL_MOVES>(*this, activeSide, onlyEvasions))
  {
    // only check if move is legal is it is one of:
    // case 1: piece is pinned
    // case 2: piece is king
    // case 3: move is en passant
    if ((blockersBB && blockersBB & SQUARE_BBS[move.originSquare]) || move.originSquare == kingSquare || move.type == EN_PASSANT)
    {
      if (moveIsLegal(move, activeSide, blockersBB, kingAttackersBB, kingSquare, occupiedBB))
        *moveList++ = move;
    }
    else
    {
      *moveList++ = move;
    }
  }
  return moveList;
}

bool Board::moveIsLegal(const Move &move, bool activeSide, BB blockers, BB kingAttackersBB, int kingSquare, BB occupied)
{
  BB kingNoSlideAttackersBB = attackers(kingSquare, activeSide, occupied, false, true);
  BB kingSlideAttackersBB = attackers(kingSquare, activeSide, occupied, true);
  // special case: castle
  if (move.type == CASTLING)
  {
    for (auto &castle : CASTLING_OPTIONS)
    {
      int cSquare = castle[0];
      BB cWay = castle[1];
      if (move.targetSquare == cSquare)
        while (cWay)
          // check is attacked on any square he has to move over in order to castle
          if (attackers(pop_lsb(cWay), activeSide, occupied))
            return false;
    }
  }
  // special case: en passant
  if (move.type == EN_PASSANT)
    // if piece is pinned it has to move in the ray it is pinned
    return (!blockers & SQUARE_BBS[move.originSquare]) || LINE_BBS[move.originSquare][kingSquare] & SQUARE_BBS[move.targetSquare];
  // special case: king is moving
  if (move.originSquare == kingSquare)
    // is king attacked after moving
    return !attackers(move.targetSquare, activeSide, occupied ^ SQUARE_BBS[move.originSquare]);
  // rest is eiter not a blocker or is moving along the ray of him and the king
  return (!blockers & SQUARE_BBS[move.originSquare]) || LINE_BBS[move.originSquare][kingSquare] & SQUARE_BBS[move.targetSquare];
}

bool Board::stalemate() {}
bool Board::checkmate() {}
auto Board::getMovesTree(int depth) {}
u64 Board::perft(int depth) {}

StoredBoard Board::store()
{
  StoredBoard stored;
  stored.pieces = pieces;
  stored.castleWhiteKingSide = castleWhiteKingSide;
  stored.castleWhiteQueenSide = castleWhiteQueenSide;
  stored.castleBlackKingSide = castleBlackKingSide;
  stored.castleBlackQueenSide = castleBlackQueenSide;
  stored.activeSide = activeSide;
  stored.friendliesBB = friendliesBB;
  stored.enemiesBB = enemiesBB;
  stored.epSquareBB = epSquareBB;
  stored.fullMoves = fullMoves;
  stored.halfMoves = halfMoves;
  return stored;
}

void Board::restore(StoredBoard &board)
{
  pieces = board.pieces;
  castleWhiteKingSide = board.castleWhiteKingSide;
  castleWhiteQueenSide = board.castleWhiteQueenSide;
  castleBlackKingSide = board.castleBlackKingSide;
  castleBlackQueenSide = board.castleBlackQueenSide;
  activeSide = board.activeSide;
  friendliesBB = board.friendliesBB;
  enemiesBB = board.enemiesBB;
  epSquareBB = board.epSquareBB;
  fullMoves = board.fullMoves;
  halfMoves = board.halfMoves;
}

void Board::hash() {}

bool Board::makeMove(const Move &newMove)
{
  StoredBoard storedBoard = store();
  // track if capture for half_moves
  bool capture = false;
  BB originSquareBB = SQUARE_BBS[newMove.originSquare];
  BB targetSquareBB = SQUARE_BBS[newMove.targetSquare];
  Piece originPiece = getPieceOnSquare(originSquareBB);
  Piece targetPiece = getPieceOnSquare(targetSquareBB);
  if (originPiece == NO_PIECE)
    return false;
  // update bitboards to represent change
  pieces[originPiece] &= ~originSquareBB;
  pieces[originPiece] |= targetSquareBB;
  // target piece only exists on capture
  if (targetPiece)
  {
    pieces[targetPiece] &= ~targetSquareBB;
    capture = true;
  }

  // en passant
  if (originPiece == WHITE_PAWN || originPiece == BLACK_PAWN)
  {
    // promotion
    if (newMove.promotion)
    {
      // remove pawn
      pieces[originPiece] &= ~targetSquareBB;
      // add promoted piece
      pieces[newMove.promotion] |= targetSquareBB;
    }
    // complate ep move
    if (epSquareBB)
    {
      if (targetSquareBB == epSquareBB)
      {
        BB capturedPawnBB = move(epSquareBB, activeSide ? DOWN : UP);
        Piece capturedPawn = getPieceOnSquare(capturedPawnBB);
        pieces[capturedPawn] &= ~capturedPawnBB;
        enemiesBB &= ~capturedPawnBB;
      }
      epSquareBB = 0;
      capture = true;
    }
    // check for resulting en passant
    BB movedUpx2 = originSquareBB & RANK_2 && targetSquareBB & RANK_4;
    BB movedDownx2 = originSquareBB & RANK_7 && targetSquareBB & RANK_5;
    if (movedUpx2 || movedDownx2)
    {
      Piece leftSquarePiece = getPieceOnSquare(move(targetSquareBB, LEFT));
      Piece rightSquarePiece = getPieceOnSquare(move(targetSquareBB, RIGHT));
      Piece enemyPawnKey = activeSide ? BLACK_PAWN : WHITE_PAWN;
      if (leftSquarePiece == enemyPawnKey || rightSquarePiece == enemyPawnKey)
        epSquareBB = move(targetSquareBB & RANK_4, DOWN) | move(targetSquareBB & RANK_5, UP);
    }
  }
  else
  {
    epSquareBB = 0;
  }
  // castles
  // check rook moves
  if (originPiece == WHITE_ROOK || targetPiece == WHITE_ROOK)
  {
    if (originSquareBB == (FILE_H & RANK_1) || targetSquareBB == (FILE_H & RANK_1))
      castleWhiteKingSide = false;
    else if (originSquareBB == (FILE_A & RANK_1) || targetSquareBB == (FILE_A & RANK_1))
      castleWhiteQueenSide = false;
  }
  else if (originPiece == BLACK_ROOK || targetPiece == BLACK_ROOK)
  {
    if (originSquareBB == (FILE_H & RANK_8) || targetSquareBB == (FILE_H & RANK_8))
      castleBlackKingSide = false;
    else if (originSquareBB == (FILE_A & RANK_8) || targetSquareBB == (FILE_A & RANK_8))
      castleBlackQueenSide = false;
  }
  // king moves
  if (originPiece == WHITE_KING || originPiece == BLACK_KING)
  {
    if (activeSide)
    {
      castleWhiteKingSide = false;
      castleWhiteQueenSide = false;
    }
    else
    {
      castleBlackKingSide = false;
      castleBlackQueenSide = false;
    }
    // check if king move was castle
    if (!(KING_MOVES_BBS[newMove.originSquare] & targetSquareBB))
    {
      // castle king side
      if (targetSquareBB & move(move(originSquareBB, RIGHT), RIGHT))
      {
        // get rook
        BB rookSquareBB = activeSide ? (FILE_H & RANK_1) : (FILE_H & RANK_8);
        BB targetSquareBB = move(move(rookSquareBB, LEFT), LEFT);
        Piece rookPiece = getPieceOnSquare(rookSquareBB);
        // move rook
        pieces[rookPiece] &= ~rookSquareBB;
        pieces[rookPiece] |= targetSquareBB;
        friendliesBB &= ~rookSquareBB;
        friendliesBB |= targetSquareBB;
      }
      // castle queen side
      else
      {
        // get rook
        BB rookSquareBB = activeSide ? (FILE_A & RANK_1) : (FILE_A & RANK_8);
        BB targetSquareBB = move(move(move(rookSquareBB, RIGHT), RIGHT), RIGHT);
        Piece rookPiece = getPieceOnSquare(rookSquareBB);
        // move rook
        pieces[rookPiece] &= ~rookSquareBB;
        pieces[rookPiece] |= targetSquareBB;
        friendliesBB &= ~rookSquareBB;
        friendliesBB |= targetSquareBB;
      }
    }
  }
  // update board properties
  friendliesBB &= ~originSquareBB;
  friendliesBB |= targetSquareBB;
  if (capture)
  {
    halfMoves = 0;
    enemiesBB &= ~targetSquareBB;
  }
  else
    halfMoves++;
  if (activeSide)
    fullMoves++;
  // swap sides
  activeSide = !activeSide;
  std::swap(friendliesBB, enemiesBB);
  // unmake move if it was illegal
  if (attackers(bitScanForward(pieces[activeSide ? BLACK_KING : WHITE_KING]), !activeSide, friendliesBB | enemiesBB))
  {
    restore(storedBoard);
    return false;
  }
  return true;
}