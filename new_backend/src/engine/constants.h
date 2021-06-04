#pragma once

#include <stdint.h>
#include <string>

typedef uint64_t BB;
typedef uint64_t u64;
typedef std::string FenString;

const FenString START_POS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const int MAX_MOVES = 256;
const int MAX_DEPTH = 20;

enum MoveGenType
{
  PSEUDO_LEGAL_MOVES,
  LEGAL_MOVES
};

// 3 lsb for piece type and the 4th for side identification
enum Piece : int
{
  NO_PIECE = 0,
  BLACK_PAWN,
  BLACK_KNIGHT,
  BLACK_BISHOP,
  BLACK_ROOK,
  BLACK_QUEEN,
  BLACK_KING,
  WHITE_PAWN = 9,
  WHITE_KNIGHT,
  WHITE_BISHOP,
  WHITE_ROOK,
  WHITE_QUEEN,
  WHITE_KING,
};

const Piece PIECE_ENUMERATED[] = {
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
};

const std::string CharIndexToPiece(" pnbrqk  PNBRQK");

constexpr bool
getPieceSide(Piece piece)
{
  return piece >> 3;
}

enum PieceType : int
{
  ALL_PIECES = 0,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
};

constexpr Piece makePiece(bool activeSide, PieceType pt)
{
  return Piece((activeSide << 3) + pt);
}

constexpr PieceType getPieceType(Piece piece)
{
  return PieceType(piece & 0b111);
}

constexpr Piece PROMOTION_OPTIONS_WHITE[4] = {
    Piece::WHITE_KNIGHT,
    Piece::WHITE_BISHOP,
    Piece::WHITE_QUEEN,
    Piece::WHITE_ROOK};

constexpr Piece PROMOTION_OPTIONS_BLACK[4] = {
    Piece::BLACK_KNIGHT,
    Piece::BLACK_BISHOP,
    Piece::BLACK_QUEEN,
    Piece::BLACK_ROOK};

enum Mask : BB
{
  FULL = 0xffffffffffffffff,
  FILE_A = 0x8080808080808080,
  FILE_B = 0x4040404040404040,
  FILE_C = 0x2020202020202020,
  FILE_D = 0x1010101010101010,
  FILE_E = 0x808080808080808,
  FILE_F = 0x404040404040404,
  FILE_G = 0x202020202020202,
  FILE_H = 0x101010101010101,
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

// lookuptable, indexed by the piece enum in constants
constexpr int PieceValues[15] = {
    0,
    -100,
    -330,
    -320,
    -500,
    -900,
    -20000,
    0,
    0,
    100,
    320,
    330,
    500,
    900,
    20000};

// lookuptable, indexed by the piece enum in constants
constexpr int PIECE_SQUARE_TABLES[15][64] = {
    {0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0,
     5, 10, 10, -20, -20, 10, 10, 5,
     5, -5, -10, 0, 0, -10, -5, 5,
     0, 0, 0, 20, 20, 0, 0, 0,
     5, 5, 10, 25, 25, 10, 5, 5,
     10, 10, 20, 30, 30, 20, 10, 10,
     50, 50, 50, 50, 50, 50, 50, 50,
     0, 0, 0, 0, 0, 0, 0, 0},
    {-50, -40, -30, -30, -30, -30, -40, -50,
     -40, -20, 0, 5, 5, 0, -20, -40,
     -30, 5, 10, 15, 15, 10, 5, -30,
     -30, 0, 15, 20, 20, 15, 0, -30,
     -30, 5, 15, 20, 20, 15, 5, -30,
     -30, 0, 10, 15, 15, 10, 0, -30,
     -40, -20, 0, 0, 0, 0, -20, -40,
     -50, -40, -30, -30, -30, -30, -40, -50},
    {-20, -10, -10, -10, -10, -10, -10, -20,
     -10, 5, 0, 0, 0, 0, 5, -10,
     -10, 10, 10, 10, 10, 10, 10, -10,
     -10, 0, 10, 10, 10, 10, 0, -10,
     -10, 5, 5, 10, 10, 5, 5, -10,
     -10, 0, 5, 10, 10, 5, 0, -10,
     -10, 0, 0, 0, 0, 0, 0, -10,
     -20, -10, -10, -10, -10, -10, -10, -20},
    {0, 0, 0, 5, 5, 0, 0, 0, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, 5, 10, 10, 10, 10, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0},
    {-20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 5, 0, 0, 0, 0, -10, -10, 5, 5, 5, 5, 5, 0, -10, 0, 0, 5, 5, 5, 5, 0, -5, -5, 0, 5, 5, 5, 5, 0, -5, -10, 0, 5, 5, 5, 5, 0, -10, -10, 0, 0, 0, 0, 0, 0, -10, -20, -10, -10, -5, -5, -10, -10, -20},
    {20, 30, 10, 0, 0, 10, 30, 20, 20, 20, 0, 0, 0, 0, 20, 20, -10, -20, -20, -20, -20, -20, -20, -10, -20, -30, -30, -40, -40, -30, -30, -20, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30, 20, 10, 10, 5, 5, 10, 25, 25, 10, 5, 5, 0, 0, 0, 20, 20, 0, 0, 0, 5, -5, -10, 0, 0, -10, -5, 5, 5, 10, 10, -20, -20, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0},
    {-50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 0, 0, 0, -20, -40, -30, 0, 10, 15, 15, 10, 0, -30, -30, 5, 15, 20, 20, 15, 5, -30, -30, 0, 15, 20, 20, 15, 0, -30, -30, 5, 10, 15, 15, 10, 5, -30, -40, -20, 0, 5, 5, 0, -20, -40, -50, -40, -30, -30, -30, -30, -40, -50},
    {-20, -10, -10, -10, -10, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 5, 5, 10, 10, 5, 5, -10, -10, 0, 10, 10, 10, 10, 0, -10, -10, 10, 10, 10, 10, 10, 10, -10, -10, 5, 0, 0, 0, 0, 5, -10, -20, -10, -10, -10, -10, -10, -10, -20},
    {0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0},
    {-20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -5, 0, 5, 5, 5, 5, 0, -5, 0, 0, 5, 5, 5, 5, 0, -5, -10, 5, 5, 5, 5, 5, 0, -10, -10, 0, 5, 0, 0, 0, 0, -10, -20, -10, -10, -5, -5, -10, -10, -20},
    {-30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20, -20, -20, -20, -10, 20, 20, 0, 0, 0, 0, 20, 20, 20, 30, 10, 0, 0, 10, 30, 20}};

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

extern std::string SQUARE_TO_ALGEBRAIC[64];

// precalculate moves for squares
extern BB SQUARE_BBS[64];
extern BB HORIZONTAL_MOVE_BBS[64];
extern BB VERTICAL_MOVE_BBS[64];
extern BB ROOK_MOVE_BBS[64];
extern BB BISHOP_MOVE_BBS[64];
extern BB QUEEN_MOVE_BBS[64];
extern BB KNIGHT_MOVE_BBS[64];
extern BB KING_MOVES_BBS[64];
extern BB PAWN_ATTACKS_BBS[64][2];
extern BB REY_BBS[64][64];
extern BB LINE_BBS[64][64];

extern void initConstants();

/** Based on https: //en.wikipedia.org/wiki/Linear_congruential_generator */
constexpr u64 lcg(u64 seed)
{
  return (2787869 * seed + 17767698) % 0xfdab38264; //Some Random values for pseudo random generation
}
// constexpr int ZOBRIST_TABLE[781];

// EVALUATE_TABLE = {}

// import json

// with open('opening-extractor/output/openings.json') as json_file:
//     OPENING_TABLE = json.load(json_file)