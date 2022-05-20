#include <algorithm>
#include "movepicker.h"
#include "movehelper.h"
#include "board.h"
#include "move.h"

MovePicker::MovePicker(Board &b, Move hMove, bool onlyAttacks /*= false*/) : board(b), hashMove(hMove), onlyWinningEqualAttacks(onlyAttacks)
{
  if (board.isKingAttacked())
    if (hMove && b.moveIsPseudoLegal(hMove))
      stage = EVASION_HASH_STAGE;
    else
      stage = EVASIONS_INIT_STAGE;
  else if (hMove && b.moveIsPseudoLegal(hMove))
    stage = HASH_STAGE;
  else
    stage = ATTACKS_INIT_STAGE;
}

Move MovePicker::nextMove()
{
  switch (stage)
  {
  case EVASION_HASH_STAGE:
  case HASH_STAGE:
    stage++;
    return hashMove;
  case ATTACKS_INIT_STAGE:
    current = losingCapturesEnd = moves;
    last = board.generateLegalMoves(current, board.activeSide, ATTACKS);
    evaluate<ATTACKS>();
    stage++;
    [[fallthrough]];
  case WINNING_EQUAL_ATTACKS_STAGE:
    if (searchBest([&]()
                   {
                     // use see to filter moves and if see eval -> bad move add it to bad moves
                     return see(*current) ? true : (*losingCapturesEnd++ = *current, false);
                   }))
      return *(current - 1);
    if (onlyWinningEqualAttacks)
    {
      // stop searching
      return NONE_MOVE;
      // stage = LOSING_ATTACKS_INIT_STAGE;
      // return nextMove();
    }
    stage++;
    [[fallthrough]];
  case KILLERS_INIT_STAGE:
    // swap pointer to the killer move array
    current = std::begin(board.killerMoves[board.ply]);
    last = std::end(board.killerMoves[board.ply]);

    stage++;
    [[fallthrough]];
  case KILLERS_STAGE:
    if (searchNext([&]()
                   { return *current != NONE_MOVE && board.moveIsPseudoLegal(*current); }))
      return *(current - 1);

    stage++;
    [[fallthrough]];
  case QUIETS_INIT_STAGE:
    // skip bad captures
    current = losingCapturesEnd;
    last = board.generateLegalMoves(current, board.activeSide, QUIETS);

    evaluate<QUIETS>();
    std::sort(current, last);

    stage++;
    [[fallthrough]];
  case QUIETS_STAGE:
    if (searchNext([&]()
                   { return *current != board.killerMoves[board.ply][0].move &&
                            *current != board.killerMoves[board.ply][1].move; }))
      return *(current - 1);
    stage++;
    [[fallthrough]];
  case LOSING_ATTACKS_INIT_STAGE:
    current = moves;
    last = losingCapturesEnd;
    stage++;
    [[fallthrough]];
  case LOSING_ATTACKS_STAGE:
    return searchNext([]()
                      { return true; });
    // done
  case EVASIONS_INIT_STAGE:
    current = moves;
    last = board.generateLegalMoves(current, board.activeSide, EVASIONS);

    evaluate<EVASIONS>();
    std::sort(current, last);

    stage++;
    [[fallthrough]];
  case EVASIONS_STAGE:
    return searchBest([]()
                      { return true; });
    // done
  }
  return NONE_MOVE; // fallback
}

bool MovePicker::see(Move move)
{
  // discard EP_MOVES
  if (moveType(move) != NORMAL)
    return true;

  // btw we don't care about pinned pieces (prob to much of a time waste and complexity)

  int gain[32], d = 0;
  BB mayXRay = board.pieces(PAWN) | board.pieces(BISHOP) | board.pieces(ROOK) | board.pieces(QUEEN);
  int originSq = originSquare(move);
  int targetSq = targetSquare(move);
  BB occupiedBB = board.piecesByType[ALL_PIECES];
  bool side = board.activeSide;
  BB attackersDefendersBB = board.attackers(targetSq, side, occupiedBB) | board.attackers(targetSq, !side, occupiedBB);
  gain[d] = NormalizedPieceValues[board.piecePos[targetSq]];
  BB bb;
  try
  {
    do
    {
      // next depth and side
      d++;
      // swap sides
      side = !side;
      gain[d] = NormalizedPieceValues[board.piecePos[originSq]] - gain[d - 1]; // speculative store, if defended
      if (std::max(-gain[d - 1], gain[d]) < 0)
        break; // pruning does not influence the result

      // reset bit in set to traverse
      attackersDefendersBB ^= SQUARE_BBS[originSq];
      // reset bit in temporary occupancy (for x-Rays)
      occupiedBB ^= SQUARE_BBS[originSq];

      // add discovered attackers if needed
      if (SQUARE_BBS[originSq] & mayXRay) {
        if (SQUARE_BBS[originSq] & board.pieces(PAWN) || SQUARE_BBS[originSq] & board.pieces(BISHOP) || SQUARE_BBS[originSq] & board.pieces(QUEEN)) {
          attackersDefendersBB |= (bishop_moves(SQUARE_BBS[targetSq], ~occupiedBB, BB(0)) & occupiedBB) & (board.pieces(!side, BISHOP) | board.pieces(!side, QUEEN));
        } else if (SQUARE_BBS[originSq] & board.pieces(ROOK) || SQUARE_BBS[originSq] & board.pieces(QUEEN)) {
          attackersDefendersBB |= (rook_moves(SQUARE_BBS[targetSq], ~occupiedBB, BB(0)) & occupiedBB) & (board.pieces(!side, ROOK) | board.pieces(!side, QUEEN));
        }
      }      // select next piece in least to most valuable order
      if ((bb = attackersDefendersBB & board.pieces(side, PAWN)))
        originSq = pop_lsb(bb);
      else if ((bb = attackersDefendersBB & board.pieces(side, KNIGHT)))
        originSq = pop_lsb(bb);
      else if ((bb = attackersDefendersBB & board.pieces(side, BISHOP)))
        originSq = pop_lsb(bb);
      else if ((bb = attackersDefendersBB & board.pieces(side, ROOK)))
        originSq = pop_lsb(bb);
      else if ((bb = attackersDefendersBB & board.pieces(side, QUEEN)))
        originSq = pop_lsb(bb);
      else if ((bb = attackersDefendersBB & board.pieces(side, KING)))
        originSq = pop_lsb(bb);
      else
        originSq = 64;
    } while (originSq < 64);
  }
  catch (...)
  {
  }
  // propagate result up
  while (--d)
    gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
  return bool(gain[0] >= 0);
}

template <MoveGenCategory category>
void MovePicker::evaluate()
{
  for (auto &move : *this)
  {
    if constexpr (category == ATTACKS)
      // order by MVV-LVA
      move.value = mvvLva[board.piecePos[originSquare(move)]][board.piecePos[targetSquare(move)]];
    else if constexpr (category == QUIETS)
      // order by piece square table -> should be replaced by history heuristics later
      move.value = PIECE_SQUARE_TABLES[board.piecePos[originSquare(move)]][63 - originSquare(move)] - PIECE_SQUARE_TABLES[board.piecePos[targetSquare(move)]][63 - targetSquare(move)];
    // move.value = board.historyHeuristicTable[board.piecePos[originSquare(move)]][targetSquare(move)];
    else // category == EVASIONS
    {
      if (board.piecePos[targetSquare(move)] || moveType(move) == EN_PASSANT)
        // if capture do MVV-LVA
        move.value = mvvLva[board.piecePos[originSquare(move)]][board.piecePos[targetSquare(move)]];
      else
        // order by piece square table -> should be replaced by history heuristics later
        move.value = PIECE_SQUARE_TABLES[board.piecePos[originSquare(move)]][63 - originSquare(move)] - PIECE_SQUARE_TABLES[board.piecePos[targetSquare(move)]][63 - targetSquare(move)];
      // move.value = board.historyHeuristicTable[board.piecePos[originSquare(move)]][targetSquare(move)];
    }
  }
}

template <typename condition>
Move MovePicker::searchBest(condition filter)
{
  while (current < last)
  {
    // find biggest element and swap it to the current position
    std::swap(*current, *std::max_element(current, last));

    // always skip the hashMove because it already has been searched
    // then look for the filter cond
    // and return if found
    if (*current != hashMove && filter())
      return *current++; // will be best move because of the swap earlier

    current++;
  }
  return NONE_MOVE; // fallback
}

template <typename condition>
Move MovePicker::searchNext(condition filter)
{
  while (current < last)
  {
    // always skip the hashMove because it already has been searched
    // and return if found
    if (*current != hashMove && filter())
      return *current++; // will be best move because of the swap earlier

    current++;
  }
  return NONE_MOVE; // fallback
}