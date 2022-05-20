#include "time.h"
#include "stdlib.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include <bitset>
#include <map>
#include <algorithm>
#include <string.h>
#include <cmath>

#include "board.h"
#include "movehelper.h"
#include "environment.h"
#include "movepicker.h"

Board::Board(FenString fen /*=START_POS_FEN*/)
{
  initHashTableSize(DEFAULT_HASH_TABLE_SIZE);
  fullMoves = 0;
  state = nullptr;
  stopSearch = false;
  parseFenString(fen);
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

int Board::evaluateNextMove(float movetime /*= 0*/, float wtime /*= 0*/, float btime /*= 0*/, int maxDepth /*= MAX_DEPTH*/)
{
  if (!movetime)
  {
    float owntime = activeSide ? wtime : btime;
    if (owntime)
      movetime = owntime / MOVES_TO_GO;
    else
      movetime = thinkingTime;
  }
  if (movetime > TIME_BUFFER)
    movetime -= TIME_BUFFER;
  int score = iterativeDeepening(movetime, maxDepth);
  return score;
}

int Board::iterativeDeepening(float timeInSeconds /*= std::numeric_limits<float>::max()*/, int maxDepth /*= MAX_DEPTH*/)
{
  if (timeInSeconds)
  {
    endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + ((long long)(timeInSeconds * 1000));
  }
  int score, currDepth = 1;
  stopSearch = false;
  // init val for aspiration windows
  int alpha = latestScore - ASPIRATION_WINDOW_VALUE;
  int beta = latestScore + ASPIRATION_WINDOW_VALUE;
  std::vector<Move> pv;
  while (!stopSearch && currDepth <= maxDepth)
  {
    nodeCount = 0;
    score = negaMax(currDepth, alpha, beta);
    // only print info if search wasn't stopped during search
    if (!stopSearch)
    {
      pv = getPV();
      // if checkmate was found quit
      if (score <= CHECKMATE_VALUE || score >= -CHECKMATE_VALUE)
      {
        // position is already mate
        if (pv[0] == NONE_MOVE)
        {
          std::cout << "info depth 0 score mate 0" << std::endl;
          std::cout << "bestmove (none)" << std::endl;
        }
        stopSearch = true;
      }
      // possible aspiration window widening
      if (score <= alpha)
      {
        alpha -= ASPIRATION_WINDOW_VALUE;
        continue;
      }
      if (score >= beta)
      {
        beta += ASPIRATION_WINDOW_VALUE;
        continue;
      }
      // setup aspiration window
      alpha = score - ASPIRATION_WINDOW_VALUE;
      beta = score + ASPIRATION_WINDOW_VALUE;
      latestScore = score;
      std::cout << "info depth " << currDepth << " nodes " << nodeCount;
      printScore(latestScore, pv);
      std::cout << " pv";
      for (Move move : pv)
      {
        std::cout << " " << toUciString(move);
      }
      std::cout << std::endl;
      currDepth++;
    }
  }
  stopSearch = true;
  if (pv.size() > 0 && pv.at(0) != NONE_MOVE)
    // signal we made out final decision if there are moves left to make
    std::cout << "bestmove " << toUciString(pv[0]) << std::endl;
  else
    std::cout << "bestmove (none)" << std::endl;

  return latestScore;
}

void Board::printScore(int score, std::vector<Move> &pv) {
  std::cout << " score";
  // check if its mate
  if (score < CHECKMATE_VALUE || score > -CHECKMATE_VALUE) {
    std::cout << " mate " << std::ceil((( pv.size() - 1) / 2));
  } else {
    std::cout << " cp " << score;
  }
}

void Board::resetBoard()
{
  stopSearch = false;
  memset(piecesByType, 0, sizeof piecesByType);
  memset(piecesBySide, 0, sizeof piecesBySide);
  memset(piecePos, 0, sizeof piecePos);

  latestPv.clear();
  castleWhiteKingSide = false;
  castleWhiteQueenSide = false;
  castleBlackKingSide = false;
  castleBlackQueenSide = false;
  activeSide = true;
  stopSearch = false;
  fullMoves = 0;
  latestScore = 0;
  halfMoves = 0;
  fullMoves = 0;
  nodeCount = 0;
  hashTableHits = 0;
  epSquareBB = 0;
  pieceValues = 0;
  pieceSquareValues = 0;

  // keep following data to increase speed
  // delete[] hashTable;
  // initHashTableSize(hashTableSize * sizeof(HashEntry) / 1000000);
  // memset(pvTable, 0, sizeof pvTable);
  // memset(pvLength, 0, sizeof pvLength);
  // memset(killerMoves, 0, sizeof killerMoves);
  // delete (state);
  // state = nullptr;
}

void Board::printBitboard(BB bb)
{
  std::string result = "";
  std::string lineSeperator = "+---+---+---+---+---+---+---+---+";
  for (int i = 0; i < 8; i++)
  {
    std::string tmpLine = " ";
    for (int j = 0; j < 8; j++)
    {
      tmpLine.insert(0, "" + std::to_string(bb & 1) + " | ");
      bb >>= 1;
    }

    // tmpLine.erase(0, 1);
    result.insert(0, " " + to_string(i + 1) + " | " + tmpLine + "\r\n   " + lineSeperator + "\r\n");
  }
  result.insert(0, "   " + lineSeperator + "\r\n");
  std::cout << result;
  std::cout << "     A   B   C   D   E   F   G   H" << std::endl;
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
  // track time control in interval
  if ((nodeCount & 2047) == 0)
  {
    stopSearch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() > endTime;
  }

  nodeCount++;

  int score;
  // HashEntryFlag hashFlag = UPPER_BOUND;
  Move bestMove = NONE_MOVE;
  // HashEntry *entry = probeHash();
  // if (entry->key == hashValue)
  // {
  //   bestMove = entry->bestMove;
  //   if (entry->depth >= depth)
  //   {
  //     hashTableHits++;
  //     if (entry->flag == EXACT)
  //       score = entry->score;
  //     else if ((entry->flag == UPPER_BOUND) && (entry->score <= alpha))
  //       score = alpha;
  //     else if ((entry->flag == LOWER_BOUND) && (entry->score >= beta))
  //       score = beta;
  //   }
  // }

  if (partialStalemate())
    return STALEMATE_VALUE;

  int standPat = evaluate();
  if (standPat >= beta)
    return beta;
  if (alpha < standPat)
    alpha = standPat;

  MovePicker movePicker(*this, bestMove, true);
  Move move;
  while ((move = movePicker.nextMove()) != NONE_MOVE)
  {
    ply++;
    makeMove(move);
    score = -quiesce(-beta, -alpha, depth - 1);
    unmakeMove(move);
    ply--;
    if (stopSearch)
      return 0;

    if (score >= beta)
    {
      // storeHash(depth, beta, move, LOWER_BOUND);
      return beta;
    }
    if (score > alpha)
    {
      // bestMove = move;
      // hashFlag = EXACT;
      alpha = score;
    }
  }
  // storeHash(depth, alpha, bestMove, hashFlag);
  return alpha;
}

int Board::evaluate()
{
  int sideMultiplier = activeSide ? 1 : -1;
  int score = 0;
  score += pieceValues;
  score += pieceSquareValues;
  // Bishop pair bonus
  if (pop_last_bb(pieces(1, BISHOP)))
    score += BISHOP_PAIR;
  if (pop_last_bb(pieces(0, BISHOP)))
    score -= BISHOP_PAIR;
  // Knight pair penalty
  if (pop_last_bb(pieces(1, KNIGHT)))
    score += KNIGHT_PAIR;
  if (pop_last_bb(pieces(0, KNIGHT)))
    score -= KNIGHT_PAIR;
  // Rook pair penalty
  if (pop_last_bb(pieces(1, ROOK)))
    score += ROOK_PAIR;
  if (pop_last_bb(pieces(0, ROOK)))
    score -= ROOK_PAIR;
  // No Pawn penalty
  if (!(pieces(1, PAWN)))
    score += NO_PAWNS;
  if (!(pieces(0, PAWN)))
    score -= NO_PAWNS;
  score += evaluateMobility();

  return score * sideMultiplier;
}

int Board::evaluateMobility()
{
  int score = 0;
  score += popCount(pieceMoves(PAWN, 1)) * 7;
  score += popCount(pieceMoves(PAWN, 0)) * -7;
  score += popCount(pieceMoves(BISHOP, 1)) * 6;
  score += popCount(pieceMoves(BISHOP, 0)) * -6;
  score += popCount(pieceMoves(KNIGHT, 1)) * 5;
  score += popCount(pieceMoves(KNIGHT, 0)) * -5;
  score += popCount(pieceMoves(ROOK, 1)) * 4;
  score += popCount(pieceMoves(ROOK, 0)) * -4;
  score += popCount(pieceMoves(QUEEN, 1)) * 3;
  score += popCount(pieceMoves(QUEEN, 0)) * -3;
  // score += popCount(pieceMoves(KING,1)) * 4;
  // score += popCount(pieceMoves(KING,0)) * -4;
  return score;
}

int Board::negaMax(int depth, int alpha, int beta, bool nullMoveAllowed /*=true*/)
{
  // track time control in interval
  if ((nodeCount & 2047) == 0)
    stopSearch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() > endTime;

  // check for potential stalemate by repetition
  // try to make it a directed acyclic graph (no cycles)
  // if stalemate improves the result
  if (halfMoves >= 3 && alpha < STALEMATE_VALUE && hasRepetitions())
  {
    if (STALEMATE_VALUE >= beta)
      return STALEMATE_VALUE;
  }

  pvLength[ply] = ply;
  HashEntryFlag hashFlag = UPPER_BOUND;
  int score;
  Move bestMove = NONE_MOVE;
  HashEntry *entry = probeHash();
  if (entry->key == state->hashValue)
  {
    bestMove = entry->bestMove;
    if (entry->depth >= depth)
    {
      hashTableHits++;
      if (entry->flag == EXACT)
        score = entry->score; // may be risky
      else if ((entry->flag == UPPER_BOUND) && (entry->score <= alpha))
        score = alpha; // may be risky
      else if ((entry->flag == LOWER_BOUND) && (entry->score >= beta))
        score = beta;
    }
  }

  if (depth == 0)
  {
    // return evaluate();
    return quiesce(alpha, beta);
  }
  bool kingChecked = isKingAttacked();

  // check extension
  if (kingChecked)
    depth++;
  // prune only if not in check
  // (don't know if causes bugs)
  // if (!kingChecked)
  // {
  //   int staticEvaluation = evaluate(); // for pruning purposes
  //   // null move pruning
  //   // prove that any move is better than no move
  //   if (nullMoveAllowed)
  //   {
  //     if (ply > 0 && depth > 2 && staticEvaluation >= beta)
  //     {
  //       makeNullMove();
  //       int nullScore = -negaMax(depth - 3, -beta, 1 - beta, false);

  //       if (stopSearch)
  //         return 0;

  //       if (nullScore >= beta)
  //       {
  //         unmakeNullMove();
  //         return beta;
  //       }
  //       else // mate thread extension
  //       {
  //         if (negaMax(depth - 3, CHECKMATE_VALUE / 2 - 1, CHECKMATE_VALUE / 2, false) > CHECKMATE_VALUE / 2)
  //           depth++;
  //         unmakeNullMove();
  //       }

  //       if (nullScore >= beta)
  //       {
  //         // verification search
  //         int s = negaMax(depth - 3, beta - 1, beta, false);
  //         // if still ok
  //         if (s >= beta)
  //           return beta;
  //       }
  //     }
  //   }
  // }

  nodeCount++;
  int moveCounter = 0;
  MovePicker movePicker(*this, bestMove);
  Move move;
  while ((move = movePicker.nextMove()) != NONE_MOVE)
  {
    ply++;
    makeMove(move);
    score = -negaMax(depth - 1, -beta, -alpha);
    unmakeMove(move);
    ply--;
    if (stopSearch)
      return 0;

    moveCounter++;

    if (score >= beta)
    {
      // add killer moves if quiet move
      if (!piecePos[targetSquare(move)])
      {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = move;
      }
      storeHash(depth, beta, move, LOWER_BOUND);
      return beta;
    }
    if (score > alpha)
    {
      hashFlag = EXACT;
      alpha = score;
      bestMove = move;
      // add quiet moves to history heuristics
      // if (!piecePos[targetSquare(move)])
      //   historyHeuristicTable[piecePos[originSquare(move)]][targetSquare(move)] += depth * depth;
      // add move to pv table
      pvTable[ply][ply] = move;
      for (int next_ply = ply + 1; next_ply < pvLength[ply + 1]; next_ply++)
        pvTable[ply][next_ply] = pvTable[ply + 1][next_ply];
      pvLength[ply] = pvLength[ply + 1];
    }
  }
  if (moveCounter == 0) // stalemate
  {
    if (isKingAttacked()) // checkmate
      return CHECKMATE_VALUE - depth;
    return 0;
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
  u64 hashValue = 0ULL;
  BB placementSquare = 63;
  std::istringstream ss(fen);
  ss >> std::noskipws; // don't default on skipping white space
  // placement
  while ((ss >> character) && !isspace(character))
  {
    if (isdigit(character))
      placementSquare -= character - '0';
    else if (character >= 'A' && character <= 'Z' || character >= 'a' && character <= 'z')
    {
      createPiece(Piece(CharIndexToPiece.find(character)), placementSquare);
      hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[Piece(CharIndexToPiece.find(character))] + originSquare(placementSquare)];
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
  zobristToggleCastle(hashValue);
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
  state->hashValue = hashValue;
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
  // make sure attacker is on an occupied square
  return attackersBB & occupied;
}

BB Board::pieceMoves(PieceType type, bool activeSide)
{
  BB attackFields = BB(0);
  BB friendlies = piecesBySide[activeSide];
  switch (type)
  {
  case PAWN:
  {
    BB pawns = pieces(activeSide, PAWN);
    attackFields = pawn_attacks(pawns, activeSide, friendlies) & piecesBySide[!activeSide];
    if (activeSide)
    {
      BB oneUP = move(pawns, UP) & ~piecesByType[ALL_PIECES];
      attackFields |= oneUP;
      BB doublePawnPush = oneUP & RANK_3;
      attackFields |= move(doublePawnPush, UP) & ~piecesByType[ALL_PIECES];
    }
    else
    {
      BB oneDown = move(pawns, DOWN) & ~piecesByType[ALL_PIECES];
      attackFields |= oneDown;
      BB doublePawnPush = oneDown & RANK_6;
      attackFields |= move(doublePawnPush, DOWN) & ~piecesByType[ALL_PIECES];
    }
    return attackFields;
  }
  case BISHOP:
  {
    BB bishops = pieces(activeSide, BISHOP);
    return bishop_moves(bishops, ~piecesByType[ALL_PIECES], friendlies);
  }
  case KNIGHT:
  {
    BB knights = pieces(activeSide, KNIGHT);
    int KnightSquare = 0;
    while (knights)
    {
      KnightSquare = pop_lsb(knights);
      attackFields |= KNIGHT_MOVE_BBS[KnightSquare] & ~friendlies;
    }
    return attackFields;
  }
  case ROOK:
  {
    BB rooks = pieces(activeSide, ROOK);
    return rook_moves(rooks, ~piecesByType[ALL_PIECES], friendlies);
  }
  case QUEEN:
  {
    BB queens = pieces(activeSide, QUEEN);
    return queen_moves(queens, ~piecesByType[ALL_PIECES], friendlies);
  }
  case KING:
  {
    BB king = pieces(activeSide, KING);
    int kingSquare = 0;
    kingSquare = pop_lsb(king);
    BB kingMoves = KING_MOVES_BBS[kingSquare] & ~friendlies;
    int kingMoveSquare = 0;
    BB realKingMoves = BB(0);
    while (kingMoves)
    {
      kingMoveSquare = pop_lsb(kingMoves);
      BB kingAttackers = attackers(kingMoveSquare, activeSide, piecesByType[ALL_PIECES]);
      if (!kingAttackers)
        realKingMoves |= (1ULL << kingMoveSquare);
    }
    attackFields |= realKingMoves;
    break;
  }
  default: // ALL
    return BB(0);
  }
  return BB(0);
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
  std::vector<Move> moves;
  if (stopSearch || (pvLength[0] == 0) && !latestPv.empty())
  {
    return latestPv;
  }
  else if (pvLength[0] == 0) // fallback
  {
    moves.push_back(pvTable[0][0]);
    return moves;
  }
  // get PV from quadratic pv table
  for (int count = 0; count < pvLength[0] && pvTable[0][count]; count++)
  {
    moves.push_back(pvTable[0][count]);
  }
  latestPv = moves;
  return moves;
}

ValuedMove *Board::generatePseudoLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category)
{
  BB pawnBB = pieces(activeSide, PAWN), rookBB = pieces(activeSide, ROOK), knightBB = pieces(activeSide, KNIGHT), bishopBB = pieces(activeSide, BISHOP), queenBB = pieces(activeSide, QUEEN), kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB friendliesBB = piecesBySide[activeSide];
  BB enemiesBB = piecesBySide[!activeSide];
  BB occupiedBB = piecesByType[ALL_PIECES];
  BB emptyBB = ~occupiedBB;
  BB kingAttackersBB = attackers(kingSquare, activeSide, occupiedBB);
  BB targetSquaresBB = ~friendliesBB;
  BB bb, bb2;
  int originSquare, targetSquare;
  if (category == EVASIONS)
  {
    int fistKingAttackerSquare = bitScanForward(kingAttackersBB);
    targetSquaresBB = in_between(kingSquare, fistKingAttackerSquare) | SQUARE_BBS[fistKingAttackerSquare];
    enemiesBB = kingAttackersBB;
  }
  else if (category == ATTACKS)
  {
    targetSquaresBB = enemiesBB;
  }
  else if (category == QUIETS)
  {
    targetSquaresBB = ~occupiedBB;
    enemiesBB = BB(0);
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
      bb2 = move(R3orR6 & bb, moveDirection) & emptyBB;
      if (category == EVASIONS)
      {
        bb &= targetSquaresBB;
        bb2 &= targetSquaresBB;
      }
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
      bb = move(pawnsOnPromotionRank, moveDirection) & emptyBB;
      if (category == EVASIONS)
        bb &= targetSquaresBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 8 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 9 * directionFactor, targetSquare, promotion);
      }
      bb = move(move(pawnsOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        for (PieceType promotion : PROMOTION_OPTIONS)
          *moveList++ = createMove<PROMOTION>(targetSquare - 7 * directionFactor, targetSquare, promotion);
      }
    }
    if (category != QUIETS)
    {
      // pawn captures
      bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? LEFT : RIGHT) & enemiesBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = createMove(targetSquare - 9 * directionFactor, targetSquare);
      }
      bb = move(move(pawnsNotOnPromotionRank, moveDirection), activeSide ? RIGHT : LEFT) & enemiesBB;
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
      bb = KNIGHT_MOVE_BBS[originSquare] & targetSquaresBB;
      while (bb)
      {
        targetSquare = pop_lsb(bb);
        *moveList++ = createMove(originSquare, targetSquare);
      }
    }
  }
  // king moves
  bb = KING_MOVES_BBS[kingSquare] & ~friendliesBB;
  if (category != EVASIONS)
    bb &= targetSquaresBB;
  while (bb)
  {
    targetSquare = pop_lsb(bb);
    *moveList++ = createMove(kingSquare, targetSquare);
  }
  if (category != ATTACKS && category != EVASIONS)
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
  int kingSquare = bitScanForward(pieces(activeSide, KING));
  BB blockersBB = blockers(kingSquare, activeSide, piecesByType[ALL_PIECES]);
  if (category != EVASIONS && kingAttackers())
    category = EVASIONS;
  for (auto move : MoveList<PSEUDO_LEGAL_MOVES>(*this, activeSide, category))
  {
    // only check if move is legal is it is one of:
    // case 1: piece is pinned
    // case 2: piece is king
    // case 3: move is en passant
    if ((blockersBB && (blockersBB & SQUARE_BBS[originSquare(move)])) || originSquare(move) == kingSquare || moveType(move) == EN_PASSANT)
    {
      if (moveIsLegal(move, activeSide, blockersBB, kingSquare))
        *moveList++ = move;
    }
    else
    {
      *moveList++ = move;
    }
  }
  return moveList;
}

bool Board::moveIsLegal(const Move checkedMove, bool activeSide, BB blockersBB, int kingSquare)
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
          if (attackers(pop_lsb(cWay), activeSide, piecesByType[ALL_PIECES]))
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
    return !(attackers(targetSquare(checkedMove), activeSide, piecesByType[ALL_PIECES] ^ SQUARE_BBS[originSquare(checkedMove)]));
  // rest is either not a blocker or is moving along the ray of him and the king
  return (!(blockersBB & SQUARE_BBS[originSquare(checkedMove)])) || LINE_BBS[originSquare(checkedMove)][kingSquare] & SQUARE_BBS[targetSquare(checkedMove)];
}

// used for hash move validation to avoid making moves generated by hash collision of different positions but same hash keys
bool Board::moveIsPseudoLegal(const Move checkedMove)
{
  int originSq = originSquare(checkedMove);
  int targetSq = targetSquare(checkedMove);
  Piece originPiece = piecePos[originSq];
  PieceType originPieceType = getPieceType(originPiece);

  // do expensive check for none normal moves
  if (true || moveType(checkedMove) != NORMAL || originPieceType == PAWN)
  {
    MoveList moveList = MoveList<LEGAL_MOVES>(*this, activeSide, isKingAttacked() ? EVASIONS : ALL);
    // is move contained in pseudo legal moves
    return std::find(moveList.begin(), moveList.end(), checkedMove) != moveList.end();
  }
  // is the promotion is anything other than 0 it is a promotion move (can't because it has to be a normal one)
  if (promotion(checkedMove) != ALL_PIECES)
    return false;
  // origin piece is no piece
  if (originPiece == NO_PIECE)
    return false;
  // piece os not belong to active side
  if (getPieceSide(originPiece) != activeSide)
    return false;
  // piece target square is already occupied by a friendly
  if (pieces(activeSide) & SQUARE_BBS[targetSq])
    return false;
  // check for slider move
  if (
      (originPieceType == BISHOP || originPieceType == ROOK || originPieceType == QUEEN) &&
      (!may_move(originSq, targetSq, piecesByType[ALL_PIECES])))
    return false;
  else if (originPieceType == KING && attackers(targetSquare(checkedMove), activeSide, piecesByType[ALL_PIECES] ^ SQUARE_BBS[originSquare(checkedMove)]))
    return false; // is king attacked after moving
  // king is under attack
  BB kingAttackersBB = kingAttackers();
  if (kingAttackersBB)
  {
    if (originPieceType != KING)
    {
      // double check
      if (pop_last_bb(kingAttackersBB))
        return false;

      // remove the attacker or piece has to block the attacker
      if (!((kingAttackersBB & SQUARE_BBS[targetSq]) ||
            (in_between(bitScanForward(pieces(activeSide, KING)), bitScanForward(kingAttackersBB)) & SQUARE_BBS[targetSq])))
        return false;
    }
  }
  return true;
}

bool Board::hasRepetitions()
{
  StoredBoard *stateP = state;
  int bound = halfMoves;
  while (bound-- >= 4 && stateP)
  {
    if (stateP->repetition)
      return true;
    stateP = stateP->oldBoard;
  }
  return false;
}

// checks for 50-Move Rule and repetition draws
bool Board::partialStalemate()
{
  if (halfMoves >= 100 && (!kingAttackers() || MoveList<LEGAL_MOVES>(*this, activeSide).size()))
    return true;

  return state->repetition;
}

bool Board::stalemate()
{
  MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide);
  return moveIterator.size() == 0;
}

bool Board::checkmate()
{
  BB kingAttackersBB = kingAttackers();
  if (kingAttackersBB)
  {
    MoveList moveIterator = MoveList<LEGAL_MOVES>(*this, activeSide, EVASIONS);
    return moveIterator.size() == 0;
  }
  return false;
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

void Board::store(Piece capturedPiece /*= NO_PIECE*/)
{
  StoredBoard *stored = new StoredBoard();
  stored->castleWhiteKingSide = castleWhiteKingSide;
  stored->castleWhiteQueenSide = castleWhiteQueenSide;
  stored->castleBlackKingSide = castleBlackKingSide;
  stored->castleBlackQueenSide = castleBlackQueenSide;
  stored->epSquareBB = epSquareBB;
  stored->fullMoves = fullMoves;
  stored->halfMoves = halfMoves;
  stored->capturedPiece = capturedPiece;
  stored->repetition = NO_REPETITION;
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

void Board::storeHash(int depth, int score, Move move, HashEntryFlag hashFlag)
{
  HashEntry *entry = &hashTable[state->hashValue % hashTableSize];
  if (entry->depth > depth)
  {
    return;
  }
  if (entry->key && entry->key != state->hashValue)
  {
    overwrites++;
  }
  entry->key = state->hashValue;
  entry->depth = depth;
  entry->flag = hashFlag;
  entry->score = score;
  entry->bestMove = move;
}

void Board::zobristToggleCastle(u64 &hashValue)
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

  u64 hashValue = state->hashValue;

  store(targetPiece); // store to save partial board information in order to be able to do unmakeMove
  state->move = newMove;
  // DEBUG
  if (originPiece == NO_PIECE)
  {
    prettyPrint();
    std::cout << "move was illegal (no piece on origin square): " << toUciString(newMove) << std::endl;
    return false;
  }

  // target piece only exists on capture
  if (targetPiece)
  {
    // DEBUG
    if (in_between(originSquare(newMove), targetSquare(newMove)) & piecesByType[ALL_PIECES])
      std::cout << "capture thru friendly piece" << std::endl;
    deletePiece(targetSquare(newMove));
    hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[targetPiece] + targetSquare(newMove)];
    capture = true;
  }

  // update bitboards to represent change
  updatePiece(originSquare(newMove), targetSquare(newMove));
  hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[originPiece] + originSquare(newMove)] | ZOBRIST_TABLE[ZobristPieceOffset[originPiece] + targetSquare(newMove)];

  // pawn move
  if (originPieceType == PAWN)
  {
    // promotion
    if (moveType(newMove) == PROMOTION)
    {
      // remove pawn
      // remove piece on target square because it already moved
      deletePiece(targetSquare(newMove));
      hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[originPiece] + targetSquare(newMove)];
      // add promoted piece
      createPiece(makePiece(activeSide, promotion(newMove)), targetSquare(newMove));
      hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[makePiece(activeSide, promotion(newMove))] + targetSquare(newMove)];
    }
    // en passant move
    if (moveType(newMove) == EN_PASSANT)
    {
      BB capturedPawnBB = move(epSquareBB, activeSide ? DOWN : UP);
      int capturedSquare = bitScanForward(capturedPawnBB);
      state->capturedPiece = Piece(piecePos[capturedSquare]);
      deletePiece(capturedSquare);
      hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[state->capturedPiece] + capturedSquare];
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
  zobristToggleCastle(hashValue);
  if (originPiece == WHITE_ROOK || targetPiece == WHITE_ROOK)
  {
    if (originSquareBB == (FILE_H & RANK_1) || targetSquareBB == (FILE_H & RANK_1))
      castleWhiteKingSide = false;
    else if (originSquareBB == (FILE_A & RANK_1) || targetSquareBB == (FILE_A & RANK_1))
      castleWhiteQueenSide = false;
  }
  if (originPiece == BLACK_ROOK || targetPiece == BLACK_ROOK)
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
        hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare] | ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare + 2];
      }
      // castle queen side
      else
      {
        // get rook & move rook
        int rookSquare = activeSide ? 7 : 63;
        updatePiece(rookSquare, rookSquare - 3);
        hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare] | ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare + 2];
      }
    }
  }
  // set it to what it is now
  zobristToggleCastle(hashValue);
  if (capture || originPieceType == PAWN)
    halfMoves = 0;
  else
    halfMoves++;
  if (!activeSide)
    fullMoves++;
  // swap sides
  activeSide = !activeSide;
  hashValue ^= ZOBRIST_TABLE[ACTIVE_SIDE];
  // update key to reflect actual value
  state->hashValue = hashValue;
  // calculate repetitions
  // a halfMove counts up reversible moves so we can use it for repetition counting
  // can only be repetition if atleast 4 half moves where made
  if (halfMoves >= 4)
  {
    // only check every 2nd board because we only check our side moves
    StoredBoard *stateP = state->oldBoard->oldBoard;
    for (int i = 4; i < halfMoves; i += 2)
    {
      if (stateP->oldBoard && stateP->oldBoard->oldBoard)
        stateP = stateP->oldBoard->oldBoard;
      else
        break;
      if (stateP->hashValue == hashValue)
      {
        state->repetition = stateP->repetition == TWO_FOLD ? THREE_FOLD : TWO_FOLD;
        break;
      }
    }
  }
  // unmake move if it was illegal
  if (attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]))
  {
    std::cout << "move was illegal (king attacked): " << toUciString(newMove) << ", Piece: " << CharIndexToPiece[originPiece] << ", Side: " << !activeSide << std::endl;
    std::cout << "side to move: " << activeSide << std::endl;
    printBitboard(attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]));
    prettyPrint();
    printStateHistory();
    unmakeMove(newMove);
    std::cout << "side to move: " << activeSide << std::endl;
    std::cout << "fen (after unmake): " << toFenString() << std::endl;
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
// changes side without moving anything
void Board::makeNullMove()
{
  u64 hashValue = state->hashValue;
  store();

  if (epSquareBB)
  {
    hashValue ^= ZOBRIST_TABLE[EP_SQUARE_H + (bitScanForward(epSquareBB) & 7)];
    epSquareBB = 0;
  }
  halfMoves = 0;
  activeSide = !activeSide;
  hashValue ^= ZOBRIST_TABLE[ACTIVE_SIDE];

  state->hashValue = hashValue;
}

void Board::unmakeNullMove()
{
  restore();
  activeSide = !activeSide;
}