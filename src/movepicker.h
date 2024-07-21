#pragma once
#include "board.h"
#include "move.h"
#include "constants.h"

enum Stages
{
  HASH_STAGE,
  EVALUATE_STAGE,
  WINNING_EQUAL_ATTACKS_STAGE,
  KILLERS_STAGE_1,
  KILLERS_STAGE_2,
  QUIETS_INIT_STAGE,
  QUIETS_STAGE,
};

class MovePicker
{
public:
  // attacksOnly = true will only return good/equal moves
  MovePicker(Board &b, Move hMove, bool attacksOnly = false);
  // gets a next best move
  Move nextMove();

private:
  Board &board;
  ValuedMove *current, *last;
  ValuedMove moves[MAX_MOVES];
  Move hashMove;
  bool inQuiesce;
  int stage = HASH_STAGE;
  ValuedMove *begin() { return current; }
  ValuedMove *end() { return last; }
  // Static Exchange Evaluation (swap variation) to filter winning and equal attacks
  // for more info see https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
  bool see(Move move);
  // adds a score to every move (current to last)
  void evaluate();
  // searches for a move that satisfies the filter condition
  template <typename condition>
  Move searchBest(condition filter);
  // just gets the next move that satisfies the filter condition
  template <typename condition>
  Move searchNext(condition filter);
};
