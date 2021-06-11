#pragma once

#include <stdint.h>
#include <string>

typedef uint64_t BB;
typedef uint64_t u64;
typedef std::string FenString;

const int MIN_ALPHA = -2000000;
const int MIN_BETA = 2000000;
const int CHECKMATE_VALUE = -20000;
const int SCORE_UNKNOWN = MIN_ALPHA - 1;
const FenString START_POS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const int MAX_MOVES = 256;
const int MAX_DEPTH = 64;

enum MoveGenType
{
  PSEUDO_LEGAL_MOVES,
  LEGAL_MOVES
};

enum MoveGenCategory
{
  ALL,
  EVASIONS,
  ATTACKS,
  QUIETS,
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

constexpr PieceType PROMOTION_OPTIONS[4] = {
    KNIGHT,
    BISHOP,
    QUEEN,
    ROOK};

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
    -320,
    -330,
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

constexpr int NormalizedPieceValues[15] = {
    0,
    100,
    320,
    330,
    500,
    900,
    20000,
    0,
    0,
    100,
    320,
    330,
    500,
    900,
    20000};

// arbitrary MVV-LVA lookup table
constexpr int mvvLva[15][15] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                         // empty
    {0, 106, 206, 306, 406, 506, 606, 0, 0, 106, 206, 306, 406, 506, 606}, // pawn
    {0, 105, 205, 305, 405, 505, 605, 0, 0, 105, 205, 305, 405, 505, 605}, // knight
    {0, 104, 204, 304, 404, 504, 604, 0, 0, 104, 204, 304, 404, 504, 604}, // bishop
    {0, 103, 203, 303, 403, 503, 603, 0, 0, 103, 203, 303, 403, 503, 603}, // rook
    {0, 101, 201, 301, 401, 501, 601, 0, 0, 101, 201, 301, 401, 501, 601}, // queen
    {0, 100, 200, 300, 400, 500, 600, 0, 0, 100, 200, 300, 400, 500, 600}, // king
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 106, 206, 306, 406, 506, 606, 0, 0, 106, 206, 306, 406, 506, 606}, // pawn
    {0, 105, 205, 305, 405, 505, 605, 0, 0, 105, 205, 305, 405, 505, 605}, // knight
    {0, 104, 204, 304, 404, 504, 604, 0, 0, 104, 204, 304, 404, 504, 604}, // bishop
    {0, 103, 203, 303, 403, 503, 603, 0, 0, 103, 203, 303, 403, 503, 603}, // rook
    {0, 101, 201, 301, 401, 501, 601, 0, 0, 101, 201, 301, 401, 501, 601}, // queen
    {0, 100, 200, 300, 400, 500, 600, 0, 0, 100, 200, 300, 400, 500, 600}, // king
};
enum AdditionalMaterial
{
  BISHOP_PAIR = 50,
  ROOK_PAIR = -20,
  KNIGHT_PAIR = -20,
  NO_PAWNS = -400,
};

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

//ZOBRIST STUFF
extern u64 ZOBRIST_TABLE[781];
const int ZOBRIST_ARRAY_LENGTH = 781;
const u64 ZOBRIST_SEED = 13;

enum ZobristKeys : int
{
  ACTIVE_SIDE = 768,
  CASTLE_WHITE_KING_SIDE,
  CASTLE_WHITE_QUEEN_SIDE,
  CASTLE_BLACK_KING_SIDE,
  CASTLE_BLACK_QUEEN_SIDE,
  EP_SQUARE_H,
  EP_SQUARE_G,
  EP_SQUARE_F,
  EP_SQUARE_E,
  EP_SQUARE_D,
  EP_SQUARE_C,
  EP_SQUARE_B,
  EP_SQUARE_A,
};

constexpr int ZobristPieceOffset[15] = {
    0,
    0,
    64,
    124,
    192,
    256,
    320,
    0,
    0,
    384,
    448,
    512,
    576,
    640,
    704};

extern void initConstants();

/** Based on https: //en.wikipedia.org/wiki/Linear_congruential_generator */
constexpr u64 lcg(u64 seed)
{
  return seed * 6364136223846793005ULL + 1442695040888963407ULL;
  // return (2787869 * seed + 17767698) % 0xfdab38264; //Some Random values for pseudo random generation
}

enum HashEntryFlag
{
  // exact value = no cut-off
  EXACT,
  // alpha cut-off
  UPPER_BOUND,
  // beta cut-off
  LOWER_BOUND
};
