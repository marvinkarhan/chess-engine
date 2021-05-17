from constants cimport *
from move import *
from functools import lru_cache, cache
import re

@lru_cache(maxsize=None)
def set_bit_on_bb(u64 bb, int index, int value):
    cdef u64 bb_mask = 1 << index
    # remove potential bit
    bb &= ~bb_mask
    # add bit if value is true
    if value:
        bb |= bb_mask
    return bb


"""
following functions are helper functions
they take a bitboard (bb)
the bb must only contain one piece
the piece (bit) is then shifted
if a shift results in a piece wrapping the board (going from line A to H) it gets returned as 0
"""

@lru_cache(maxsize=None)
def move_left(u64 bb):
    return (bb << 1 & ~H) & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_right(u64 bb):
    return bb >> 1 & ~A


@lru_cache(maxsize=None)
def move_up(u64 bb):
    return bb << 8 & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_down(u64 bb):
    return bb >> 8


# castle optimizations
@lru_cache(maxsize=None)
def move_leftx2(u64 bb):
    return ((bb << 2 & ~H) & ~G) & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_leftx3(u64 bb):
    return (((bb << 3 & ~H) & ~G) & ~F) & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_rightx2(u64 bb):
    return ((bb >> 2 & ~A) & ~ B)


# pawn optimizations
@lru_cache(maxsize=None)
def move_downx2(u64 bb):
    return bb >> 16


@lru_cache(maxsize=None)
def move_upx2(u64 bb):
    return bb << 16 & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_left_up(u64 bb):
    return (bb << 9 & ~H) & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_left_down(u64 bb):
    return bb >> 7 & ~H


@lru_cache(maxsize=None)
def move_right_up(u64 bb):
    return (bb << 7 & ~A) & FULL_BB_MASK


@lru_cache(maxsize=None)
def move_right_down(u64 bb):
    return bb >> 9 & ~A


"""
following functions move generator functions
they take a bitboard (bb) containing one pice
the bb must only contain one piece
"""
HORIZONTAL_VERTICAL_MOVES = [move_left, move_right, move_up, move_down]
DIAGONALS_MOVES = [move_left_up, move_left_down,
                   move_right_up, move_right_down]
DIRECTIONS = HORIZONTAL_VERTICAL_MOVES + DIAGONALS_MOVES
KNIGHT_MOVES = [
    lambda x: move_leftx2(move_up(x)),
    lambda x: move_leftx2(move_down(x)),
    lambda x: move_upx2(move_left(x)),
    lambda x: move_upx2(move_right(x)),
    lambda x: move_rightx2(move_up(x)),
    lambda x: move_rightx2(move_down(x)),
    lambda x: move_downx2(move_left(x)),
    lambda x: move_downx2(move_right(x))
]

"""
Following Board Filling Functions are using the Kogge-Stone Algorithm
"""
@lru_cache(maxsize=None)
def south_occluded(u64 bb, u64 empty_bb):
    bb |= empty_bb & (bb >> 8)
    empty_bb &= (empty_bb >> 8)
    bb |= empty_bb & (bb >> 16)
    empty_bb &= (empty_bb >> 16)
    bb |= empty_bb & (bb >> 32)
    return bb


@lru_cache(maxsize=None)
def north_occluded(u64 bb, u64 empty_bb):
    bb |= empty_bb & (bb << 8)
    empty_bb &= (empty_bb << 8)
    bb |= empty_bb & (bb << 16)
    empty_bb &= (empty_bb << 16)
    bb |= empty_bb & (bb << 32)
    return bb


@lru_cache(maxsize=None)
def west_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_H
    bb |= empty_bb & (bb << 1)
    empty_bb &= (empty_bb << 1)
    bb |= empty_bb & (bb << 2)
    empty_bb &= (empty_bb << 2)
    bb |= empty_bb & (bb << 4)
    return bb


@lru_cache(maxsize=None)
def no_we_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_H
    bb |= empty_bb & (bb << 9)
    empty_bb &= (empty_bb << 9)
    bb |= empty_bb & (bb << 18)
    empty_bb &= (empty_bb << 18)
    bb |= empty_bb & (bb << 36)
    return bb


@lru_cache(maxsize=None)
def so_we_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_H
    bb |= empty_bb & (bb >> 7)
    empty_bb &= (empty_bb >> 7)
    bb |= empty_bb & (bb >> 14)
    empty_bb &= (empty_bb >> 14)
    bb |= empty_bb & (bb >> 28)
    return bb


@lru_cache(maxsize=None)
def east_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_A
    bb |= empty_bb & (bb >> 1)
    empty_bb &= (empty_bb >> 1)
    bb |= empty_bb & (bb >> 2)
    empty_bb &= (empty_bb >> 2)
    bb |= empty_bb & (bb >> 4)
    return bb


@lru_cache(maxsize=None)
def so_ea_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_A
    bb |= empty_bb & (bb >> 9)
    empty_bb &= (empty_bb >> 9)
    bb |= empty_bb & (bb >> 18)
    empty_bb &= (empty_bb >> 18)
    bb |= empty_bb & (bb >> 36)
    return bb


@lru_cache(maxsize=None)
def no_ea_occluded(u64 bb, u64 empty_bb):
    empty_bb &= NOT_A
    bb |= empty_bb & (bb << 7)
    empty_bb &= (empty_bb << 7)
    bb |= empty_bb & (bb << 14)
    empty_bb &= (empty_bb << 14)
    bb |= empty_bb & (bb << 28)
    return bb


@lru_cache(maxsize=None)
def south_occluded_attacks(u64 bb, empty: int):
    return move_down(south_occluded(bb, empty))


@lru_cache(maxsize=None)
def north_occluded_attacks(u64 bb, u64 empty):
    return move_up(north_occluded(bb, empty))


@lru_cache(maxsize=None)
def east_occluded_attacks(u64 bb, u64 empty):
    return move_right(east_occluded(bb, empty))


@lru_cache(maxsize=None)
def no_ea_occluded_attacks(u64 bb, empty: int):
    return move_right_up(no_ea_occluded(bb, empty))


@lru_cache(maxsize=None)
def so_ea_occluded_attacks(u64 bb, empty: int):
    return move_right_down(so_ea_occluded(bb, empty))


@lru_cache(maxsize=None)
def west_occluded_attacks(u64 bb,u64  empty):
    return move_left(west_occluded(bb, empty))


@lru_cache(maxsize=None)
def so_we_occluded_attacks(u64 bb, empty: int):
    return move_left_down(so_we_occluded(bb, empty))


@lru_cache(maxsize=None)
def no_we_occluded_attacks(u64 bb, empty: int):
    return move_left_up(no_we_occluded(bb, empty))


@lru_cache(maxsize=None)
def horizontal_vertical_moves(u64 bb,u64 empty_bb):
    cdef u64 acc = 0
    acc |= north_occluded_attacks(bb, empty_bb)
    acc |= east_occluded_attacks(bb, empty_bb)
    acc |= south_occluded_attacks(bb, empty_bb)
    acc |= west_occluded_attacks(bb, empty_bb)
    return acc

@lru_cache(maxsize=None)
def diagonal_moves(u64 bb, u64 empty_bb):
    cdef u64 acc = 0
    acc |= no_ea_occluded_attacks(bb, empty_bb)
    acc |= so_ea_occluded_attacks(bb, empty_bb)
    acc |= so_we_occluded_attacks(bb, empty_bb)
    acc |= no_we_occluded_attacks(bb, empty_bb)
    return acc


@lru_cache(maxsize=None)
def king_moves(u64 bb, u64 friendlies_bb):
    cdef u64 moves_bb = 0
    for moves_func in DIRECTIONS:
        moves_bb |= moves_func(bb)
    return moves_bb & ~friendlies_bb


@lru_cache(maxsize=None)
def queen_moves(u64 bb, u64 empties_bb, u64 friendlies_bb):
    return rook_moves(bb, empties_bb, friendlies_bb) | bishop_moves(bb, empties_bb, friendlies_bb)


@lru_cache(maxsize=None)
def rook_moves(u64 bb, u64 empties_bb, u64 friendlies_bb):
    return horizontal_vertical_moves(bb, empties_bb) & ~friendlies_bb


@lru_cache(maxsize=None)
def bishop_moves(u64 bb, u64 empties_bb, u64 friendlies_bb):
    return diagonal_moves(bb, empties_bb) & ~friendlies_bb


@lru_cache(maxsize=None)
def knight_moves(u64 bb, u64 friendlies_bb):
    cdef u64 moves_bb = 0
    for move_func in KNIGHT_MOVES:
        moves_bb |= move_func(bb)
    return moves_bb & ~friendlies_bb


@lru_cache(maxsize=None)
def pawn_moves(u64 bb, int active_side, u64 friendlies_bb, u64 enemies_bb):
    cdef u64 first_step
    if active_side:
        first_step = (move_up(bb) & ~enemies_bb) & ~friendlies_bb
        if first_step:
            return ((first_step | move_upx2(bb & R2)) & ~enemies_bb) & ~friendlies_bb
        return 0
    first_step = (move_down(bb) & ~enemies_bb) & ~friendlies_bb
    if first_step:
        return ((move_down(bb) | move_downx2(bb & R7)) & ~enemies_bb) & ~friendlies_bb
    return 0


@lru_cache(maxsize=None)
def pawn_attacks(u64 bb, int active_side, u64 friendlies_bb):
    if active_side:
        return (move_left_up(bb) | move_right_up(bb)) & ~friendlies_bb
    return (move_left_down(bb) | move_right_down(bb)) & ~friendlies_bb


SLIDING_MOVES = [rook_moves, bishop_moves, queen_moves]


def uci_to_Move(uci: str):
    m = re.match('([a-h][1-8])([a-h][1-8])(.)?', uci)

    if m is None:
        print('Invalid uci')
        return

    promotion = m.group(3)
    if promotion is not None and promotion not in PROMOTION_OPTIONS_W + PROMOTION_OPTIONS_B:
        print('Invalid promotion in uci')
        return
    origin_square_bb = set_bit_on_bb(0, ALGEBRAIC_TO_INDEX[m.group(1)], 1)
    target_square_bb = set_bit_on_bb(0, ALGEBRAIC_TO_INDEX[m.group(2)], 1)
    return Move(origin_square_bb, target_square_bb, promotion)
