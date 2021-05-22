#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stdint.h>
#include <string>
typedef uint64_t BB;
typedef uint64_t u64;
typedef std::string FenString;

const FenString START_POS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/** TODO Fast Piece enums **/
enum Piece : char
{
      WHITE_PAWN = 'P',
      WHITE_KNIGHT = 'N',
      WHITE_BISHOP = 'B',
      WHITE_ROOK = 'R',
      WHITE_QUEEN = 'Q',
      WHITE_KING = 'K',
      BLACK_PAWN = 'p',
      BLACK_KNIGHT = 'n',
      BLACK_BISHOP = 'b',
      BLACK_ROOK = 'r',
      BLACK_QUEEN = 'q',
      BLACK_KING = 'k'
};

constexpr int PROMOTION_OPTIONS_WHITE[4] = {
    Piece::WHITE_KNIGHT,
    Piece::WHITE_BISHOP,
    Piece::WHITE_QUEEN,
    Piece::WHITE_ROOK};

constexpr int PROMOTION_OPTIONS_BLACK[4] = {
    Piece::BLACK_KNIGHT,
    Piece::BLACK_BISHOP,
    Piece::BLACK_QUEEN,
    Piece::BLACK_ROOK};

enum File : int
{
      A,
      B,
      C,
      D,
      E,
      F,
      G,
      H
};

enum Masks : BB
{
      FULL = 0xffffffffffffffff,
      A_FILE = 0x8080808080808080,
      B_FILE = 0x4040404040404040,
      C_FILE = 0x2020202020202020,
      D_FILE = 0x1010101010101010,
      E_FILE = 0x808080808080808,
      F_FILE = 0x404040404040404,
      G_FILE = 0x202020202020202,
      H_FILE = 0x101010101010101,
      NOT_H = 0xfefefefefefefefe,
      NOT_A = 0x7f7f7f7f7f7f7f7f,
      RANK_1 = 0xff,
      RANK_2 = 0xff00,
      RANK_3 = 0xff0000,
      RANK_4 = 0xff000000,
      RANK_5 = 0xff00000000,
      RANK_6 = 0xff0000000000,
      RANK_7 = 0xff000000000000,
      RANK_8 = 0xff00000000000000
};

enum Castling : BB
{
      WHITE_KING_SIDE_WAY = 0x6,
      WHITE_KING_SIDE_KING_WAY = 0xe,
      WHITE_KING_SIDE_SQUARE = 1,

      WHITE_QUEEN_SIDE_WAY = 0x70,
      WHITE_QUEEN_SIDE_KING_WAY = 0x38,
      WHITE_QUEEN_SIDE_SQUARE = 5,

      BLACK_KING_SIDE_WAY = 0x600000000000000,
      BLACK_KING_SIDE_KING_WAY = 0xe00000000000000,
      BLACK_KING_SIDE_SQUARE = 57,

      BLACK_QUEEN_SIDE_WAY = 0x7000000000000000,
      BLACK_QUEEN_SIDE_KING_WAY = 0x3800000000000000,
      BLACK_QUEEN_SIDE_SQUARE = 61
};

constexpr BB CASTLING_OPTIONS[4][2] = {
    {Castling::WHITE_KING_SIDE_SQUARE, Castling::WHITE_KING_SIDE_KING_WAY},
    {Castling::WHITE_QUEEN_SIDE_SQUARE, Castling::WHITE_QUEEN_SIDE_KING_WAY},
    {Castling::BLACK_KING_SIDE_SQUARE, Castling::BLACK_KING_SIDE_KING_WAY},
    {Castling::BLACK_QUEEN_SIDE_SQUARE, Castling::BLACK_QUEEN_SIDE_KING_WAY}};

constexpr BB DEBRUIJN = 0x03f79d71b4cb0a89;
constexpr int DEBRUIJN_INDEX[64] = {
    0, 47, 1, 56, 48, 27, 2, 60,
    57, 49, 41, 37, 28, 16, 3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11, 4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30, 9, 24,
    13, 18, 8, 12, 7, 6, 5, 63};

enum Square : int
{ 
      H1,
      G1,
      F1,
      E1,
      D1,
      C1,
      B1,
      A1,
      H2,
      G2,
      F2,
      E2,
      D2,
      C2,
      B2,
      A2,
      H3,
      G3,
      F3,
      E3,
      D3,
      C3,
      B3,
      A3,
      H4,
      G4,
      F4,
      E4,
      D4,
      C4,
      B4,
      A4,
      H5,
      G5,
      F5,
      E5,
      D5,
      C5,
      B5,
      A5,
      H6,
      G6,
      F6,
      E6,
      D6,
      C6,
      B6,
      A6,
      H7,
      G7,
      F7,
      E7,
      D7,
      C7,
      B7,
      A7,
      H8,
      G8,
      F8,
      E8,
      D8,
      C8,
      B8,
      A8
};

const std::string SQUARE_TO_ALGEBRAIC[64] = {    
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

// TODO MARVIN
// #precalculate moves for squares
// DIAGONALS_MOVE_BBS = [traverse_bb(1 << i, DIAGONALS_MOVES, 0, 0) for i in range(0, 64)]
// HORIZONTAL_VERTICAL_MOVE_BBS = [traverse_bb(1 << i, HORIZONTAL_VERTICAL_MOVES, 0, 0) for i in range(0, 64)]
// HORIZONTAL_MOVE_BBS = [traverse_bb(1 << i, HORIZONTAL_MOVES, 0, 0) for i in range(0, 64)]
// VERTICAL_MOVE_BBS = [traverse_bb(1 << i, VERTICAL_MOVES, 0, 0) for i in range(0, 64)]
// ROOK_MOVE_BBS = HORIZONTAL_VERTICAL_MOVE_BBS
// BISHOP_MOVE_BBS = DIAGONALS_MOVE_BBS
// QUEEN_MOVE_BBS = [HORIZONTAL_VERTICAL_MOVE_BBS[i] | DIAGONALS_MOVE_BBS[i] for i in range(0, 64)]
// KNIGHT_MOVE_BBS  = [knight_moves(1 << i, 0) for i in range(0, 64)]
// PAWN_MOVE_BBS = [[pawn_moves(1 << i, j, 0, 0) for j in [0, 1]] for i in range(0, 64)]
// PAWN_ATTACKS_BBS = [[pawn_attacks(1 << i, j, 0) for j in [0, 1]] for i in range(0, 64)]
// KING_MOVES_BBS = [king_moves(1 << i, 0) for i in range(0, 64)]
// SQUARE_BBS = [1 << i for i in range(0, 64)]

// def arr_Rectangular():
//       cdef u64[64][64] rays
//       rays = [[0 for i in range(0, 64)] for j in range(0, 64)]
//       for i, bb_1 in enumerate(SQUARE_BBS):
//             for j, bb_2 in enumerate(SQUARE_BBS):
//                   if DIAGONALS_MOVE_BBS[i] & bb_2:
//                         way_bb_1 = traverse_bb(bb_1, [move_left_up], 0, bb_2)
//                         way_bb_2 = traverse_bb(bb_1, [move_left_down], 0, bb_2)
//                         way_bb_3 = traverse_bb(bb_1, [move_right_up], 0, bb_2)
//                         way_bb_4 = traverse_bb(bb_1, [move_right_down], 0, bb_2)
//                         if way_bb_1 & bb_2:
//                               rays[i][j] = way_bb_1 & ~bb_2
//                         elif way_bb_2 & bb_2:
//                               rays[i][j] = way_bb_2 & ~bb_2
//                         elif way_bb_3 & bb_2:
//                               rays[i][j] = way_bb_3 & ~bb_2
//                         elif way_bb_4 & bb_2:
//                               rays[i][j] = way_bb_4 & ~bb_2
//                   elif HORIZONTAL_MOVE_BBS[i] & bb_2:
//                         way_bb_1 = traverse_bb(bb_1, [move_left], 0, bb_2)
//                         way_bb_2 = traverse_bb(bb_1, [move_right], 0, bb_2)
//                         if way_bb_1 & bb_2:
//                               rays[i][j] = way_bb_1 & ~bb_2
//                         elif way_bb_2 & bb_2:
//                               rays[i][j] = way_bb_2 & ~bb_2
//                   elif VERTICAL_MOVE_BBS[i] & bb_2:
//                         way_bb_1 = traverse_bb(bb_1, [move_up], 0, bb_2)
//                         way_bb_2 = traverse_bb(bb_1, [move_down], 0, bb_2)
//                         if way_bb_1 & bb_2:
//                               rays[i][j] = way_bb_1 & ~bb_2
//                         elif way_bb_2 & bb_2:
//                               rays[i][j] = way_bb_2 & ~bb_2
//                   else:
//                         rays[i][j] = 0
//       return rays

// cdef u64[64][64] REY_BBS = arr_Rectangular()

// def arr_Rectangular_lines():
//       cdef u64[64][64] rays
//       rays = [[0 for i in range(0, 64)] for j in range(0, 64)]
//       for i, bb_1 in enumerate(SQUARE_BBS):
//             for j, bb_2 in enumerate(SQUARE_BBS):
//                   if DIAGONALS_MOVE_BBS[i] & bb_2:
//                         rays[i][j] = DIAGONALS_MOVE_BBS[i] & DIAGONALS_MOVE_BBS[j] | bb_1 | bb_2
//                   elif HORIZONTAL_MOVE_BBS[i] & bb_2:
//                         rays[i][j] = HORIZONTAL_MOVE_BBS[i] & HORIZONTAL_MOVE_BBS[j] | bb_1 | bb_2
//                   elif VERTICAL_MOVE_BBS[i] & bb_2:
//                         rays[i][j] = VERTICAL_MOVE_BBS[i] & VERTICAL_MOVE_BBS[j] | bb_1 | bb_2
//                   else:
//                         rays[i][j] = 0
//       return rays

// cdef u64[64][64] LINE_BBS = arr_Rectangular_lines()

/** Based on https: //en.wikipedia.org/wiki/Linear_congruential_generator */
constexpr u64 lcg(u64 seed)
{
      return (2787869 * seed + 17767698) % 0xfdab38264; //Some Random values for pseudo random generation
}
constexpr int ZOBRIST_TABLE[781];

// EVALUATE_TABLE = {}

// import json

// with open('opening-extractor/output/openings.json') as json_file:
//     OPENING_TABLE = json.load(json_file)

 #endif // CONSTANTS_H