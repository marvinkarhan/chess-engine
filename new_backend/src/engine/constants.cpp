#include "constants.h"
#include "movehelper.h"
#include <vector>
#include <bitset>

std::string SQUARE_TO_ALGEBRAIC[64] = {
    "h1",
    "g1",
    "f1",
    "e1",
    "d1",
    "c1",
    "b1",
    "a1",
    "h2",
    "g2",
    "f2",
    "e2",
    "d2",
    "c2",
    "b2",
    "a2",
    "h3",
    "g3",
    "f3",
    "e3",
    "d3",
    "c3",
    "b3",
    "a3",
    "h4",
    "g4",
    "f4",
    "e4",
    "d4",
    "c4",
    "b4",
    "a4",
    "h5",
    "g5",
    "f5",
    "e5",
    "d5",
    "c5",
    "b5",
    "a5",
    "h6",
    "g6",
    "f6",
    "e6",
    "d6",
    "c6",
    "b6",
    "a6",
    "h7",
    "g7",
    "f7",
    "e7",
    "d7",
    "c7",
    "b7",
    "a7",
    "h8",
    "g8",
    "f8",
    "e8",
    "d8",
    "c8",
    "b8",
    "a8",
};

BB SQUARE_BBS[64];
BB HORIZONTAL_MOVE_BBS[64];
BB VERTICAL_MOVE_BBS[64];
BB ROOK_MOVE_BBS[64];
BB BISHOP_MOVE_BBS[64];
BB QUEEN_MOVE_BBS[64];
BB KNIGHT_MOVE_BBS[64];
BB KING_MOVES_BBS[64];
BB PAWN_ATTACKS_BBS[64][2];
BB REY_BBS[64][64];
BB LINE_BBS[64][64];
uint8_t STATIC_POP_COUNT_16[1 << 16];
u64 ZOBRIST_TABLE[781];

void initArrRectangular()
{
  std::vector<Direction> leftUp{LEFT_UP};
  std::vector<Direction> leftDown{LEFT_DOWN};
  std::vector<Direction> rightUp{RIGHT_UP};
  std::vector<Direction> rightDown{RIGHT_DOWN};
  std::vector<Direction> left{LEFT};
  std::vector<Direction> right{RIGHT};
  std::vector<Direction> up{UP};
  std::vector<Direction> down{DOWN};
  for (int i = 0; i <= 63; i++)
  {
    BB bb_1 = SQUARE_BBS[i];
    for (int j = 0; j <= 63; j++)
    {
      BB bb_2 = SQUARE_BBS[j];
      if (BISHOP_MOVE_BBS[i] & bb_2)
      {
        BB way_bb_1 = traverse_bb(bb_1, leftUp, 0, bb_2);
        BB way_bb_2 = traverse_bb(bb_1, leftDown, 0, bb_2);
        BB way_bb_3 = traverse_bb(bb_1, rightUp, 0, bb_2);
        BB way_bb_4 = traverse_bb(bb_1, rightDown, 0, bb_2);
        if (way_bb_1 & bb_2)
          REY_BBS[i][j] = way_bb_1 & ~bb_2;
        else if (way_bb_2 & bb_2)
          REY_BBS[i][j] = way_bb_2 & ~bb_2;
        else if (way_bb_3 & bb_2)
          REY_BBS[i][j] = way_bb_3 & ~bb_2;
        else if (way_bb_4 & bb_2)
          REY_BBS[i][j] = way_bb_4 & ~bb_2;
      }
      else if (HORIZONTAL_MOVE_BBS[i] & bb_2)
      {
        BB way_bb_1 = traverse_bb(bb_1, left, 0, bb_2);
        BB way_bb_2 = traverse_bb(bb_1, right, 0, bb_2);
        if (way_bb_1 & bb_2)
          REY_BBS[i][j] = way_bb_1 & ~bb_2;
        else if (way_bb_2 & bb_2)
          REY_BBS[i][j] = way_bb_2 & ~bb_2;
      }
      else if (VERTICAL_MOVE_BBS[i] & bb_2)
      {
        BB way_bb_1 = traverse_bb(bb_1, up, 0, bb_2);
        BB way_bb_2 = traverse_bb(bb_1, down, 0, bb_2);
        if (way_bb_1 & bb_2)
          REY_BBS[i][j] = way_bb_1 & ~bb_2;
        else if (way_bb_2 & bb_2)
          REY_BBS[i][j] = way_bb_2 & ~bb_2;
      }
      else
        REY_BBS[i][j] = 0;
    }
  }
}

void initArrRectangularLines()
{
  for (int i = 0; i <= 63; i++)
  {
    BB bb_1 = SQUARE_BBS[i];
    for (int j = 0; j <= 63; j++)
    {
      BB bb_2 = SQUARE_BBS[j];
      if (BISHOP_MOVE_BBS[i] & bb_2)
        LINE_BBS[i][j] = BISHOP_MOVE_BBS[i] & BISHOP_MOVE_BBS[j] | bb_1 | bb_2;
      else if (HORIZONTAL_MOVE_BBS[i] & bb_2)
        LINE_BBS[i][j] = HORIZONTAL_MOVE_BBS[i] & HORIZONTAL_MOVE_BBS[j] | bb_1 | bb_2;
      else if (VERTICAL_MOVE_BBS[i] & bb_2)
        LINE_BBS[i][j] = VERTICAL_MOVE_BBS[i] & VERTICAL_MOVE_BBS[j] | bb_1 | bb_2;
      else
        LINE_BBS[i][j] = 0;
    }
  }
}

void initConstants()
{
  for (int i = 0; i <= 63; i++)
    SQUARE_BBS[i] = BB(1) << i;

  for (int i = 0; i <= 63; i++)
    HORIZONTAL_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], HORIZONTAL_MOVES, 0, 0);

  for (int i = 0; i <= 63; i++)
    VERTICAL_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], VERTICAL_MOVES, 0, 0);

  for (int i = 0; i <= 63; i++)
    ROOK_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], HORIZONTAL_VERTICAL_MOVES, 0, 0);

  for (int i = 0; i <= 63; i++)
    BISHOP_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], DIAGONALS_MOVES, 0, 0);

  for (int i = 0; i <= 63; i++)
    QUEEN_MOVE_BBS[i] = ROOK_MOVE_BBS[i] | BISHOP_MOVE_BBS[i];

  for (int i = 0; i <= 63; i++)
    KNIGHT_MOVE_BBS[i] = knight_moves(SQUARE_BBS[i], 0);

  for (int i = 0; i <= 63; i++)
    KING_MOVES_BBS[i] = king_moves(SQUARE_BBS[i], 0);

  for (int i = 0; i <= 63; i++)
  {
    for (int j = 0; j <= 1; j++)
      PAWN_ATTACKS_BBS[i][j] = pawn_attacks(SQUARE_BBS[i], j, 0);
  }
  u64 seed = ZOBRIST_SEED;
  for (int i = 0; i < ZOBRIST_ARRAY_LENGTH; i++) {
    seed = lcg(seed);
    ZOBRIST_TABLE[i] = seed;
  }

  initArrRectangular();

  initArrRectangularLines();

  for (int i = 0; i < (1 << 16); i++)
    STATIC_POP_COUNT_16[i] = uint8_t(std::bitset<16>(i).count());
}