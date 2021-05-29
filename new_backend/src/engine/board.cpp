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

using namespace nlohmann;
Board::Board(FenString fen /*=START_POS_FEN*/)
{
  /* TODO: Opening init */
  openingFinished = false;
  fullMoves = 0;
  openingMoves = 10;
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
    json newJson = currentOpeningTable[lastMove];
    string nextMove = getRandomMove(newJson);
    currentOpeningTable = currentOpeningTable[lastMove][nextMove];

    eval.moves.push_back(nextMove);
    cout << "OPENING TABLE" << endl;
    return eval;
  }
  return negaMax(depth, -20000, 20000);
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
  for (int i = 0; i < 7; i++)
    piecesByType[i] = BB(0);
  for (int i = 0; i < 2; i++)
    piecesBySide[i] = BB(0);
  for (int i = 0; i < 64; i++)
    piecePos[i] = NO_PIECE;
  
  castleWhiteKingSide = false;
  castleWhiteQueenSide = false;
  castleBlackKingSide = false;
  castleBlackQueenSide = false;
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
  for (auto piece : PIECE_ENUMERATED)
  {
    BB bb = pieces(piece);
    score += PieceValues[piece] * std::bitset<64>(bb).count();
    while (bb)
    {
      score += PIECE_SQUARE_TABLES[piece][63 - pop_lsb(bb)] * (getPieceSide(piece) ? 1 : -1);
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
  if (depth == 0)
  {
    bestEvaluation.evaluation = evaluate();
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
        bestEvaluation.moves = newEvaluation.moves;
        bestEvaluation.moves.push_back(move.to_uci_string());
        return bestEvaluation;
      }
      if (score > bestEvaluation.evaluation)
      {
        bestEvaluation.evaluation = score;
        bestEvaluation.moves = newEvaluation.moves;
        bestEvaluation.moves.push_back(move.to_uci_string());
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
      Piece piece = piecePos[63 - (i + row * 8)];
      if (piece)
      {
        if (counter > 0)
        {
          fen += std::to_string(counter);
          counter = 0;
        }
        fen += CharIndexToPiece[piece];
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
  for (auto piece : PIECE_ENUMERATED)
  {
    BB bb = pieces(piece);
    std::cout << std::to_string(piece) + ":" << std::endl;
    printBitboard(pieces(piece));
    std::cout << std::endl;
  }
}

BB Board::allPiecesBB()
{
  piecesByType[ALL_PIECES];
}


void Board::parseFenString(FenString fen)
{
  resetBoard();
  unsigned char character;
  BB placementSquare = 63;
  std::istringstream ss(fen);
  ss >> std::noskipws; // don't default on skiping white space
  // placement
  while ((ss >> character) && !isspace(character))
  {
    if (isdigit(character))
      placementSquare -= character - '0';
    else if (character >= 'A' && character <= 'Z' || character >= 'a' && character <= 'z')
    {
      createPiece(Piece(CharIndexToPiece.find(character)), placementSquare);
      placementSquare--;
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
  /* TODO Hash */
  // hashValue = hash();
}

BB Board::potentialSlidingAttackers(int square, bool activeSide)
{
  BB rookBB = pieces(!activeSide, ROOK), bishopBB = pieces(!activeSide, BISHOP), queenBB = pieces(!activeSide, QUEEN);
  BB potentialAttackersBB = BB(0);
  potentialAttackersBB |= BISHOP_MOVE_BBS[square] & (bishopBB | queenBB);
  potentialAttackersBB |= ROOK_MOVE_BBS[square] & (rookBB | queenBB);
  return potentialAttackersBB;
}

BB Board::attackers(int square, bool activeSide, BB occupied, bool onlySliders /*= false*/, bool excludeSliders /*= false*/)
{
  BB potentialSlidingAttackersBB = BB(0);
  if (!excludeSliders)
    potentialSlidingAttackersBB = potentialSlidingAttackers(square, activeSide);

  BB attackersBB = BB(0);
  while (potentialSlidingAttackersBB)
  {
    int moveSquare = pop_lsb(potentialSlidingAttackersBB);
    if (may_move(moveSquare, square, occupied))
      attackersBB |= SQUARE_BBS[moveSquare];
  }
  if (!onlySliders)
  {
    attackersBB |= KNIGHT_MOVE_BBS[square] & pieces(!activeSide, KNIGHT);
    attackersBB |= PAWN_ATTACKS_BBS[square][activeSide] & pieces(!activeSide, PAWN);
    attackersBB |= KING_MOVES_BBS[square] & pieces(!activeSide, KING);
  }
  return attackersBB;
}

BB Board::blockers(int square, bool activeSide, BB occupied)
{
  BB blockersBB = BB(0);

  BB potentialSlidingAttackersBB = potentialSlidingAttackers(square, activeSide);
  occupied ^= potentialSlidingAttackersBB;

  while (potentialSlidingAttackersBB)
  {
    BB potentialPinnedBB = in_between(square, pop_lsb(potentialSlidingAttackersBB)) & occupied;
    // if there ist at most one piece between king and a potential pinned piece it is pinned
    if (potentialPinnedBB && !pop_last_bb(potentialPinnedBB))
      blockersBB |= potentialPinnedBB;
  }
  return blockersBB;
}

Move *Board::generatePseudoLegalMoves(Move *moveList, bool activeSide, bool onlyEvasions /*= false*/)
{
  BB pawnBB = pieces(activeSide, PAWN), rookBB = pieces(activeSide, ROOK), knightBB = pieces(activeSide, KNIGHT), bishopBB = pieces(activeSide, BISHOP), queenBB = pieces(activeSide, QUEEN), kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB friendliesBB = piecesBySide[activeSide];
  BB notFriendliesBB = ~friendliesBB;
  BB enemiesBB = piecesBySide[!activeSide];
  BB occupiedBB = piecesByType[ALL_PIECES];
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
  BB kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB occupiedBB = piecesByType[ALL_PIECES];
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
  for (int i = 0; i < 7; i++)
    stored.piecesByType[i] = piecesByType[i];
  for (int i = 0; i < 2; i++)
    stored.piecesBySide[i] = piecesBySide[i];
  for (int i = 0; i < 64; i++)
    stored.piecePos[i] = piecePos[i];
  stored.castleWhiteKingSide = castleWhiteKingSide;
  stored.castleWhiteQueenSide = castleWhiteQueenSide;
  stored.castleBlackKingSide = castleBlackKingSide;
  stored.castleBlackQueenSide = castleBlackQueenSide;
  stored.activeSide = activeSide;
  stored.epSquareBB = epSquareBB;
  stored.fullMoves = fullMoves;
  stored.halfMoves = halfMoves;
  return stored;
}

void Board::restore(StoredBoard &board)
{
  for (int i = 0; i < 7; i++)
    piecesByType[i] = board.piecesByType[i];
  for (int i = 0; i < 2; i++)
    piecesBySide[i] = board.piecesBySide[i];
  for (int i = 0; i < 64; i++)
    piecePos[i] = board.piecePos[i];
  castleWhiteKingSide = board.castleWhiteKingSide;
  castleWhiteQueenSide = board.castleWhiteQueenSide;
  castleBlackKingSide = board.castleBlackKingSide;
  castleBlackQueenSide = board.castleBlackQueenSide;
  activeSide = board.activeSide;
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
  Piece originPiece = piecePos[newMove.originSquare];
  PieceType originPieceType = getPieceType(originPiece);
  Piece targetPiece = piecePos[newMove.targetSquare];
  if (originPiece == NO_PIECE)
    return false;
  // target piece only exists on capture
  if (targetPiece)
  {
    deletePiece(targetPiece);
    capture = true;
  }

  // update bitboards to represent change
  updatePiece(newMove.originSquare, newMove.targetSquare);

  // pawn move
  if (originPieceType == PAWN)
  {
    // promotion
    if (newMove.promotion)
    {
      // remove pawn
      deletePiece(newMove.targetSquare);
      // add promoted piece
      createPiece(newMove.promotion, newMove.targetSquare);
    }
    // en passant move
    if (epSquareBB)
    {
      if (targetSquareBB == epSquareBB)
      {
        BB capturedPawnBB = move(epSquareBB, activeSide ? DOWN : UP);
        deletePiece(bitScanForward(capturedPawnBB));
      }
      epSquareBB = 0;
      capture = true;
    }
    // check for resulting en passant
    BB movedUpx2 = originSquareBB & RANK_2 && targetSquareBB & RANK_4;
    BB movedDownx2 = originSquareBB & RANK_7 && targetSquareBB & RANK_5;
    if (movedUpx2 || movedDownx2)
    {
      Piece leftSquarePiece = piecePos[bitScanForward(move(targetSquareBB, LEFT))];
      Piece rightSquarePiece = piecePos[bitScanForward(move(targetSquareBB, RIGHT))];
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
  if (originPieceType == KING)
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
    if (newMove.type == CASTLING)
    {
      // castle king side
      if (newMove.targetSquare == newMove.originSquare - 2)
      {
        // get rook & move rook
        int rookSquare = activeSide ? 0 : 56;
        updatePiece(rookSquare, rookSquare + 2);
      }
      // castle queen side
      else
      {
        // get rook & move rook
        int rookSquare = activeSide ? 7 : 63;
        updatePiece(rookSquare, rookSquare - 3);
      }
    }
  }
  if (capture || originPieceType == PAWN)
    halfMoves = 0;
  else
    halfMoves++;
  if (!activeSide)
    fullMoves++;
  // swap sides
  activeSide = !activeSide;
  // unmake move if it was illegal
  if (attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]))
  {
    restore(storedBoard);
    return false;
  }
  return true;
}