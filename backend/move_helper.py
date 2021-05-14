#from functools import cache
from constants import *
from move import *
from functools import cache,lru_cache
import re


@lru_cache(maxsize=None)
def set_bit_on_bb(bb: int, index: int, value):
    bb_mask = 1 << index
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
def move_left(bb: int):
    return (bb << 1 & ~H) & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_right(bb: int):
    return bb >> 1 & ~A

@lru_cache(maxsize=None)
def move_up(bb: int):
    return bb << 8 & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_down(bb: int):
    return bb >> 8


# castle optimizations
@lru_cache(maxsize=None)
def move_leftx2(bb: int):
    return ((bb << 2 & ~H) & ~G) & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_rightx2(bb: int):
    return ((bb >> 2 & ~A) & ~ B)


# pawn optimizations
@lru_cache(maxsize=None)
def move_downx2(bb: int):
    return bb >> 16

@lru_cache(maxsize=None)
def move_upx2(bb: int):
    return bb << 16 & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_left_up(bb: int):
    return (bb << 9 & ~H) & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_left_down(bb: int):
    return bb >> 7 & ~H

@lru_cache(maxsize=None)
def move_right_up(bb: int):
    return (bb << 7 & ~A) & FULL_BB_MASK

@lru_cache(maxsize=None)
def move_right_down(bb: int):
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

def traverse_bb(bb: int, directions, friendlies_bb: int, enemies_bb: int):
    result_bb = 0
    for move_func in directions:
        square_bb = bb
        while 1:
            square_bb = move_func(square_bb)
            if square_bb & friendlies_bb:
                break
            result_bb |= square_bb
            if square_bb & enemies_bb or square_bb == 0:
                break
    return result_bb

@lru_cache(maxsize=None)
def king_moves(bb: int, friendlies_bb: int):
    moves_bb = 0
    for moves_func in DIRECTIONS:
        moves_bb |= moves_func(bb)
    return moves_bb & ~friendlies_bb

@lru_cache(maxsize=None)
def queen_moves(bb: int, friendlies_bb: int, enemies_bb: int):
    return traverse_bb(bb, DIRECTIONS, friendlies_bb, enemies_bb)

@lru_cache(maxsize=None)
def rook_moves(bb: int, friendlies_bb: int, enemies_bb: int):
    return traverse_bb(bb, HORIZONTAL_VERTICAL_MOVES, friendlies_bb, enemies_bb)

@lru_cache(maxsize=None)
def bishop_moves(bb: int, friendlies_bb: int, enemies_bb: int):
    return traverse_bb(bb, DIAGONALS_MOVES, friendlies_bb, enemies_bb)

@lru_cache(maxsize=None)
def knight_moves(bb: int, friendlies_bb: int):
    moves_bb = 0
    for move_func in KNIGHT_MOVES:
        moves_bb |= move_func(bb)
    return moves_bb & ~friendlies_bb

@lru_cache(maxsize=None)
def pawn_moves(bb: int, active_side: int, friendlies_bb: int, enemies_bb: int):
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
def pawn_attacks(bb: int, active_side: int, friendlies_bb: int):
    if active_side:
        return (move_left_up(bb) | move_right_up(bb)) & ~friendlies_bb
    return (move_left_down(bb) | move_right_down(bb)) & ~friendlies_bb


SLIDING_MOVES = [rook_moves, bishop_moves, queen_moves]


def uci_to_Move(uci: str):
    uci = uci.lower()
    m = re.match('([a-h][1-8])([a-h][1-8])(.)?', uci)

    if m is None:
        print('Invalid uci')
        return

    promotion = m.group(3)
    if promotion is not None and promotion not in PROMOTION_OPTIONS:
        print('Invalid Promotion in uci')
        return
    origin_square_bb = set_bit_on_bb(0, ALGEBRAIC_TO_INDEX[m.group(1)], 1)
    target_square_bb = set_bit_on_bb(0, ALGEBRAIC_TO_INDEX[m.group(2)], 1)
    return Move(origin_square_bb, target_square_bb, bool(promotion))
