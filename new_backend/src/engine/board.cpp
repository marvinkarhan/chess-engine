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

#include <bitset>
#include <map>

using namespace nlohmann;
Board::Board(FenString fen /*=START_POS_FEN*/)
{
  /* TODO: Opening init */
  openingFinished = false;
  fullMoves = 0;
  openingMoves = 10;
  state = nullptr;
  parseFenString(fen);
  std::ifstream fileStream(environment::__OPENING_JSON__);
  fileStream >> currentOpeningTable;
}

Board::~Board()
{
  delete (state);
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
  // return negaMax(depth, -20000, 20000);
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
  score += pieceValues;
  score += pieceSquareValues;
  return score * sideMultiplier;
}

int Board::negaMax(int depth, int alpha, int beta, PVariation *pVariation)
{
  auto moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide);
  if (depth == 0)
  {
    pVariation->len = 0;
    return evaluate();
  }
  if (moveIterator.size() == 0)
  {
    if (checkmate())
    {
      return -30000 * (activeSide ? 1 : -1) * (pVariation->len+1);
    }
    return evaluate();
  }
  PVariation variation;
  int score;
  for (Move move : moveIterator)
  {
    makeMove(move);
    score = -negaMax(depth - 1, -beta, -alpha, &variation);
    unmakeMove(move);

    if (score >= beta)
      return beta;
    if (score > alpha)
    {
      alpha = score;
      pVariation->moves[0] = move;
      memcpy(pVariation->moves + 1, variation.moves, variation.len * sizeof(Move));
      pVariation->len = variation.len + 1;
    }
  }
  return alpha;
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
    std::cout << std::string(1, CharIndexToPiece[piece]) + ":" << std::endl;
    printBitboard(pieces(piece));
    std::cout << std::endl;
  }
}

BB Board::allPiecesBB()
{
  return piecesByType[ALL_PIECES];
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
  store();
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
    bb = move(pawnsNotOnPromotionRank, moveDirection) & emptyBB;
    bb2 = move(R3orR6 & bb, moveDirection) & emptyBB & evasionBB;
    bb &= evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = createMove(targetSquare - 8 * directionFactor, targetSquare);
    }
    while (bb2)
    {
      targetSquare = pop_lsb(bb2);
      *moveList++ = createMove(targetSquare - (8 << 1) * directionFactor, targetSquare);
    }
    // promotions by move and by capture
    BB pawnsOnPromotionRank = pawnBB & promotionRank;
    if (pawnsOnPromotionRank)
    {
      bb = move(pawnsOnPromotionRank, moveDirection) & emptyBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 8 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 9 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB & evasionBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 7 * directionFactor, targetSquare, promotion);
      }
    }
    // pawn captures
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB & evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = createMove(targetSquare - 9 * directionFactor, targetSquare);
    }
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB & evasionBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = createMove(targetSquare - 7 * directionFactor, targetSquare);
    }
    // en passant
    if (epSquareBB && !onlyEvasions)
    {
      int epSquare = bitScanForward(epSquareBB);
      bb = pawnsNotOnPromotionRank & PAWN_ATTACKS_BBS[epSquare][!activeSide];
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = createMove<EN_PASSANT>(targetSquare, epSquare);
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
        *moveList++ = createMove(originSquare, targetSquare);
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
        *moveList++ = createMove(originSquare, targetSquare);
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
        *moveList++ = createMove(originSquare, targetSquare);
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
        *moveList++ = createMove(originSquare, targetSquare);
      }
    }
  }
  // king moves
  bb = KING_MOVES_BBS[kingSquare] & ~friendliesBB;
  while (bb)
  {
    targetSquare = pop_lsb(bb);
    *moveList++ = createMove(kingSquare, targetSquare);
  }
  // castle
  if (activeSide)
  {
    // check if sth is in the way, dont check if is legal to castle
    if (castleWhiteKingSide && !(WHITE_KING_SIDE_WAY & occupiedBB))
      *moveList++ = createMove<CASTLING>(kingSquare, WHITE_KING_SIDE_SQUARE);
    if (castleWhiteQueenSide && !(WHITE_QUEEN_SIDE_WAY & occupiedBB))
      *moveList++ = createMove<CASTLING>(kingSquare, WHITE_QUEEN_SIDE_SQUARE);
  }
  else
  {
    if (castleBlackKingSide && !(BLACK_KING_SIDE_WAY & occupiedBB))
      *moveList++ = createMove<CASTLING>(kingSquare, BLACK_KING_SIDE_SQUARE);
    if (castleBlackQueenSide && !(BLACK_QUEEN_SIDE_WAY & occupiedBB))
      *moveList++ = createMove<CASTLING>(kingSquare, BLACK_QUEEN_SIDE_SQUARE);
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
  for (auto move : MoveList<PSEUDO_LEGAL_MOVES>(*this, activeSide, onlyEvasions))
  {
    // only check if move is legal is it is one of:
    // case 1: piece is pinned
    // case 2: piece is king
    // case 3: move is en passant
    if ((blockersBB && (blockersBB & SQUARE_BBS[originSquare(move)])) || originSquare(move) == kingSquare || moveType(move) == EN_PASSANT)
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
  if (moveType(move) == CASTLING)
  {
    for (auto &castle : CASTLING_OPTIONS)
    {
      int cSquare = castle[0];
      BB cWay = castle[1];
      if (targetSquare(move) == cSquare)
        while (cWay)
          // check is attacked on any square he has to move over in order to castle
          if (attackers(pop_lsb(cWay), activeSide, occupied))
            return false;
    }
  }
  // special case: en passant
  if (moveType(move) == EN_PASSANT)
    // if piece is pinned it has to move in the ray it is pinned
    return (~blockers & SQUARE_BBS[originSquare(move)]) || LINE_BBS[originSquare(move)][kingSquare] & SQUARE_BBS[targetSquare(move)];
  // special case: king is moving
  if (originSquare(move) == kingSquare)
    // is king attacked after moving
    return !attackers(targetSquare(move), activeSide, occupied ^ SQUARE_BBS[originSquare(move)]);
  // rest is eiter not a blocker or is moving along the ray of him and the king
  return (!blockers & SQUARE_BBS[originSquare(move)]) || LINE_BBS[originSquare(move)][kingSquare] & SQUARE_BBS[targetSquare(move)];
}

bool Board::stalemate() {}
bool Board::checkmate()
{
  BB kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB occupiedBB = piecesByType[ALL_PIECES];
  BB blockersBB = blockers(kingSquare, activeSide, occupiedBB);
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  auto moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide);
  return moveIterator.size() == 0 && kingAttackersBB > BB(0);
}
auto Board::getMovesTree(int depth) {}
u64 Board::perft(int depth)
{
  u64 nodes = 0;
  if (depth == 0)
    return 1ULL;
  for (auto move : MoveList<LEGAL_MOVES>(*this, activeSide))
  {
    makeMove(move);
    nodes += perft(depth - 1);
    unmakeMove(move);
  }
  return nodes;
}

std::string Board::divide(int depth)
{
  std::string resultsString = "";
  u64 nodes = 0;
  MoveList moveList = MoveList<LEGAL_MOVES>(*this, activeSide);
  for (auto move : moveList)
  {
    makeMove(move);
    int currNodes = perft(depth - 1);
    resultsString += toUciString(move) + " " + std::to_string(currNodes) + "\r\n";
    nodes += currNodes;
    unmakeMove(move);
  }
  resultsString += "Nodes: " + std::to_string(nodes) + "\r\n";
  resultsString += "Moves: " + std::to_string(moveList.size()) + "\r\n";
  return resultsString;
}

void Board::store(Piece captuedPiece /*= NO_PIECE*/)
{
  StoredBoard *stored = new StoredBoard();
  stored->castleWhiteKingSide = castleWhiteKingSide;
  stored->castleWhiteQueenSide = castleWhiteQueenSide;
  stored->castleBlackKingSide = castleBlackKingSide;
  stored->castleBlackQueenSide = castleBlackQueenSide;
  stored->epSquareBB = epSquareBB;
  stored->fullMoves = fullMoves;
  stored->halfMoves = halfMoves;
  stored->capturedPiece = captuedPiece;
  stored->oldBoard = std::move(state);
  state = std::move(stored);
}

void Board::restore()
{
  castleWhiteKingSide = state->castleWhiteKingSide;
  castleWhiteQueenSide = state->castleWhiteQueenSide;
  castleBlackKingSide = state->castleBlackKingSide;
  castleBlackQueenSide = state->castleBlackQueenSide;
  epSquareBB = state->epSquareBB;
  fullMoves = state->fullMoves;
  halfMoves = state->halfMoves;
  StoredBoard *oldBoard = std::move(state->oldBoard);
  delete state;
  state = std::move(oldBoard);
}

void Board::hash() {}

bool Board::makeMove(const Move &newMove)
{
  // track if capture for half_moves
  bool capture = false;
  BB originSquareBB = SQUARE_BBS[originSquare(newMove)];
  BB targetSquareBB = SQUARE_BBS[targetSquare(newMove)];
  Piece originPiece = piecePos[originSquare(newMove)];
  PieceType originPieceType = getPieceType(originPiece);
  Piece targetPiece = piecePos[targetSquare(newMove)];
  // if (originPiece == NO_PIECE)
  // {
  //   std::cout << castleWhiteKingSide << std::endl;
  //   std::cout << "move was illegal (no piece on origin square): " << toUciString(newMove) << std::endl;
  //   return false;
  // }
  // target piece only exists on capture
  if (targetPiece)
  {
    deletePiece(targetSquare(newMove));
    capture = true;
  }
  store(targetPiece); // store to save partial board information in order to be able to do unmakeMove

  // update bitboards to represent change
  updatePiece(originSquare(newMove), targetSquare(newMove));

  // pawn move
  if (originPieceType == PAWN)
  {
    // promotion
    if (moveType(newMove) == PROMOTION)
    {
      // remove pawn
      deletePiece(targetSquare(newMove));
      // add promoted piece
      createPiece(makePiece(activeSide, promotion(newMove)), targetSquare(newMove));
    }
    // en passant move
    if (moveType(newMove) == EN_PASSANT)
    {
      BB capturedPawnBB = move(epSquareBB, activeSide ? DOWN : UP);
      int capturedSquare = bitScanForward(capturedPawnBB);
      state->capturedPiece = Piece(piecePos[capturedSquare]);
      deletePiece(capturedSquare);
      capture = true;
    }
    // check for resulting en passant
    epSquareBB = getPotentialEPSquareBB(originSquare(newMove), targetSquare(newMove), *this);
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
    if (moveType(newMove) == CASTLING)
    {
      // castle king side
      if (targetSquare(newMove) == originSquare(newMove) - 2)
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
    printBitboard(attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]));
    printStateHistory();
    unmakeMove(newMove);
    return false;
  }
  return true;
}

void Board::unmakeMove(const Move &oldMove)
{
  // std::cout << "unmake move: " << toUciString(oldMove) << std::endl;
  activeSide = !activeSide;
  BB originSquareBB = SQUARE_BBS[originSquare(oldMove)];
  BB targetSquareBB = SQUARE_BBS[targetSquare(oldMove)];
  Piece originPiece = piecePos[targetSquare(oldMove)];

  if (moveType(oldMove) == PROMOTION)
  {
    // remove created piece
    deletePiece(targetSquare(oldMove));
    // recreate pawn
    createPiece(makePiece(activeSide, PAWN), originSquare(oldMove));
  }
  else
    // revert piece pos
    updatePiece(targetSquare(oldMove), originSquare(oldMove));

  if (moveType(oldMove) == CASTLING)
  {
    // castle king side
    if (targetSquare(oldMove) == originSquare(oldMove) - 2)
    {
      // get rook & move rook
      int rookSquare = activeSide ? 0 : 56;
      updatePiece(rookSquare + 2, rookSquare);
    }
    // castle queen side
    else
    {
      // get rook & move rook
      int rookSquare = activeSide ? 7 : 63;
      updatePiece(rookSquare - 3, rookSquare);
    }
  }
  else
  {
    // piece was captured (stored in StoredBoard)
    if (state->capturedPiece)
    {
      int capturedSquare = targetSquare(oldMove);
      if (moveType(oldMove) == EN_PASSANT)
        capturedSquare = targetSquare(oldMove) + (activeSide ? -8 : 8);
      // recreate captured piece
      createPiece(state->capturedPiece, capturedSquare);
    }
  }
  restore();
}