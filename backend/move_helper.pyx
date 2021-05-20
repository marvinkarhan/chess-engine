from constants cimport *
from move cimport *
from functools import lru_cache, cache
import re

@lru_cache(maxsize=None)
def bitScanForward(u64 bb):
  """
    bitScanForward
    @author Kim Walisch (2012)
    @param bb bitboard to scan
    @precondition bb != 0
    @return index (0..63) of least significant one bit
  """
  return debruijn64_index64[((bb ^ (bb-1)) * debruijn64) >> 58]


@lru_cache(maxsize=None)
def get_lsb_bb_array(u64 bb):
    cdef list bbs = []
    cdef u64 new_bb
    while bb:
        index = bitScanForward(bb)
        new_bb = 1 << index
        bbs.append(new_bb)
        bb &= bb - 1
    return bbs

def get_lsb_array(u64 bb):
    cdef list array = []
    while bb:
        index = bitScanForward(bb)
        array.append(index)
        bb &= bb - 1
    return array

def pop_last_bb(u64 bb):
    bb &= bb - 1
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
HORIZONTAL_MOVES = [move_left, move_right]
VERTICAL_MOVES = [move_up, move_down]
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
def king_moves(u64 bb, u64 friendlies_bb):
    cdef u64 moves_bb = 0
    for moves_func in DIRECTIONS:
        moves_bb |= moves_func(bb)
    return moves_bb & ~friendlies_bb


@lru_cache(maxsize=None)
def queen_moves(u64 bb, u64 friendlies_bb, u64 enemies_bb):
    return traverse_bb(bb, DIRECTIONS, friendlies_bb, enemies_bb)


@lru_cache(maxsize=None)
def rook_moves(u64 bb, u64 friendlies_bb, u64 enemies_bb):
    return traverse_bb(bb, HORIZONTAL_VERTICAL_MOVES, friendlies_bb, enemies_bb)


@lru_cache(maxsize=None)
def bishop_moves(u64 bb, u64 friendlies_bb, u64 enemies_bb):
    return traverse_bb(bb, DIAGONALS_MOVES, friendlies_bb, enemies_bb)


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

cdef u64 in_between(u64 origin_bb, u64 target_bb):
   return REY_BBS[origin_bb][target_bb]


cpdef u64 may_move(u64 origin_bb, u64 target_bb, u64 occupied_bb):
   return not in_between(origin_bb, target_bb) & occupied_bb


def blockers(int square, bint active_side, u64 rook_bb, u64 knight_bb, u64 bishop_bb, u64 queen_bb, u64 king_bb, u64 pawn_bb, u64 friendlies_bb):
    cdef u64 sliding_attackers_bb, blockers_bb, sliding_attacker_bb, blocker_bb, square_bb
    square_bb = SQUARE_BBS[square]
    # sliding pieces
    sliding_attackers_bb = 0
    sliding_attackers_bb |= DIAGONALS_MOVE_BBS[square] & (bishop_bb | queen_bb)
    sliding_attackers_bb |= HORIZONTAL_VERTICAL_MOVE_BBS[square] & (rook_bb | queen_bb)

    blockers_bb = 0
    for sliding_attacker_bb in get_lsb_array(sliding_attackers_bb):
        blocker_bb = in_between(square_bb, sliding_attacker_bb) & friendlies_bb
        # is only one piece
        if blocker_bb and SQUARE_BBS[bitScanForward(blocker_bb)] == blocker_bb:
            blockers_bb |= blocker_bb

    return blockers_bb


def uci_to_Move(uci: str):
    m = re.match('([a-h][1-8])([a-h][1-8])(.)?', uci)

    if m is None:
        print('Invalid uci')
        return

    promotion = m.group(3)
    if promotion is not None and promotion not in PROMOTION_OPTIONS_W + PROMOTION_OPTIONS_B:
        print('Invalid promotion in uci')
        return
    origin_square = ALGEBRAIC_TO_INDEX[m.group(1)]
    target_square = ALGEBRAIC_TO_INDEX[m.group(2)]
    return Move(origin_square, target_square, prmtn=promotion)
