#include "constants.h"
#include "board.h"

#include "board.h"
#include "moveHelper.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

Board::Board(FenString fen /*=START_POS_FEN*/)
{
  /* TODO: Opening init */
  openingFinished = false;
  parseFenString(fen);
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
  for (Move move : MoveList<PSEUDO_LEGAL_MOVES>(*this, activeSide))
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
        delete &newEvaluation;
        return bestEvaluation;
      }
      if (score > bestEvaluation.evaluation)
      {
        bestEvaluation.evaluation = score;
        addMovesToList(bestEvaluation, newEvaluation, depth, move);
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

/* TODO: fix this bad code */
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
  BB pawnBB = pieces[0], rookBB = pieces[1], knightBB = pieces[2], bishopBB = pieces[3], queenBB = pieces[4], kingBB = pieces[5];
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
  BB *pieces = getActivePieces(activeSide);
  BB pawnBB = pieces[0], rookBB = pieces[1], knightBB = pieces[2], bishopBB = pieces[3], queenBB = pieces[4], kingBB = pieces[5];
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
      printBitboard(bb);
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
    // while (rookBB)
    // {
    //   originSquare = pop_lsb(rookBB);
    //   bb = rook_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & evasionBB;
    //   while (bb)
    //   {
    //     targetSquare = pop_lsb(bb);
    //     *moveList++ = Move(originSquare, targetSquare);
    //   }
    // }
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
    if (castleWhiteKingSide && WHITE_KING_SIDE_WAY & emptyBB) {
      std::cout << "new Move: " + Move(kingSquare, WHITE_KING_SIDE_SQUARE, CASTLING).to_uci_string() << std::endl;
      *moveList++ = Move(kingSquare, WHITE_KING_SIDE_SQUARE, CASTLING);
    }
    if (castleWhiteQueenSide && WHITE_QUEEN_SIDE_WAY & emptyBB)
      *moveList++ = Move(kingSquare, WHITE_QUEEN_SIDE_SQUARE, CASTLING);
  }
  else
  {
    if (castleBlackKingSide && BLACK_KING_SIDE_WAY & emptyBB)
      *moveList++ = Move(kingSquare, BLACK_KING_SIDE_SQUARE, CASTLING);
    if (castleBlackQueenSide && BLACK_QUEEN_SIDE_WAY & emptyBB)
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
StoredBoard Board::store() {}
void Board::restore(StoredBoard board) {}
void Board::hash() {}
bool Board::makeMove(Move move) {}