#include "moveHelper.h"
#include <vector>
#include <algorithm>
#include <regex>
#include <string>


inline int bitScanForward(BB bb) {
  /* OLD (NOT C++ Way) */
  // """
  //   bitScanForward
  //   @author Kim Walisch (2012)
  //   @param bb bitboard to scan
  //   @precondition bb != 0
  //   @return index (0..63) of least significant one bit
  // """
  return DEBRUIJN_INDEX[((bb ^ (bb-1)) * DEBRUIJN) >> 58];
//   unsigned long index;
//   _BitScanForward64(&index, bb);
//   return index;
}

inline int pop_lsb(BB& bb) {
  const int index = bitScanForward(bb);
  bb &= bb - 1;
  return index;
}

inline BB pop_last_bb(BB bb) {
    bb &= bb - 1;
    return bb;
}

std::vector<BB> get_lsb_bb_array(BB bb) {
    std::vector<BB> bbs;
    BB new_bb;
    while (bb) {
        int index = pop_lsb(bb);
        new_bb = 1 << index;
        bbs.push_back(new_bb);
    }
    return bbs;
}

std::vector<int> get_lsb_array(BB bb) {
    std::vector<int> indexes;
    while (bb) {
        int index = pop_lsb(bb);
        indexes.push_back(index);
    }
    return indexes;
}



// """
// following functions are helper functions
// they take a bitboard (bb)
// the bb must only contain one piece
// the piece (bit) is then shifted
// if a shift results in a piece wrapping the board (going from line A to H) it gets returned as 0
// """

inline BB move(BB bb, Direction dir) {
    switch(dir) {
        case LEFT:
            return (bb << 1 & ~H) & FULL;
            break;
        case RIGHT:
            return bb >> 1 & ~A;
            break;
        case UP:
            return bb << 8 & FULL;
            break;
        case DOWN:
            return bb >> 8;
            break;
        case LEFT_UP:
            return (bb << 9 & ~H) & FULL;
            break;
        case LEFT_DOWN:
            return bb >> 7 & ~H;
            break;
        case RIGHT_UP:
            return (bb << 7 & ~A) & FULL;
            break;
        case RIGHT_DOWN:
            return bb >> 9 & ~A;
            break;
    }
    return 0;
}

/* Following Board Filling Functions are using the Kogge-Stone Algorithm */

inline BB north_occluded_attacks(BB bb, BB empty_bb) {
    bb |= empty_bb & (bb << 8);
    empty_bb &= (empty_bb << 8);
    bb |= empty_bb & (bb << 16);
    empty_bb &= (empty_bb << 16);
    bb |= empty_bb & (bb << 32);
    return bb << 8 & FULL;
}

inline BB east_occluded_attacks(BB bb, BB empty_bb) {
    empty_bb &= NOT_A;
    bb |= empty_bb & (bb >> 1);
    empty_bb &= (empty_bb >> 1);
    bb |= empty_bb & (bb >> 2);
    empty_bb &= (empty_bb >> 2);
    bb |= empty_bb & (bb >> 4);
    return bb >> 1 & ~A;
}

inline BB south_occluded_attacks(BB bb, BB empty_bb) {
    bb |= empty_bb & (bb >> 8);
    empty_bb &= (empty_bb >> 8);
    bb |= empty_bb & (bb >> 16);
    empty_bb &= (empty_bb >> 16);
    bb |= empty_bb & (bb >> 32);
    return bb >> 8;
}

inline BB west_occluded_attacks(BB bb,BB  empty_bb) {
    empty_bb &= NOT_H;
    bb |= empty_bb & (bb << 1);
    empty_bb &= (empty_bb << 1);
    bb |= empty_bb & (bb << 2);
    empty_bb &= (empty_bb << 2);
    bb |= empty_bb & (bb << 4);
    return (bb << 1 & ~H) & FULL;
}

inline BB no_we_occluded_attacks(BB bb, BB empty_bb) {
    empty_bb &= NOT_H;
    bb |= empty_bb & (bb << 9);
    empty_bb &= (empty_bb << 9);
    bb |= empty_bb & (bb << 18);
    empty_bb &= (empty_bb << 18);
    bb |= empty_bb & (bb << 36);
    return (bb << 9 & ~H) & FULL;
}

inline BB so_we_occluded_attacks(BB bb, BB empty_bb) {
    empty_bb &= NOT_H;
    bb |= empty_bb & (bb >> 7);
    empty_bb &= (empty_bb >> 7);
    bb |= empty_bb & (bb >> 14);
    empty_bb &= (empty_bb >> 14);
    bb |= empty_bb & (bb >> 28);
    return bb >> 7 & ~H;
}

inline BB so_ea_occluded_attacks(BB bb, BB empty_bb) {
    empty_bb &= NOT_A;
    bb |= empty_bb & (bb >> 9);
    empty_bb &= (empty_bb >> 9);
    bb |= empty_bb & (bb >> 18);
    empty_bb &= (empty_bb >> 18);
    bb |= empty_bb & (bb >> 36);
    return bb >> 9 & ~A;
}

inline BB no_ea_occluded_attacks(BB bb, BB empty_bb) {
    empty_bb &= NOT_A;
    bb |= empty_bb & (bb << 7);
    empty_bb &= (empty_bb << 7);
    bb |= empty_bb & (bb << 14);
    empty_bb &= (empty_bb << 14);
    bb |= empty_bb & (bb << 28);
    return (bb << 7 & ~A) & FULL;
}

inline BB horizontal_vertical_moves(BB bb,BB empty_bb) {
    BB acc = 0;
    acc |= north_occluded_attacks(bb, empty_bb);
    acc |= east_occluded_attacks(bb, empty_bb);
    acc |= south_occluded_attacks(bb, empty_bb);
    acc |= west_occluded_attacks(bb, empty_bb);
    return acc;
}

inline BB diagonal_moves(BB bb, BB empty_bb) {
    BB acc = 0;
    acc |= no_ea_occluded_attacks(bb, empty_bb);
    acc |= so_ea_occluded_attacks(bb, empty_bb);
    acc |= so_we_occluded_attacks(bb, empty_bb);
    acc |= no_we_occluded_attacks(bb, empty_bb);
    return acc;
}

BB traverse_bb(BB bb, Direction directions[], BB friendlies_bb, BB enemies_bb) {
    BB result_bb = 0;
    for(int i = 0; i <= sizeof(directions); i++) {
        BB square_bb = bb;
        while (1) {
            square_bb = move(square_bb, directions[i]);
            if (square_bb & friendlies_bb)
                break;
            result_bb |= square_bb;
            if (square_bb & enemies_bb || square_bb == 0)
                break;
        }
    }
    return result_bb;
}

BB rook_moves(BB bb, BB empties_bb, BB friendlies_bb) {
    return horizontal_vertical_moves(bb, empties_bb) & ~friendlies_bb;
}

BB bishop_moves(BB bb, BB empties_bb, BB friendlies_bb) {
    return diagonal_moves(bb, empties_bb) & ~friendlies_bb;
}

BB queen_moves(BB bb, BB empties_bb, BB friendlies_bb) {
    return rook_moves(bb, empties_bb, friendlies_bb) | bishop_moves(bb, empties_bb, friendlies_bb);
}

BB king_moves(BB bb, BB friendlies_bb) {
    BB moves_bb = BB(0);
    for (Direction& dir : DIRECTION_MOVES)
        moves_bb |= move(bb, dir);
    return moves_bb & ~friendlies_bb;
}

BB knight_moves(BB bb, BB friendlies_bb) {
    BB moves_bb = 0;
    for (auto& moves : KNIGHT_MOVES) {
        BB moveAcc = BB(0);
        for (Direction& dir : moves)
            moveAcc |= move(bb, dir);
        moves_bb |= moveAcc;
    }
    return moves_bb & ~friendlies_bb;
}

BB pawn_attacks(BB bb, int active_side, BB friendlies_bb) {
    if (active_side)
        return (move(bb, LEFT_UP) | move(bb, RIGHT_UP)) & ~friendlies_bb;
    return (move(bb, LEFT_DOWN) | move(bb, RIGHT_DOWN)) & ~friendlies_bb;
}

BB in_between(int origin, int target) {
   return REY_BBS[origin][target];
}

BB may_move(int origin, int target, BB occupied_bb) {
   return !(in_between(origin, target) & occupied_bb);
}

// TODO: Finish
// Move uci_to_Move(std::string uci) {
//     auto regex = std::regex("([a-h][1-8])([a-h][1-8])(.)?"); 
//     auto m = std::regex_match(regex, uci);

//     if (m == null) {
//         print('Invalid uci');
//         return;
//     }

//     promotion = m.group(3);
//     if promotion is not None and promotion not in PROMOTION_OPTIONS_W + PROMOTION_OPTIONS_B:
//         print('Invalid promotion in uci');
//         return;
//     origin_square = ALGEBRAIC_TO_INDEX[m.group(1)];
//     target_square = ALGEBRAIC_TO_INDEX[m.group(2)];
//     return Move(origin_square, target_square, prmtn=promotion);
// }
