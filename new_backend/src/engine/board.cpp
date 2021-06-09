#include "board.h"
#include "moveHelper.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "../vendor/include/nlohmann/json.hpp"
#include <fstream>
#include <chrono>
#include "environment.h"
#include "time.h"
#include "stdlib.h"

#include <bitset>
#include <map>

using namespace nlohmann;
Board::Board(FenString fen /*=START_POS_FEN*/)
{
  initHashTableSize(256);
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
  delete[] hashTable;
  delete (state);
}

void Board::initHashTableSize(int sizeInMB /*=32*/)
{
  hashTableSize = (sizeInMB * 1000000) / sizeof(HashEntry);
  hashTable = new HashEntry[hashTableSize];
}

int Board::evaluateNextMove(string lastMove)
{
  if (fullMoves * 2 < openingMoves && tableContainsKey(lastMove, currentOpeningTable) && !openingFinished)
  {
    json newJson = currentOpeningTable[lastMove];
    string nextMove = getRandomMove(newJson);
    int move = uciToMove(nextMove, *this);
    currentOpeningTable = currentOpeningTable[lastMove][nextMove];
    hashTable[hashValue % hashTableSize].bestMove = move;
    cout << "OPENING TABLE" << endl;
    return 0;
  }
  int score = iterativeDeepening(5);
  return score;
}

int Board::iterativeDeepening(int timeInSeconds)
{
  endTime = time(NULL) + timeInSeconds;
  int currDepth = 1;
  int score = 0;
  stopSearch = false;
  while (time(NULL) < endTime)
  {
    score = negaMax(currDepth, MIN_ALPHA, MIN_BETA);
    if (time(NULL) < endTime)
    {
      std::cout << "info depth " << currDepth << " nodes " << nodeCount << " pv"; // <<  " score cp " << score
      std::vector<Move> pv = getPV();
      for (Move move : pv)
      {
        std::cout << " " << toUciString(move);
      }
      std::cout << std::endl;
      currDepth++;
    }
    nodeCount = 0;
  }
  stopSearch = true;
  return score;
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

void Board::prettyPrint()
{
  BB bb = piecesByType[ALL_PIECES];
  std::string lineSeperator = "+---+---+---+---+---+---+---+---+";
  for (int i = 7; i >= 0; i--)
  {
    std::cout << "   " + lineSeperator << std::endl;
    std::cout << " " << i + 1 << " | ";
    for (int j = 7; j >= 0; j--)
    {
      std::cout << CharIndexToPiece[piecePos[i * 8 + j]] << " | ";
    }
    std::cout << std::endl;
  }
  std::cout << "   " + lineSeperator << std::endl;
  std::cout << "     A   B   C   D   E   F   G   H" << std::endl;
}

int Board::quiesce(int alpha, int beta, int depth /*= 0*/)
{
  if((nodeCount & 2047) == 0) {
		stopSearch = time(NULL) > endTime;
	}
  HashEntryFlag hashFlag = UPPER_BOUND;
  Move bestMove = NONE_MOVE;
  HashEntry *entry = probeHash();
  if (entry->key == hashValue)
  {
    bestMove = entry->bestMove;
    if (entry->depth >= depth)
    {
      hashTableHits++;
      if (entry->flag == EXACT)
        return entry->score;
      if ((entry->flag == UPPER_BOUND) && (entry->score <= alpha))
        return alpha;
      if ((entry->flag == LOWER_BOUND) && (entry->score >= beta))
        return beta;
    }
  }


  int standPat = evaluate();
  int score;
  if (standPat >= beta)
    return beta;
  if (alpha < standPat)
    alpha = standPat;

  MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide, ATTACKS, bestMove);
  int moveIteratorSize = moveIterator.size();
  if (stalemate())
  {
    if (attackers(bitScanForward(pieces(activeSide, KING)), activeSide, piecesByType[ALL_PIECES]))
      return CHECKMATE_VALUE - depth;
    return 0;
  }

  for (auto move : moveIterator)
  {
    makeMove(move);
    score = -quiesce(-beta, -alpha, depth - 1);
    unmakeMove(move);

    if (stopSearch)
      return 0;

    if (score >= beta) {
      storeHash(depth, beta, move, LOWER_BOUND);
      return beta;
    }
    if (score > alpha)
    {
      bestMove = move;
      hashFlag = EXACT;
      alpha = score;
    }
  }
  storeHash(depth, alpha, bestMove, hashFlag);
  return alpha;
}

int Board::evaluate()
{
  int sideMultiplier = activeSide ? 1 : -1;
  int score = 0;
  score += pieceValues;
  score += pieceSquareValues;
  return score * sideMultiplier;
}

int Board::negaMax(int depth, int alpha, int beta)
{
  HashEntryFlag hashFlag = UPPER_BOUND;
  int score;
  Move bestMove = NONE_MOVE;
  HashEntry *entry = probeHash();
  if (entry->key == hashValue)
  {
    bestMove = entry->bestMove;
    if (entry->depth >= depth)
    {
      hashTableHits++;
      if (entry->flag == EXACT)
        return entry->score;
      if ((entry->flag == UPPER_BOUND) && (entry->score <= alpha))
        return alpha;
      if ((entry->flag == LOWER_BOUND) && (entry->score >= beta))
        return beta;
    }
  }

  if (depth == 0)
  {
    // return evaluate();
    return quiesce(alpha, beta);
  }

  if((nodeCount & 2047) == 0) {
		stopSearch = time(NULL) > endTime;
	}
  
  nodeCount++;

  MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide, ALL, bestMove);
  int moveIteratorSize = moveIterator.size();
  if (stalemate(moveIteratorSize))
  {
    if (checkmate(moveIteratorSize))
      return CHECKMATE_VALUE - depth;
    return 0;
  }
  for (Move move : moveIterator)
  {
    makeMove(move);
    score = -negaMax(depth - 1, -beta, -alpha);
    unmakeMove(move);

    if (stopSearch)
      return 0;

    if (score >= beta)
    {
      storeHash(depth, beta, move, LOWER_BOUND);
      return beta;
    }
    if (score > alpha)
    {
      hashFlag = EXACT;
      alpha = score;
      bestMove = move;
    }
  }
  storeHash(depth, alpha, bestMove, hashFlag);
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
  if (activeSide)
    hashValue ^= ZOBRIST_TABLE[ACTIVE_SIDE];
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
  zobristToggleCastle();
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
    hashValue ^= ZOBRIST_TABLE[EP_SQUARE_H + (bitScanForward(epSquareBB) & 7)];
  }
  ss >> character;
  // half moves
  ss >> character;
  halfMoves = character - '0';
  ss >> character;
  ss >> character;
  fullMoves = character - '0';
  store();
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

std::vector<Move> Board::getPV()
{
  HashEntry entry = hashTable[hashValue % hashTableSize];
  std::vector<Move> moves;
  while (entry.bestMove && entry.key == hashValue) {
    if (entry.bestMove != NONE_MOVE)
    {
      moves.push_back(entry.bestMove);
      makeMove(entry.bestMove);
    }
    entry = hashTable[hashValue % hashTableSize];
  }

  for (auto move = moves.rbegin(); move != moves.rend(); ++move)
    unmakeMove(*move);
  
  return moves;
}

ValuedMove *Board::generatePseudoLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category)
{
  BB pawnBB = pieces(activeSide, PAWN), rookBB = pieces(activeSide, ROOK), knightBB = pieces(activeSide, KNIGHT), bishopBB = pieces(activeSide, BISHOP), queenBB = pieces(activeSide, QUEEN), kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB friendliesBB = piecesBySide[activeSide];
  BB notFriendliesBB = ~friendliesBB;
  BB enemiesBB = piecesBySide[!activeSide];
  BB occupiedBB = piecesByType[ALL_PIECES];
  BB emptyBB = ~occupiedBB;
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  BB targetSquaresBB = FULL;
  BB bb, bb2;
  int originSquare, targetSquare;
  if (category == EVASIONS)
  {
    int fistKingAttackerSquare = bitScanForward(kingAttackersBB);
    targetSquaresBB = in_between(kingSquare, fistKingAttackerSquare) | SQUARE_BBS[fistKingAttackerSquare];
  }
  else if (category == ATTACKS)
  {
    targetSquaresBB = enemiesBB;
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
    if (category != ATTACKS)
    {
      bb = move(pawnsNotOnPromotionRank, moveDirection) & emptyBB;
      bb2 = move(R3orR6 & bb, moveDirection) & emptyBB & targetSquaresBB;
      bb &= targetSquaresBB;
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
    }
    // promotions by move and by capture
    BB pawnsOnPromotionRank = pawnBB & promotionRank;
    if (pawnsOnPromotionRank)
    {
      bb = move(pawnsOnPromotionRank, moveDirection) & emptyBB & targetSquaresBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 8 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB & targetSquaresBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 9 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB & targetSquaresBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 7 * directionFactor, targetSquare, promotion);
      }
    }
    // pawn captures
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB & targetSquaresBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = createMove(targetSquare - 9 * directionFactor, targetSquare);
    }
    bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB & targetSquaresBB;
    while (bb)
    {
      targetSquare = pop_lsb(bb);
      *moveList++ = createMove(targetSquare - 7 * directionFactor, targetSquare);
    }
    // en passant
    if (epSquareBB && (targetSquaresBB & move(epSquareBB, activeSide ? DOWN : UP)))
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
      bb = rook_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & targetSquaresBB;
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
      bb = bishop_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & targetSquaresBB;
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
      bb = queen_moves(SQUARE_BBS[originSquare], emptyBB, friendliesBB) & targetSquaresBB;
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
      bb = KNIGHT_MOVE_BBS[originSquare] & notFriendliesBB & targetSquaresBB;
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
  if (category != ATTACKS)
  {
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
  }
  return moveList;
}

ValuedMove *Board::generateLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category)
{
  BB kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB occupiedBB = piecesByType[ALL_PIECES];
  BB blockersBB = blockers(kingSquare, activeSide, occupiedBB);
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  bool onlyEvasions = (bool)kingAttackersBB;
  category = onlyEvasions ? EVASIONS : category;
  for (auto move : MoveList<PSEUDO_LEGAL_MOVES>(*this, activeSide, category))
  {
    // only check if move is legal is it is one of:
    // case 1: piece is pinned
    // case 2: piece is king
    // case 3: move is en passant
    if ((blockersBB && (blockersBB & SQUARE_BBS[originSquare(move)])) || originSquare(move) == kingSquare || moveType(move) == EN_PASSANT)
    {
      if (moveIsLegal(move, activeSide, blockersBB, kingSquare, occupiedBB))
        *moveList++ = move;
    }
    else
    {
      *moveList++ = move;
    }
  }
  return moveList;
}

bool Board::moveIsLegal(const Move &checkedMove, bool activeSide, BB blockersBB, int kingSquare, BB occupied)
{
  // special case: castle
  if (moveType(checkedMove) == CASTLING)
  {
    for (auto &castle : CASTLING_OPTIONS)
    {
      int cSquare = castle[0];
      BB cWay = castle[1];
      if (targetSquare(checkedMove) == cSquare)
        while (cWay)
          // check is attacked on any square he has to move over in order to castle
          if (attackers(pop_lsb(cWay), activeSide, occupied))
            return false;
    }
  }
  // special case: en passant
  if (moveType(checkedMove) == EN_PASSANT)
    // if piece is pinned it has to move in the ray it is pinned and after making the move the king cannot be attacked
    return ((~blockersBB & SQUARE_BBS[originSquare(checkedMove)]) || LINE_BBS[originSquare(checkedMove)][kingSquare] & SQUARE_BBS[targetSquare(checkedMove)]) &&
           !(blockers(kingSquare, activeSide, piecesByType[ALL_PIECES] ^ move(epSquareBB, activeSide ? DOWN : UP)) & SQUARE_BBS[originSquare(checkedMove)]);
  // special case: king is moving
  if (originSquare(checkedMove) == kingSquare)
    // is king attacked after moving
    return !attackers(targetSquare(checkedMove), activeSide, occupied ^ SQUARE_BBS[originSquare(checkedMove)]);
  // rest is eiter not a blocker or is moving along the ray of him and the king
  return (!blockersBB & SQUARE_BBS[originSquare(checkedMove)]) || LINE_BBS[originSquare(checkedMove)][kingSquare] & SQUARE_BBS[targetSquare(checkedMove)];
}

void Board::evalMoves(ValuedMove *moveListStart, ValuedMove *moveListEnd) {
  for (ValuedMove *currMove = moveListStart; currMove != moveListEnd; currMove++)
  {
    Piece originPiece = piecePos[targetSquare(*currMove)];
    Piece targetPiece = piecePos[targetSquare(*currMove)];
    // captures
    if (targetPiece)
    {
      // implement MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
      // kinda only implementing MVV
      currMove->value = mvvLva[originPiece][targetPiece];
      // always sort attacks in front
      currMove->value += 10000;
    }
  }
}

bool Board::stalemate()
{
  MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide);
  return moveIterator.size() == 0;
}

bool Board::stalemate(int moveListSize)
{
  return moveListSize == 0;
}

bool Board::checkmate()
{
  BB kingAttackersBB = attackers(bitScanForward(pieces(activeSide, KING)), activeSide, piecesByType[ALL_PIECES]);
  MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide);
  return moveIterator.size() == 0 && kingAttackersBB;
}

bool Board::checkmate(int moveListSize)
{
  BB kingAttackersBB = attackers(bitScanForward(pieces(activeSide, KING)), activeSide, piecesByType[ALL_PIECES]);
  return moveListSize == 0 && kingAttackersBB;
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
  stored->hashValue = hashValue;
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
  hashValue = state->hashValue;
  StoredBoard *oldBoard = std::move(state->oldBoard);
  delete state;
  state = std::move(oldBoard);
}

void Board::storeHash(int depth, int score, Move move, HashEntryFlag hashFlag)
{
  HashEntry *entry = &hashTable[hashValue % hashTableSize];
  if (entry->depth > depth) {
    return;
  }
  if (entry->key && entry->key != hashValue)
  {
    overwrites++;
  }
  entry->key = hashValue;
  entry->depth = depth;
  entry->flag = hashFlag;
  entry->score = score;
  entry->bestMove = move;

}

void Board::zobristToggleCastle()
{
  if (castleWhiteKingSide)
    hashValue ^= ZOBRIST_TABLE[CASTLE_WHITE_KING_SIDE];
  if (castleWhiteQueenSide)
    hashValue ^= ZOBRIST_TABLE[CASTLE_WHITE_QUEEN_SIDE];
  if (castleBlackKingSide)
    hashValue ^= ZOBRIST_TABLE[CASTLE_BLACK_KING_SIDE];
  if (castleBlackQueenSide)
    hashValue ^= ZOBRIST_TABLE[CASTLE_BLACK_QUEEN_SIDE];
}

bool Board::makeMove(const Move &newMove)
{
  if (newMove == NONE_MOVE)
  {
    std::cout << "can't null move" << std::endl;
    return false;
  }
  // track if capture for half_moves
  bool capture = false;
  BB originSquareBB = SQUARE_BBS[originSquare(newMove)];
  BB targetSquareBB = SQUARE_BBS[targetSquare(newMove)];
  Piece originPiece = piecePos[originSquare(newMove)];
  PieceType originPieceType = getPieceType(originPiece);
  Piece targetPiece = piecePos[targetSquare(newMove)];

  store(targetPiece); // store to save partial board information in order to be able to do unmakeMove

  if (originPiece == NO_PIECE)
  {
    prettyPrint();
    std::cout << "move was illegal (no piece on origin square): " << toUciString(newMove) << std::endl;
    return false;
  }
  
  // target piece only exists on capture
  if (targetPiece)
  {
    if (in_between(originSquare(newMove), targetSquare(newMove)) & piecesByType[ALL_PIECES])
      std::cout << "capture thru friendly piece" << std::endl;
    deletePiece(targetSquare(newMove));
    capture = true;
  }
  
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
    if (epSquareBB)
      hashValue ^= ZOBRIST_TABLE[EP_SQUARE_H + (bitScanForward(epSquareBB) & 7)];
    epSquareBB = getPotentialEPSquareBB(originSquare(newMove), targetSquare(newMove), *this);
    if (epSquareBB)
      hashValue ^= ZOBRIST_TABLE[EP_SQUARE_H + (bitScanForward(epSquareBB) & 7)];
  }
  else
  {
    if (epSquareBB)
      hashValue ^= ZOBRIST_TABLE[EP_SQUARE_H + (bitScanForward(epSquareBB) & 7)];
    epSquareBB = 0;
  }
  // castles
  // check rook moves
  // set it back to start
  zobristToggleCastle();
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
  // set it to what it is now
  zobristToggleCastle();
  if (capture || originPieceType == PAWN)
    halfMoves = 0;
  else
    halfMoves++;
  if (!activeSide)
    fullMoves++;
  // swap sides
  activeSide = !activeSide;
  hashValue ^= ZOBRIST_TABLE[ACTIVE_SIDE];
  // unmake move if it was illegal
  if (attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]))
  {
    std::cout << "move was illegal (king attacked): " << toUciString(newMove) << std::endl;
    printBitboard(attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]));
    prettyPrint();
    printStateHistory();
    unmakeMove(newMove);
    throw;
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