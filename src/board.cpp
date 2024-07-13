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
#include "nnue/init.h"

int64_t timeNow() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Board::Board(FenString fen /*=START_POS_FEN*/)
{
  initHashTableSize(DEFAULT_HASH_TABLE_SIZE);
  fullMoves = 0;
  state = nullptr;
  stopSearch = false;
  parseFenString(fen);

  useNNUE = NNUE::init();
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

int Board::evaluateNextMove(float movetime /*= 0*/, float wtime /*= 0*/, float btime /*= 0*/, int maxDepth /*= MAX_DEPTH*/, bool print /*= true*/)
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
  int score = iterativeDeepening(movetime, maxDepth, print);
  return score;
}

int Board::iterativeDeepening(float timeInSeconds /*= std::numeric_limits<float>::max()*/, int maxDepth /*= MAX_DEPTH*/, bool print /*= true*/)
{
  int64_t startTime = timeNow();
  if (timeInSeconds)
  {
    // search for atleast 200ms
    timeInSeconds = std::max(0.2f, timeInSeconds);
    endTime = startTime + ((long long)(timeInSeconds * 1000));
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
        if (print && (pv[0] == NONE_MOVE || MoveList<ALL>(*this, activeSide).size() == 0))
        {
          std::cout << "info depth 0 score mate 0" << std::endl;
          std::cout << "bestmove (none)" << std::endl;
          return 0;
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
      auto timePassed = timeNow() - startTime + 1;
      if (print)
      {
        std::cout << "info depth " << currDepth << " nodes " << nodeCount << " nps " << nodeCount * 1000 / timePassed;
        printScore(latestScore, pv);
        std::cout << " pv";
        for (Move move : pv)
        {
          std::cout << " " << toUciString(move);
        }
        std::cout << std::endl;
      }
      currDepth++;
    }
  }
  stopSearch = true;
  if (print)
  {
    if (pv.size() > 0 && pv.at(0) != NONE_MOVE)
      // signal we made out final decision if there are moves left to make
      std::cout << "bestmove " << toUciString(pv[0]) << std::endl;
    else
      std::cout << "bestmove (none)" << std::endl;
  }

  return latestScore;
}

void Board::printScore(int score, std::vector<Move> &pv)
{
  std::cout << " score";
  // check if its mate
  if (score < CHECKMATE_VALUE || score > -CHECKMATE_VALUE)
  {
    std::cout << " mate " << (std::ceil(((pv.size() - 1) / 2)) + 1) * (activeSide ? -1 : 1);
  }
  else
  {
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


void Board::printBitboard(BB bb) {
  std::ostringstream result;
  std::string lineSeparator = "+---+---+---+---+---+---+---+---+";

  result << "   " << lineSeparator << "\r\n";
  for (int i = 0; i < 8; ++i) {
    result << " " << (8 - i) << " |";
    for (int j = 0; j < 8; ++j) {
      result << " " << ((bb & (1ULL << ((7 - i) * 8 + (7 - j)))) ? "1" : "0") << " |";
    }
    result << "\r\n   " << lineSeparator << "\r\n";
  }
  result << "     A   B   C   D   E   F   G   H" << std::endl;
  std::cout << result.str();
}

void Board::prettyPrint()
{
  std::string lineSeparator = "+---+---+---+---+---+---+---+---+";
  for (int i = 7; i >= 0; i--)
  {
    std::cout << "   " + lineSeparator << std::endl;
    std::cout << " " << i + 1 << " | ";
    for (int j = 7; j >= 0; j--)
    {
      std::cout << CharIndexToPiece[piecePos[i * 8 + j]] << " | ";
    }
    std::cout << std::endl;
  }
  std::cout << "   " + lineSeparator << std::endl;
  std::cout << "     A   B   C   D   E   F   G   H" << std::endl;
}

void Board::printHashTable()
{
  HashEntry *entry = probeHash();
  std::cout << "Best move:  " << entry->bestMove << "\n"
            << "Depth:      " << entry->depth << "\n"
            << "Flag:       " << entry->flag << "\n"
            << "Key:        " << entry->key << "\n"
            << "Score:      " << entry->score << std::endl;
}

int Board::quiesce(int alpha, int beta, int depth /*= 0*/)
{
  // track time control in interval
  if ((nodeCount & 2047) == 0)
  {
    stopSearch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() > endTime;
  }

  nodeCount++;

  int score, bestScore;
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

  // only do standPat when king is not attacked
  bool kingChecked = isKingAttacked();
  if (kingChecked) {
    bestScore = MIN_ALPHA;
  } else {
    bestScore = evaluate();
    if (bestScore >= beta)
      return beta;
    if (bestScore > alpha)
      alpha = bestScore;
  }

  int moveCounter = 0;
  MovePicker movePicker(*this, bestMove, !kingChecked);
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

    moveCounter++;

    if (score > bestScore) {
      bestScore = score;
      if (score > alpha) {
        alpha = score;
        if (score >= beta)
          break;
      }
    }
  }
  if (kingChecked && moveCounter == 0)
    return CHECKMATE_VALUE + ply;

  return bestScore;
}

int Board::evaluate()
{
  if (useNNUE)
  {
    return NNUE::evaluate(*this);
  }
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

  int sideMultiplier = activeSide ? 1 : -1;
  return score * sideMultiplier;
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
  if (moveCounter == 0)
    // checkmate or stalemate
    return isKingAttacked() ? CHECKMATE_VALUE - depth : 0;

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
    std::cout << std::string(1, CharIndexToPiece[piece]) + ":" << std::endl;
    printBitboard(pieces(piece));
    std::cout << std::endl;
  }
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
    else if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z'))
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

BB Board::attackers(int square, bool activeSide, BB occupied)
{
  // get attackers
  const BB rookBB = pieces(!activeSide, ROOK);
  const BB bishopBB = pieces(!activeSide, BISHOP);
  const BB queenBB = pieces(!activeSide, QUEEN);
  const BB knightBB = pieces(!activeSide, KNIGHT);
  const BB pawnBB = pieces(!activeSide, PAWN);
  const BB kingBB = pieces(!activeSide, KING);

  const BB raysBB = bishop_moves(square, occupied);
  const BB linesBB = rook_moves(square, occupied);

  BB attackersBB = raysBB & (bishopBB | queenBB);
  attackersBB |= linesBB & (rookBB | queenBB);
  attackersBB |= KNIGHT_MOVE_BBS[square] & knightBB;
  attackersBB |= PAWN_ATTACKS_BBS[activeSide][square] & pawnBB;
  attackersBB |= KING_MOVES_BBS[square] & kingBB;

  return attackersBB;
}

// get attacked squares from enemy
// used for king move gen
BB Board::attackedSquares(bool activeSide, BB emptyAndEmpty)
{
  BB attackBB = BB(0);
  
  int ownKingSquare = bitScanForward(pieces(!activeSide, KING));
  int kingSquare = bitScanForward(pieces(activeSide, KING));

  BB pawnsBB = pieces(activeSide, PAWN);
  BB knightsBB = pieces(activeSide, KNIGHT);
  // add queen to rooks and bishops as it is a combination of both
  BB queensBB = pieces(activeSide, QUEEN);
  BB bishopsBB = pieces(activeSide, BISHOP) | queensBB;
  BB rooksBB = pieces(activeSide, ROOK) | queensBB;

  BB occupiedBB = piecesByType[ALL_PIECES];

  BB kingAttacksBB = KING_MOVES_BBS[ownKingSquare] & emptyAndEmpty;

  // if king is surrounded none of these squares can be attacked
  if (kingAttacksBB == 0ULL) {
    return 0ULL;
  }

  // remove the king
  occupiedBB &= ~SQUARE_BBS[ownKingSquare];

  attackBB |= pawn_attacks(pawnsBB, activeSide, 0ULL);

  while (knightsBB)
  {
    int knightSquare = pop_lsb(knightsBB);
    attackBB |= KNIGHT_MOVE_BBS[knightSquare];
  }

  while (bishopsBB)
  {
    int bishopSquare = pop_lsb(bishopsBB);
    attackBB |= bishop_moves(bishopSquare, occupiedBB);
  }

  while (rooksBB)
  {
    int rookSquare = pop_lsb(rooksBB);
    attackBB |= rook_moves(rookSquare, occupiedBB);
  }

  attackBB |= KING_MOVES_BBS[kingSquare];

  return attackBB;
}

// if square is attacked provides a BB with alls squares that are attacked on the path to the king
// including the attacking piece
// additionally Knights and Pawns are added
// tracks double checks by reference
// return FULL BB is square is not attacked
BB Board::checkedSquares(int square, bool activeSide, int &double_check)
{
  BB checkedBB = BB(0);
  double_check = 0;

  BB enemyKnightBB = pieces(!activeSide, KNIGHT);
  BB enemyPawnBB = pieces(!activeSide, PAWN);
  // add queen to rooks and bishops as it is a combination of both
  BB enemyQueenBB = pieces(!activeSide, QUEEN);
  BB enemyBishopBB = pieces(!activeSide, BISHOP) | enemyQueenBB;
  BB enemyRookBB = pieces(!activeSide, ROOK) | enemyQueenBB;

  BB occupiedBB = piecesByType[ALL_PIECES];

  // knight checks
  BB knightAttacksBB = KNIGHT_MOVE_BBS[square] & enemyKnightBB;
  double_check += bool(knightAttacksBB);
  checkedBB |= knightAttacksBB;

  // pawn checks
  BB pawnAttacksBB = PAWN_ATTACKS_BBS[activeSide][square] & enemyPawnBB;
  double_check += bool(pawnAttacksBB);
  checkedBB |= pawnAttacksBB;

  // bishop checks
  BB bishopAttacksBB = bishop_moves(square, occupiedBB) & enemyBishopBB;
  if (bishopAttacksBB)
  {
    int bishopSquare = bitScanForward(bishopAttacksBB);

    double_check++;
    checkedBB |= in_between(square, bishopSquare) | SQUARE_BBS[bishopSquare];
  }

  BB rookAttacksBB = rook_moves(square, occupiedBB) & enemyRookBB;
  if (rookAttacksBB)
  {
    // immediate can occur on pawn promotion
    if (popCount(rookAttacksBB) > 1)
    {
      double_check = 2;
      return checkedBB;
    }
    int rookSquare = bitScanForward(rookAttacksBB);

    double_check++;
    checkedBB |= in_between(square, rookSquare) | SQUARE_BBS[rookSquare];
  }

  if (!checkedBB)
  {
    return FULL;
  }

  return checkedBB;
}

// path from enemy rooks/queens to pinned pieces
// keeping in mind double checks
BB Board::horizontalVerticalPinned(int square, bool activeSide, BB friendliesBB, BB enemiesBB)
{
  BB hvPinnedBB = BB(0);

  BB enemyRookBB = pieces(!activeSide, ROOK);
  BB enemyQueenBB = pieces(!activeSide, QUEEN);

  BB rookAttacksBB = rook_moves(square, enemiesBB) & (enemyRookBB | enemyQueenBB);

  while (rookAttacksBB)
  {
    int rookSquare = pop_lsb(rookAttacksBB);

    BB potentialPin = in_between(square, rookSquare) | SQUARE_BBS[rookSquare];
    if (popCount(potentialPin & friendliesBB) == 1)
    {
      hvPinnedBB |= potentialPin;
    }
  }
  return hvPinnedBB;
}

BB Board::diagonalPinned(int square, bool activeSide, BB friendliesBB, BB enemiesBB)
{
  BB diagonalPinnedBB = BB(0);

  BB enemyBishopBB = pieces(!activeSide, BISHOP);
  BB enemyQueenBB = pieces(!activeSide, QUEEN);

  BB bishopAttacksBB = bishop_moves(square, enemiesBB) & (enemyBishopBB | enemyQueenBB);

  while (bishopAttacksBB)
  {
    int bishopSquare = pop_lsb(bishopAttacksBB);

    BB potentialPin = in_between(square, bishopSquare) | SQUARE_BBS[bishopSquare];
    if (popCount(potentialPin & friendliesBB) == 1)
    {
      diagonalPinnedBB |= potentialPin;
    }
  }
  return diagonalPinnedBB;
}

std::vector<Move> Board::getPV()
{
  std::vector<Move> moves;
  if (stopSearch || ((pvLength[0] == 0) && !latestPv.empty()))
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

ValuedMove *Board::generateLegalMoves(ValuedMove *moveList, bool activeSide, MoveGenCategory category)
{
  int double_check = 0;

  BB kingBB = pieces(activeSide, KING);
  int kingSquare = bitScanForward(kingBB);
  BB friendliesBB = piecesBySide[activeSide];
  BB enemiesBB = piecesBySide[!activeSide];
  BB occupiedBB = piecesByType[ALL_PIECES];
  BB enemyAndEmptyBB = ~friendliesBB;

  BB attackedBB = attackedSquares(!activeSide, enemyAndEmptyBB);
  BB checkedBB = checkedSquares(kingSquare, activeSide, double_check);
  BB hvPinnedBB = horizontalVerticalPinned(kingSquare, activeSide, friendliesBB, enemiesBB);
  BB diagonalPinnedBB = diagonalPinned(kingSquare, activeSide, friendliesBB, enemiesBB);

  BB targetSquaresBB;

  if (category == ALL)
    targetSquaresBB = enemyAndEmptyBB;
  else if (category == ATTACKS)
    targetSquaresBB = enemiesBB;
  else if (category == EVASIONS)
    targetSquaresBB = checkedBB;
  else // if (category == QUIETS)
    targetSquaresBB = ~occupiedBB;

  // king moves
  BB kingMoves = KING_MOVES_BBS[kingSquare] & targetSquaresBB & ~attackedBB;
  
  // ensure that king is not attacked after moving
  targetSquaresBB &= checkedBB;

  while (kingMoves)
  {
    int targetSquare = pop_lsb(kingMoves);
    *moveList++ = createMove(kingSquare, targetSquare);
  }

  // castle
  if (category != ATTACKS && category != EVASIONS &&
      (kingBB & (activeSide ? RANK_1 : RANK_8)) &&
      checkedBB == FULL)
  {
    if (activeSide)
    {
      // check if sth is in the way, dont check if is legal to castle
      if (castleWhiteKingSide && !(WHITE_KING_SIDE_WAY & occupiedBB) && !(attackedBB & WHITE_KING_SIDE_WAY))
        *moveList++ = createMove<CASTLING>(kingSquare, WHITE_KING_SIDE_SQUARE);
      if (castleWhiteQueenSide && !(WHITE_QUEEN_SIDE_WAY_OCCUPIED & occupiedBB) && !(attackedBB & WHITE_QUEEN_SIDE_WAY))
        *moveList++ = createMove<CASTLING>(kingSquare, WHITE_QUEEN_SIDE_SQUARE);
    }
    else
    {
      if (castleBlackKingSide && !(BLACK_KING_SIDE_WAY & occupiedBB) && !(attackedBB & BLACK_KING_SIDE_WAY))
        *moveList++ = createMove<CASTLING>(kingSquare, BLACK_KING_SIDE_SQUARE);
      if (castleBlackQueenSide && !(BLACK_QUEEN_SIDE_WAY_OCCUPIED & occupiedBB) && !(attackedBB & BLACK_QUEEN_SIDE_WAY))
        *moveList++ = createMove<CASTLING>(kingSquare, BLACK_QUEEN_SIDE_SQUARE);
    }
  }

  // if double check only king moves are possible
  if (double_check > 1)
    return moveList;

  // remove pinned pieces
  BB knightBB = pieces(activeSide, KNIGHT) & ~(hvPinnedBB | diagonalPinnedBB);
  BB bishopBB = pieces(activeSide, BISHOP) & ~hvPinnedBB;
  BB rookBB = pieces(activeSide, ROOK) & ~diagonalPinnedBB;
  BB queenBB = pieces(activeSide, QUEEN) & ~(hvPinnedBB & diagonalPinnedBB);

  // pawn moves
  BB pawnBB = pieces(activeSide, PAWN);
  // generate vars to handle pawn colors easier
  BB R3orR6 = activeSide ? RANK_3 : RANK_6;
  BB R2orR7 = activeSide ? RANK_7 : RANK_2;
  BB R1orR8 = activeSide ? RANK_8 : RANK_1;
  Direction moveDirection = activeSide ? UP : DOWN;
  int directionFactor = activeSide ? 1 : -1;

  // pawns that can move diagonal (attacks)
  BB diagonalPawnBB = pawnBB & ~hvPinnedBB;
  BB diagonalUnpinnedPawnBB = diagonalPawnBB & ~diagonalPinnedBB;
  BB diagonalPinnedPawnBB = pawnBB & diagonalPinnedBB;

  // get bb for pawns that can actually move diagonally
  // keep in mind that pinned pawns can only move in the direction they are pinned
  BB diagonalLeftPawnBB = pawn_left_attacks(diagonalUnpinnedPawnBB, activeSide) | (pawn_left_attacks(diagonalPinnedPawnBB, activeSide) & diagonalPinnedBB);
  BB diagonalRightPawnBB = pawn_right_attacks(diagonalUnpinnedPawnBB, activeSide) | (pawn_right_attacks(diagonalPinnedPawnBB, activeSide) & diagonalPinnedBB);
  diagonalLeftPawnBB &= enemiesBB & checkedBB;
  diagonalRightPawnBB &= enemiesBB & checkedBB;

  // pawns that can move one/two
  BB forwardPawnBB = pawnBB & ~diagonalPinnedBB;
  BB forwardUnpinnedPawnBB = forwardPawnBB & ~hvPinnedBB;
  BB forwardPinnedPawnBB = pawnBB & hvPinnedBB;

  BB oneForwardPawnBB = ((move(forwardUnpinnedPawnBB, moveDirection) & ~occupiedBB) | (move(forwardPinnedPawnBB, moveDirection) & ~occupiedBB & hvPinnedBB));
  BB doubleForwardPawnBB = (move(oneForwardPawnBB & R3orR6, moveDirection) & ~occupiedBB) & checkedBB;
  oneForwardPawnBB &= checkedBB;

  if ((category != QUIETS) && pawnBB & R2orR7)
  {
    BB promoteLeftPawnBB = diagonalLeftPawnBB & R1orR8;
    while (promoteLeftPawnBB)
    {
      int targetSquare = pop_lsb(promoteLeftPawnBB);
      for (PieceType promotion : PROMOTION_OPTIONS)
        *moveList++ = createMove<PROMOTION>(targetSquare - 9 * directionFactor, targetSquare, promotion);
    }
    BB promoteRightPawnBB = diagonalRightPawnBB & R1orR8;
    while (promoteRightPawnBB)
    {
      int targetSquare = pop_lsb(promoteRightPawnBB);
      for (PieceType promotion : PROMOTION_OPTIONS)
        *moveList++ = createMove<PROMOTION>(targetSquare - 7 * directionFactor, targetSquare, promotion);
    }
    BB promoteForwardPawnBB = oneForwardPawnBB & R1orR8;
    while (promoteForwardPawnBB)
    {
      int targetSquare = pop_lsb(promoteForwardPawnBB);
      for (PieceType promotion : PROMOTION_OPTIONS)
        *moveList++ = createMove<PROMOTION>(targetSquare - 8 * directionFactor, targetSquare, promotion);
    }
  }
  // remove promoted pawns
  oneForwardPawnBB &= ~R1orR8;
  diagonalLeftPawnBB &= ~R1orR8;
  diagonalRightPawnBB &= ~R1orR8;
  
  if (category != ATTACKS)
  {
    // single forward pawn moves
    while (oneForwardPawnBB)
    {
      int targetSquare = pop_lsb(oneForwardPawnBB);
      *moveList++ = createMove(targetSquare - 8 * directionFactor, targetSquare);
    }

    // double forward pawn moves
    while (doubleForwardPawnBB)
    {
      int targetSquare = pop_lsb(doubleForwardPawnBB);
      *moveList++ = createMove(targetSquare - (8 << 1) * directionFactor, targetSquare);
    }
  }
  
  if (category != QUIETS)
  {
    // diagonal left pawn attacks
    while (diagonalLeftPawnBB)
    {
      int targetSquare = pop_lsb(diagonalLeftPawnBB);
      *moveList++ = createMove(targetSquare - 9 * directionFactor, targetSquare);
    }
    // diagonal right pawn attacks
    while (diagonalRightPawnBB)
    {
      int targetSquare = pop_lsb(diagonalRightPawnBB);
      *moveList++ = createMove(targetSquare - 7 * directionFactor, targetSquare);
    }
    // en passant
    if (epSquareBB && (targetSquaresBB & move(epSquareBB, activeSide ? DOWN : UP)))
    {
      int epSquare = bitScanForward(epSquareBB);
      int epPawnSquare = epSquare - 8 * directionFactor;

      if (((epSquareBB | SQUARE_BBS[epPawnSquare]) & checkedBB) != 0)
      {
        // check if king is on ep rank and there are enemy queens and rooks left
        BB enemyQueenRookBB = pieces(!activeSide, QUEEN) | pieces(!activeSide, ROOK);
        bool potentialKingPin = (int)(kingSquare / 8) == (int)(epPawnSquare / 8) && enemyQueenRookBB;

        BB epBB = PAWN_ATTACKS_BBS[!activeSide][epSquare] & diagonalPawnBB;
        // may be multiple pawns that can take en passant
        while (epBB) {
            int originSquare = pop_lsb(epBB);

            // remove pinned pawns that don't move on the pinned diagonal
            if (SQUARE_BBS[originSquare] & diagonalPinnedBB && !(diagonalPinnedBB & SQUARE_BBS[epSquare])) continue;

            // check for enemy queen/rook on the same rank as the en passant pawn
            if (potentialKingPin &&
                (rook_moves(kingSquare, occupiedBB & ~(SQUARE_BBS[epPawnSquare] | SQUARE_BBS[originSquare])) & enemyQueenRookBB) != 0)
                break;

            *moveList++ = createMove<EN_PASSANT>(originSquare, epSquare);
        }
      }
    }
  }

  // knight moves
  while (knightBB) {
    int originSquare = pop_lsb(knightBB);
    BB knightMoves = KNIGHT_MOVE_BBS[originSquare] & targetSquaresBB;
    while (knightMoves) {
      int targetSquare = pop_lsb(knightMoves);
      *moveList++ = createMove(originSquare, targetSquare);
    }
  }

  // bishop moves
  while (bishopBB) {
    int originSquare = pop_lsb(bishopBB);
    BB bishopMoves = bishop_moves(originSquare, occupiedBB) & targetSquaresBB;
    if (diagonalPinnedBB & SQUARE_BBS[originSquare])
      bishopMoves &= diagonalPinnedBB;
    while (bishopMoves) {
      int targetSquare = pop_lsb(bishopMoves);
      *moveList++ = createMove(originSquare, targetSquare);
    }
  }
  
  // rook moves
  while (rookBB) {
    int originSquare = pop_lsb(rookBB);
    // check for horizontal/vertical pins
    BB rookMoves = rook_moves(originSquare, occupiedBB) & targetSquaresBB;
    if (hvPinnedBB & SQUARE_BBS[originSquare])
      rookMoves &= hvPinnedBB;
    while (rookMoves) {
      int targetSquare = pop_lsb(rookMoves);
      *moveList++ = createMove(originSquare, targetSquare);
    }
  }

  // queen moves
  while (queenBB) {
    int originSquare = pop_lsb(queenBB);
    BB queenMoves = targetSquaresBB;
    if (diagonalPinnedBB & SQUARE_BBS[originSquare])
      queenMoves &= bishop_moves(originSquare, occupiedBB) & diagonalPinnedBB;
    else if (hvPinnedBB & SQUARE_BBS[originSquare])
      queenMoves &= rook_moves(originSquare, occupiedBB) & hvPinnedBB;
    else
      queenMoves &= queen_moves(originSquare, occupiedBB);
    while (queenMoves) {
      int targetSquare = pop_lsb(queenMoves);
      *moveList++ = createMove(originSquare, targetSquare);
    }
  }

  return moveList;
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
    MoveList moveList = MoveList<ALL>(*this, activeSide);
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
  if (halfMoves >= 100 && (!kingAttackers() || MoveList(*this, activeSide).size()))
    return true;

  return state->repetition;
}

bool Board::stalemate()
{
  MoveList moveIterator = MoveList<ALL>(*this, activeSide);
  return moveIterator.size() == 0;
}

bool Board::checkmate()
{
  BB kingAttackersBB = kingAttackers();
  if (kingAttackersBB)
  {
    MoveList moveIterator = MoveList<EVASIONS>(*this, activeSide);
    return moveIterator.size() == 0;
  }
  return false;
}

u64 Board::perft(int depth)
{
  u64 nodes = 0;
  if (depth == 0)
    return 1ULL;
  for (auto move : MoveList<ALL>(*this, activeSide))
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
  MoveList moveList = MoveList<ALL>(*this, activeSide);
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

constexpr HashEntry *Board::probeHash()
{
  return &hashTable[state->hashValue % hashTableSize];
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

  state->accumulator.computed_accumulation = false;
  auto &dirtyPiece = state->dirtyPiece;
  dirtyPiece.dirty_num = 1;

  state->move = newMove;
  // DEBUG
  // if (originPiece == NO_PIECE)
  // {
  //   prettyPrint();
  //   std::cout << "move was illegal (no piece on origin square): " << toUciString(newMove) << std::endl;
  //   return false;
  // }

  // target piece only exists on capture
  if (targetPiece)
  {
    // DEBUG
    // if (in_between(originSquare(newMove), targetSquare(newMove)) & piecesByType[ALL_PIECES])
    //   std::cout << "capture thru friendly piece" << std::endl;
    deletePiece(targetSquare(newMove));
    hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[targetPiece] + targetSquare(newMove)];
    capture = true;

    if (USE_NNUE)
    {
      dirtyPiece.dirty_num = 2;
      // add removed piece
      dirtyPiece.piece[1] = targetPiece;
      dirtyPiece.from[1] = toNNUESquare(targetSquare(newMove));
      dirtyPiece.to[1] = NONE_SQUARE;
    }
  }

  if (USE_NNUE)
  {
    // add removed piece
    dirtyPiece.piece[0] = originPiece;
    dirtyPiece.from[0] = toNNUESquare(originSquare(newMove));
    dirtyPiece.to[0] = toNNUESquare(targetSquare(newMove));
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

      if (USE_NNUE)
      {
        dirtyPiece.to[0] = NONE_SQUARE;
        dirtyPiece.piece[dirtyPiece.dirty_num] = makePiece(activeSide, promotion(newMove));
        dirtyPiece.from[dirtyPiece.dirty_num] = NONE_SQUARE;
        dirtyPiece.to[dirtyPiece.dirty_num] = toNNUESquare(targetSquare(newMove));
        dirtyPiece.dirty_num++;
      }
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
      int rookSquare;
      int rookTargetSquare;
      // castle king side
      if (targetSquare(newMove) == originSquare(newMove) - 2)
      {
        // get rook & move rook
        rookSquare = activeSide ? 0 : 56;
        rookTargetSquare = rookSquare + 2;
        updatePiece(rookSquare, rookTargetSquare);
        hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare] | ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare + 2];
      }
      // castle queen side
      else
      {
        // get rook & move rook
        rookSquare = activeSide ? 7 : 63;
        rookTargetSquare = rookSquare - 3;
        updatePiece(rookSquare, rookTargetSquare);
        hashValue ^= ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare] | ZOBRIST_TABLE[ZobristPieceOffset[activeSide ? WHITE_ROOK : BLACK_ROOK] + rookSquare + 2];
      }
      if (USE_NNUE)
      {
        dirtyPiece.dirty_num = 2;
        dirtyPiece.piece[1] = makePiece(activeSide, ROOK);
        dirtyPiece.from[1] = toNNUESquare(rookSquare);
        dirtyPiece.to[1] = toNNUESquare(rookTargetSquare);
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
  // DEBUG
  // unmake move if it was illegal
  // if (attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]))
  // {
  //   std::cout << "move was illegal (king attacked): " << toUciString(newMove) << ", Piece: " << CharIndexToPiece[originPiece] << ", Side: " << !activeSide << std::endl;
  //   std::cout << "side to move: " << activeSide << std::endl;
  //   printBitboard(attackers(bitScanForward(pieces(!activeSide, KING)), !activeSide, piecesByType[ALL_PIECES]));
  //   prettyPrint();
  //   printStateHistory();
  //   unmakeMove(newMove);
  //   std::cout << "side to move: " << activeSide << std::endl;
  //   std::cout << "fen (after unmake): " << toFenString() << std::endl;
  //   throw;
  //   return false;
  // }
  return true;
}

void Board::unmakeMove(const Move &oldMove)
{
  activeSide = !activeSide;

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

void Board::printStateHistory()
{
  StoredBoard *currState = state;
  int total = 0;
  while (currState)
  {
    currState = currState->oldBoard;
    total++;
  }
  std::cout << "Current states stored in history: " << std::to_string(total) << std::endl;
}
int Board::getStateHistory()
{
  StoredBoard *currState = state;
  int total = 0;
  while (currState)
  {
    currState = currState->oldBoard;
    total++;
  }
  return total;
}