START_POS_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

cdef u64 FULL_BB_MASK = 0xffffffffffffffff

# masks for lines
cdef u64 A = 0x8080808080808080
cdef u64 B = 0x4040404040404040
cdef u64 C = 0x2020202020202020
cdef u64 D = 0x1010101010101010
cdef u64 E = 0x808080808080808
cdef u64 F = 0x404040404040404
cdef u64 G = 0x202020202020202
cdef u64 H = 0x101010101010101
cdef u64 NOT_H = 0xfefefefefefefefe
cdef u64 NOT_A = 0x7f7f7f7f7f7f7f7f
# masks for ranks
cdef u64 R1 = 0xff
cdef u64 R2 = 0xff00
cdef u64 R3 = 0xff0000
cdef u64 R4 = 0xff000000
cdef u64 R5 = 0xff00000000
cdef u64 R6 = 0xff0000000000
cdef u64 R7 = 0xff000000000000
cdef u64 R8 = 0xff00000000000000


cdef u64 CASTLING_W_KING_SIDE_WAY = 0x6
cdef u64 CASTLING_W_KING_SIDE_KING_WAY = 0xe
cdef u64 CASTLING_W_KING_SIDE_SQUARE = 1

cdef u64 CASTLING_W_QUEEN_SIDE_WAY = 0x70
cdef u64 CASTLING_W_QUEEN_SIDE_KING_WAY = 0x38
cdef u64 CASTLING_W_QUEEN_SIDE_SQUARE = 5

cdef u64 CASTLING_B_KING_SIDE_WAY = 0x600000000000000
cdef u64 CASTLING_B_KING_SIDE_KING_WAY = 0xe00000000000000
cdef u64 CASTLING_B_KING_SIDE_SQUARE = 57

cdef u64 CASTLING_B_QUEEN_SIDE_WAY = 0x7000000000000000
cdef u64 CASTLING_B_QUEEN_SIDE_KING_WAY = 0x3800000000000000
cdef u64 CASTLING_B_QUEEN_SIDE_SQUARE = 61
cdef u64[4][2] CASTELING_ARR = [
      [CASTLING_W_KING_SIDE_SQUARE, CASTLING_W_KING_SIDE_KING_WAY],
      [CASTLING_W_QUEEN_SIDE_SQUARE, CASTLING_W_QUEEN_SIDE_KING_WAY],
      [CASTLING_B_KING_SIDE_SQUARE, CASTLING_B_KING_SIDE_KING_WAY],
      [CASTLING_B_QUEEN_SIDE_SQUARE, CASTLING_B_QUEEN_SIDE_KING_WAY],
]

cdef u64 debruijn64 = 0x03f79d71b4cb0a89
cdef int[64] debruijn64_index64 = [
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
]

FIELDS_TO_INDEX = [
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
] = range(63, -1, -1)

ALGEBRAIC_TO_INDEX = {
    'a8': 63, 'b8': 62, 'c8': 61, 'd8': 60, 'e8': 59, 'f8': 58, 'g8': 57, 'h8': 56,
    'a7': 55, 'b7': 54, 'c7': 53, 'd7': 52, 'e7': 51, 'f7': 50, 'g7': 49, 'h7': 48,
    'a6': 47, 'b6': 46, 'c6': 45, 'd6': 44, 'e6': 43, 'f6': 42, 'g6': 41, 'h6': 40,
    'a5': 39, 'b5': 38, 'c5': 37, 'd5': 36, 'e5': 35, 'f5': 34, 'g5': 33, 'h5': 32,
    'a4': 31, 'b4': 30, 'c4': 29, 'd4': 28, 'e4': 27, 'f4': 26, 'g4': 25, 'h4': 24,
    'a3': 23, 'b3': 22, 'c3': 21, 'd3': 20, 'e3': 19, 'f3': 18, 'g3': 17, 'h3': 16,
    'a2': 15, 'b2': 14, 'c2': 13, 'd2': 12, 'e2': 11, 'f2': 10, 'g2': 9, 'h2': 8,
    'a1': 7, 'b1': 6, 'c1': 5, 'd1': 4, 'e1': 3, 'f1': 2, 'g1': 1, 'h1': 0,
}

PIECE_VALUES = {
    'P': 100, 'p': -100,
    'Q': 900, 'q': -900,
    'B': 330, 'b': -330,
    'N': 320, 'n': -320,
    'R': 500, 'r': -500,
    'K': 20000, 'k': -20000

}

PIECE_SQUARE_TABLES = {
    'P': [0,  0,  0,  0,  0,  0,  0,  0,
          50, 50, 50, 50, 50, 50, 50, 50,
          10, 10, 20, 30, 30, 20, 10, 10,
          5,  5, 10, 25, 25, 10,  5,  5,
          0,  0,  0, 20, 20,  0,  0,  0,
          5, -5, -10,  0,  0, -10, -5,  5,
          5, 10, 10, -20, -20, 10, 10,  5,
          0,  0,  0,  0,  0,  0,  0,  0],

    'p': [0,  0,  0,  0,  0,  0,  0,  0,
          5, 10, 10, -20, -20, 10, 10,  5,
          5, -5, -10,  0,  0, -10, -5,  5,
          0,  0,  0, 20, 20,  0,  0,  0,
          5,  5, 10, 25, 25, 10,  5,  5,
          10, 10, 20, 30, 30, 20, 10, 10,
          50, 50, 50, 50, 50, 50, 50, 50,
          0,  0,  0,  0,  0,  0,  0,  0],
    'N': [-50, -40, -30, -30, -30, -30, -40, -50,
          -40, -20,  0,  0,  0,  0, -20, -40,
          -30,  0, 10, 15, 15, 10,  0, -30,
          -30,  5, 15, 20, 20, 15,  5, -30,
          -30,  0, 15, 20, 20, 15,  0, -30,
          -30,  5, 10, 15, 15, 10,  5, -30,
          -40, -20,  0,  5,  5,  0, -20, -40,
          -50, -40, -30, -30, -30, -30, -40, -50],
    'n': [-50, -40, -30, -30, -30, -30, -40, -50,
          -40, -20,  0,  5,  5,  0, -20, -40,
          -30,  5, 10, 15, 15, 10,  5, -30,
          -30,  0, 15, 20, 20, 15,  0, -30,
          -30,  5, 15, 20, 20, 15,  5, -30,
          -30,  0, 10, 15, 15, 10,  0, -30,
          -40, -20,  0,  0,  0,  0, -20, -40,
          -50, -40, -30, -30, -30, -30, -40, -50],
    'B': [-20, -10, -10, -10, -10, -10, -10, -20,
          -10,  0,  0,  0,  0,  0,  0, -10,
          -10,  0,  5, 10, 10,  5,  0, -10,
          -10,  5,  5, 10, 10,  5,  5, -10,
          -10,  0, 10, 10, 10, 10,  0, -10,
          -10, 10, 10, 10, 10, 10, 10, -10,
          -10,  5,  0,  0,  0,  0,  5, -10,
          -20, -10, -10, -10, -10, -10, -10, -20],
    'b': [-20, -10, -10, -10, -10, -10, -10, -20,
          -10,  5,  0,  0,  0,  0,  5, -10,
          -10, 10, 10, 10, 10, 10, 10, -10,
          -10,  0, 10, 10, 10, 10,  0, -10,
          -10,  5,  5, 10, 10,  5,  5, -10,
          -10,  0,  5, 10, 10,  5,  0, -10,
          -10,  0,  0,  0,  0,  0,  0, -10,
          -20, -10, -10, -10, -10, -10, -10, -20],
    'R': [0,  0,  0,  0,  0,  0,  0,  0,
          5, 10, 10, 10, 10, 10, 10,  5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          0,  0,  0,  5,  5,  0,  0,  0],
    'r': [0,  0,  0,  5,  5,  0,  0,  0,
          - 5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          -5,  0,  0,  0,  0,  0,  0, -5,
          5, 10, 10, 10, 10, 10, 10,  5,
          0,  0,  0,  0,  0,  0,  0,  0],
    'Q': [-20, -10, -10, -5, -5, -10, -10, -20,
          -10,  0,  0,  0,  0,  0,  0, -10,
          -10,  0,  5,  5,  5,  5,  0, -10,
          -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
          -10,  5,  5,  5,  5,  5,  0, -10,
          -10,  0,  5,  0,  0,  0,  0, -10,
          -20, -10, -10, -5, -5, -10, -10, -20],
    'q': [-20, -10, -10, -5, -5, -10, -10, -20,
          -10,  0,  5,  0,  0,  0,  0, -10,
          -10,  5,  5,  5,  5,  5,  0, -10,
          0,  0,  5,  5,  5,  5,  0, -5,
          -5,  0,  5,  5,  5,  5,  0, -5,
          -10,  0,  5,  5,  5,  5,  0, -10,
          -10,  0,  0,  0,  0,  0,  0, -10,
          -20, -10, -10, -5, -5, -10, -10, -20],
    'K': [-30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -20, -30, -30, -40, -40, -30, -30, -20,
          -10, -20, -20, -20, -20, -20, -20, -10,
          20, 20,  0,  0,  0,  0, 20, 20,
          20, 30, 10,  0,  0, 10, 30, 20],
    'k': [20, 30, 10,  0,  0, 10, 30, 20,
          20, 20,  0,  0,  0,  0, 20, 20,
          -10, -20, -20, -20, -20, -20, -20, -10,
          -20, -30, -30, -40, -40, -30, -30, -20,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30]


}

PROMOTION_OPTIONS_W = ['N', 'B', 'Q', 'R']
PROMOTION_OPTIONS_B = ['n', 'b', 'q', 'r']

from move_helper import *

# precalculate moves for squares
DIAGONALS_MOVE_BBS = [traverse_bb(1 << i, DIAGONALS_MOVES, 0, 0) for i in range(0, 64)]
HORIZONTAL_VERTICAL_MOVE_BBS = [traverse_bb(1 << i, HORIZONTAL_VERTICAL_MOVES, 0, 0) for i in range(0, 64)]
HORIZONTAL_MOVE_BBS = [traverse_bb(1 << i, HORIZONTAL_MOVES, 0, 0) for i in range(0, 64)]
VERTICAL_MOVE_BBS = [traverse_bb(1 << i, VERTICAL_MOVES, 0, 0) for i in range(0, 64)]
ROOK_MOVE_BBS = HORIZONTAL_VERTICAL_MOVE_BBS
BISHOP_MOVE_BBS = DIAGONALS_MOVE_BBS
QUEEN_MOVE_BBS = [HORIZONTAL_VERTICAL_MOVE_BBS[i] | DIAGONALS_MOVE_BBS[i] for i in range(0, 64)]
KNIGHT_MOVE_BBS  = [knight_moves(1 << i, 0) for i in range(0, 64)]
PAWN_MOVE_BBS = [[pawn_moves(1 << i, j, 0, 0) for j in [0, 1]] for i in range(0, 64)]
PAWN_ATTACKS_BBS = [[pawn_attacks(1 << i, j, 0) for j in [0, 1]] for i in range(0, 64)]
KING_MOVES_BBS = [king_moves(1 << i, 0) for i in range(0, 64)]
SQUARE_BBS = [1 << i for i in range(0, 64)]


def arr_Rectangular():
      cdef u64[64][64] rays
      rays = [[0 for i in range(0, 64)] for j in range(0, 64)]
      for i, bb_1 in enumerate(SQUARE_BBS):
            for j, bb_2 in enumerate(SQUARE_BBS):
                  if DIAGONALS_MOVE_BBS[i] & bb_2:
                        way_bb_1 = traverse_bb(bb_1, [move_left_up], 0, bb_2)
                        way_bb_2 = traverse_bb(bb_1, [move_left_down], 0, bb_2)
                        way_bb_3 = traverse_bb(bb_1, [move_right_up], 0, bb_2)
                        way_bb_4 = traverse_bb(bb_1, [move_right_down], 0, bb_2)
                        if way_bb_1 & bb_2:
                              rays[i][j] = way_bb_1 & ~bb_2
                        elif way_bb_2 & bb_2:
                              rays[i][j] = way_bb_2 & ~bb_2
                        elif way_bb_3 & bb_2:
                              rays[i][j] = way_bb_3 & ~bb_2
                        elif way_bb_4 & bb_2:
                              rays[i][j] = way_bb_4 & ~bb_2
                  elif HORIZONTAL_MOVE_BBS[i] & bb_2:
                        way_bb_1 = traverse_bb(bb_1, [move_left], 0, bb_2)
                        way_bb_2 = traverse_bb(bb_1, [move_right], 0, bb_2)
                        if way_bb_1 & bb_2:
                              rays[i][j] = way_bb_1 & ~bb_2
                        elif way_bb_2 & bb_2:
                              rays[i][j] = way_bb_2 & ~bb_2
                  elif VERTICAL_MOVE_BBS[i] & bb_2:
                        way_bb_1 = traverse_bb(bb_1, [move_up], 0, bb_2)
                        way_bb_2 = traverse_bb(bb_1, [move_down], 0, bb_2)
                        if way_bb_1 & bb_2:
                              rays[i][j] = way_bb_1 & ~bb_2
                        elif way_bb_2 & bb_2:
                              rays[i][j] = way_bb_2 & ~bb_2
                  else:
                        rays[i][j] = 0
      return rays

cdef u64[64][64] REY_BBS = arr_Rectangular()

def arr_Rectangular_lines():
      cdef u64[64][64] rays
      rays = [[0 for i in range(0, 64)] for j in range(0, 64)]
      for i, bb_1 in enumerate(SQUARE_BBS):
            for j, bb_2 in enumerate(SQUARE_BBS):
                  if DIAGONALS_MOVE_BBS[i] & bb_2:
                        rays[i][j] = DIAGONALS_MOVE_BBS[i] & DIAGONALS_MOVE_BBS[j] | bb_1 | bb_2
                  elif HORIZONTAL_MOVE_BBS[i] & bb_2:
                        rays[i][j] = HORIZONTAL_MOVE_BBS[i] & HORIZONTAL_MOVE_BBS[j] | bb_1 | bb_2
                  elif VERTICAL_MOVE_BBS[i] & bb_2:
                        rays[i][j] = VERTICAL_MOVE_BBS[i] & VERTICAL_MOVE_BBS[j] | bb_1 | bb_2
                  else:
                        rays[i][j] = 0
      return rays

cdef u64[64][64] LINE_BBS = arr_Rectangular_lines()

# Based on https://en.wikipedia.org/wiki/Linear_congruential_generator
cdef lcg(u64 modulus, u64 a, u64 b, u64 c, u64 seed, int amount):
    """Linear congruential generator."""
    cdef int index
    cdef list numbers
    numbers = []
    for i in range(0,amount):
        seed = (a * seed + c) % modulus
        numbers.append(seed)

    return numbers


ZOBRIST_TABLE = lcg(0xfdab38264, 2787869, 17767698, 5786987, 107987, 781)
EVALUATE_TABLE = {}

import json

with open('opening-extractor/output/openings.json') as json_file:
    OPENING_TABLE = json.load(json_file)