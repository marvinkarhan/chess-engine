#include <vector>
#include <bitset>

#include "constants.h"
#include "movehelper.h"


Square& operator++(Square& s)
{
    if (s == NONE_SQUARE)
        return s; // Do nothing if it's already NONE_SQUARE
    s = static_cast<Square>(static_cast<uint8_t>(s) + 1);
    return s;
}

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
uint8_t STATIC_POP_COUNT_16[1 << 16];
u64 ZOBRIST_TABLE[781];


void initConstants()
{
  for (int i = 0; i <= 63; i++)
    SQUARE_BBS[i] = BB(1) << i;

  u64 seed = ZOBRIST_SEED;
  for (int i = 0; i < ZOBRIST_ARRAY_LENGTH; i++) {
    seed = lcg(seed);
    ZOBRIST_TABLE[i] = seed;
  }

  for (int i = 0; i < (1 << 16); i++)
    STATIC_POP_COUNT_16[i] = uint8_t(std::bitset<16>(i).count());
}