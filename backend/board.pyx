# cython: profile=True
from random import choice, random
from math import pi, trunc
from re import M
from constants cimport *
from move_helper import *
from move cimport *
import time
import cProfile, pstats
from functools import lru_cache

def get_zobrist():
    return ZOBRIST_TABLE


cdef u64 hash_board(Board board):
    # """"
    #     The hash indizes are defined as following:
    #     The max size of the array is 781
    #     It's always (PRNBQK) for all 64 fields,
    #     The Field ar defined like this A1, A2, B1, ..., F8
    #     It Starts with White followed by black, meaning the first 6 * 64 are defined like this
    #     [WP * A1, ..., WK * F8] for the first 6 * 64 - 1 indizes = 384 - 1 = 383 [0,383]
    #     followed by black:
    #     [BP * A1, ..., BK * F8] for the first 383 + 6 * 64 = 383 + 384 = 767 [384,767]
    #     After that we process if black is to move this is the key: 767 + 1 = 768
    #     Then the 4 castling rights in this order (WK, WQ, BK, BQ) = [769,772]
    #     The last 8 are the en passant squares fields in this order (A, B, C, D, E, F, G, H) = [773,780]
    # """"
    cdef int offset, index
    cdef list lsb_array
    cdef u64 final_hash_key, hash_value
    offset = 0
    final_hash_key = 0
    hash_value = 0
    for piece, bb in board.pieces.items():
        lsb_array = get_lsb_array(bb)
        for position in lsb_array:
            index = 63 - position # index from behind
            hash_value = ZOBRIST_TABLE[offset+index]
            final_hash_key ^= hash_value
        offset += 64 # Every piece is a shift to 64 indizes in the array

    if not board.active_side:
        final_hash_key ^= ZOBRIST_TABLE[768]
    if board.castle_w_king_side:
        final_hash_key ^= ZOBRIST_TABLE[769]
    if board.castle_w_queen_side:
        final_hash_key ^= ZOBRIST_TABLE[770]
    if board.castle_b_king_side:
        final_hash_key ^= ZOBRIST_TABLE[771]
    if board.castle_b_queen_side:
        final_hash_key ^= ZOBRIST_TABLE[772]   
    if board.ep_square_bb & A:
        final_hash_key ^= ZOBRIST_TABLE[773]
    elif board.ep_square_bb & B:
        final_hash_key ^= ZOBRIST_TABLE[774]
    elif board.ep_square_bb & C:
        final_hash_key ^= ZOBRIST_TABLE[775]
    elif board.ep_square_bb & D:
        final_hash_key ^= ZOBRIST_TABLE[776]   
    elif board.ep_square_bb & E:
        final_hash_key ^= ZOBRIST_TABLE[777]
    elif board.ep_square_bb & F:
        final_hash_key ^= ZOBRIST_TABLE[778]
    elif board.ep_square_bb & G:
        final_hash_key ^= ZOBRIST_TABLE[779]      
    elif board.ep_square_bb & H:
        final_hash_key ^= ZOBRIST_TABLE[780]   

    return final_hash_key


cdef class Board:
    cdef:
        dict pieces, current_opening_table
        bint castle_w_king_side, castle_w_queen_side, castle_b_king_side, castle_b_queen_side, active_side, opening_finished
        u64 friendlies_bb, enemies_bb, ep_square_bb
        int full_moves, half_moves, opening_moves


    def __init__(self, fen=None) -> None:
        self.pieces = {
            'P': 0,
            'R': 0,
            'N': 0,
            'B': 0,
            'Q': 0,
            'K': 0,
            'p': 0,
            'r': 0,
            'n': 0,
            'b': 0,
            'q': 0,
            'k': 0
        }

        self.castle_w_king_side = True
        self.castle_w_queen_side = True
        self.castle_b_king_side = True
        self.castle_b_queen_side = True
        self.friendlies_bb = 0
        self.enemies_bb = 0
        

        self.current_opening_table = OPENING_TABLE
        self.opening_moves = 10
        self.opening_finished = False


        # 0: black, 1: white
        self.active_side = 1
        self.full_moves = 0
        self.half_moves = 0

        self.ep_square_bb = 0
        fen = fen if fen else START_POS_FEN
        self.parse_FEN_string(fen)

    def reset_board(self):
        self.pieces = {
            'P': 0,
            'R': 0,
            'N': 0,
            'B': 0,
            'Q': 0,
            'K': 0,
            'p': 0,
            'r': 0,
            'n': 0,
            'b': 0,
            'q': 0,
            'k': 0
        }
        self.castle_w_king_side = True
        self.castle_w_queen_side = True
        self.castle_b_king_side = True
        self.castle_b_queen_side = True
        self.friendlies_bb = 0
        self.enemies_bb = 0
        self.active_side = 1
        self.full_moves = 0
        self.half_moves = 0
        self.ep_square_bb = 0

    def print_bitboard(self, u64 bb):
        print('\n'.join(['{0:064b}'.format(bb)[i:i + 8]
              for i in range(0, 64, 8)]))


    cpdef list process_next_move(self, int depth, last_move: str):
        if self.full_moves * 2 < self.opening_moves and last_move in self.current_opening_table and not self.opening_finished:
            black_key = choice(list(self.current_opening_table[last_move].keys()))
            print('book move: ', black_key)
            result =  [[uci_to_Move(black_key)],0]
            self.current_opening_table = self.current_opening_table[last_move][black_key]
            return result
        else:
            self.opening_finished = True
            return self.nega_max(depth, -20000000, 20000000)

    cpdef list nega_max(self, int depth, int alpha, int beta):
        cdef int score
        cdef dict backup
        cdef list best_moves, moves
        best_moves = []
        if(depth == 0):
            return [[],self.evaluate()]
        for move in self.pseudo_legal_moves_generator(self.active_side):
            backup = self.store()    
            if self.make_move(move):
                [moves, score] = self.nega_max(depth-1,-beta,-alpha)
                score = -score
                self.restore(backup)
                if score >= beta:
                    return [best_moves, beta]
                if score > alpha:
                    alpha = score
                    moves.append(move)
                    best_moves = moves
        return [best_moves, alpha]

    cpdef int evaluate(self):
        cdef u64 amount, position, hash_key
        cdef int side_to_move, score, moves_white, moves_black
        cdef list lsb_array
        hash_key = hash_board(self)
        if hash_key in EVALUATE_TABLE:
            return EVALUATE_TABLE[hash_key]
        side_to_move = 1 if self.active_side else -1
        score = 0
        for piece, amount in self.pieces.items():
            lsb_array = get_lsb_array(amount)
            score += PIECE_VALUES[piece] * len(lsb_array)
            for position in lsb_array:
                index = 63 - position # index from behind
                #print(index, piece, PIECE_SQUARE_TABLES[piece][index] )

                score += PIECE_SQUARE_TABLES[piece][index] * (1 if piece.isupper() else -1)

        #moves_white = len(list(self.pseudo_legal_moves_generator(1)))
        #moves_black = len(list(self.pseudo_legal_moves_generator(0)))
        score += 10 #* (moves_white - moves_black)
        EVALUATE_TABLE[hash_key] = score * side_to_move
        return score * side_to_move

    def to_fen_string(self):
        fen = ''
        bb = 1 << 63
        for row in range(0, 8, 1):
            counter = 0
            for x in range(0, 8, 1):
                piece = self.get_piece_on_square(bb)
                if piece:
                    if counter > 0:
                        fen += str(counter)
                        counter = 0
                    fen += piece
                else:
                    counter += 1
                bb >>= 1
            if counter > 0:
                fen += str(counter)
            if(row < 7):
                fen += "/"

        fen += ' w ' if self.active_side else ' b '
        castle_rights = ''
        castle_rights += 'K' if self.castle_w_king_side else ''
        castle_rights += 'Q' if self.castle_w_queen_side else ''
        castle_rights += 'k' if self.castle_w_king_side else ''
        castle_rights += 'q' if self.castle_w_queen_side else ''
        if castle_rights == '':
            castle_rights += '-'
        fen += castle_rights + ' '
        keys = list(ALGEBRAIC_TO_INDEX.keys())
        if(self.ep_square_bb == 0):
            fen += '- '
        else:
            fen += keys[-self.ep_square_bb.bit_length()] + ' '
        fen += str(self.half_moves) + ' '
        fen += str(self.full_moves)
        return fen

    def get_active_side(self):
        return self.active_side

    def print_every_piece(self):
        for k, v in self.pieces.items():
            print(k + ':')
            self.print_bitboard(v)
            print()

    cdef u64 w_pieces_bb(self):
        cdef u64 bb = 0
        for p_bb in self.get_active_pieces(1):
                bb |= p_bb
        return bb

    cdef u64 b_pieces_bb(self):
        cdef u64 bb = 0
        for p_bb in self.get_active_pieces(0):
                bb |= p_bb
        return bb

    cpdef u64 all_pieces_bb(self):
        return self.w_pieces_bb() | self.b_pieces_bb()

    cdef list get_active_pieces(self, bint active_side): 
        if active_side:
            return [self.pieces['P'], self.pieces['R'],self.pieces['N'], self.pieces['B'], self.pieces['Q'], self.pieces['K']]
        else:
            return [self.pieces['p'], self.pieces['r'],self.pieces['n'], self.pieces['b'], self.pieces['q'], self.pieces['k']]

    cdef get_piece_on_square(self, u64 bb):
        cdef u64 v
        for k, v in self.pieces.items():
            if bb & v:
                return k
        return None

    def parse_FEN_string(self, fen: str):
        self.reset_board()
        # for example: 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
        placement, active_side, castling_ability, ep_square, half_moves, full_moves = fen.split(
            ' ')
        self.full_moves = int(full_moves)
        self.half_moves = int(half_moves)
        self.active_side = 1 if active_side == 'w' else 0
        self.castle_w_king_side = 'K' in castling_ability
        self.castle_w_queen_side = 'Q' in castling_ability
        self.castle_b_king_side = 'k' in castling_ability
        self.castle_b_queen_side = 'q' in castling_ability
        if ep_square in ALGEBRAIC_TO_INDEX:
            self.ep_square_bb = 1 << ALGEBRAIC_TO_INDEX[ep_square]

        ranks = placement.split('/')
        # fen starts notation from the top left
        bb_placement_mask = 1 << 63
        for rank in ranks:
            for char in rank:
                if char.isdigit():
                    bb_placement_mask >>= int(char)
                else:
                    self.pieces[char] |= bb_placement_mask
                    bb_placement_mask >>= 1
        self.friendlies_bb = self.w_pieces_bb() if self.active_side else self.b_pieces_bb()
        self.enemies_bb = self.b_pieces_bb() if self.active_side else self.w_pieces_bb()

        
    cdef u64 attackers(self, int square, bint active_side):
        cdef u64 pot_attackers_bb, attackers_bb, move_bb, pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb, occupied_bb
        
        occupied_bb = self.enemies_bb | self.friendlies_bb
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(not active_side)
        pot_attackers_bb = 0
        pot_attackers_bb |= DIAGONALS_MOVE_BBS[square] & (bishop_bb | queen_bb)
        pot_attackers_bb |= HORIZONTAL_VERTICAL_MOVE_BBS[square] & (rook_bb | queen_bb)
        pot_attackers_bb |= KNIGHT_MOVE_BBS[square] & knight_bb
        pot_attackers_bb |= PAWN_ATTACKS_BBS[square][not active_side] & pawn_bb
        pot_attackers_bb |= KING_MOVES_BBS[square] & king_bb

        attackers_bb = 0
        for move_square in get_lsb_array(pot_attackers_bb):
            if may_move(move_square, square, occupied_bb):
                attackers_bb |= SQUARE_BBS[move_square]
        return attackers_bb


    def pseudo_legal_moves_generator(self, bint active_side):
        cdef u64 pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb, king_attackers_bb, friendlies_bb, not_friendlies_bb, enemies_bb, occupied_bb, empty_bb, R2_or_R7, R3_or_R6, promotion_rank, bb, pawn_on_promotion_rank
        cdef int king_square, direction, rook, bishop, queen, knight, move, ep_square
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(active_side)
        king_square = bitScanForward(king_bb)
        king_attackers_bb = self.attackers(king_square, active_side)
        friendlies_bb = self.enemies_bb if active_side != self.active_side else self.friendlies_bb
        not_friendlies_bb = ~friendlies_bb
        enemies_bb = self.friendlies_bb if active_side != self.active_side else self.enemies_bb
        occupied_bb = friendlies_bb | enemies_bb
        empty_bb = ~occupied_bb
        # more than one king attacker = only king moves possible
        if not pop_last_bb(king_attackers_bb):
            # pawn moves
            # generate vars to hande pawn colors easier
            R2_or_R7 = R2 if active_side else R7
            R3_or_R6 = R3 if active_side else R6
            promotion_rank = R7 if active_side else R2
            direction = 1 if active_side else -1
            move_up_f = move_up if active_side else move_down
            move_left_up_f = move_left_up if active_side else move_right_down
            move_right_up_f = move_right_up if active_side else move_left_down
            # normal moves
            # pawns that can move one
            pawn_not_on_promotion_rank = pawn_bb & ~promotion_rank
            bb = move_up_f(pawn_not_on_promotion_rank) & empty_bb
            for move in get_lsb_array(bb):
                yield Move(move - 8 * direction, move)
            # pawns that can move two
            bb = move_up_f(R3_or_R6 & bb) & empty_bb
            for move in get_lsb_array(bb):
                yield Move(move - (8 << 1) * direction, move)
            # promotions by move and by capture
            pawn_on_promotion_rank = pawn_bb & promotion_rank
            if pawn_on_promotion_rank:
                promotion_options = PROMOTION_OPTIONS_W if active_side else PROMOTION_OPTIONS_B
                bb = move_up_f(pawn_on_promotion_rank) & enemies_bb
                for move in get_lsb_array(bb):
                    for promotion in promotion_options:
                        yield Move(move - 8 * direction, move, move_type.PROMOTION, promtion)
                bb = move_left_up_f(pawn_on_promotion_rank) & enemies_bb
                for move in get_lsb_array(bb):
                    for promotion in promotion_options:
                        yield Move(move - 7 * direction, move, move_type.PROMOTION, promtion)
                bb = move_right_up_f(pawn_on_promotion_rank) & enemies_bb
                for move in get_lsb_array(bb):
                    for promotion in promotion_options:
                        yield Move(move - 9 * direction, move, move_type.PROMOTION, promtion)
            # captures
            bb = move_left_up_f(pawn_not_on_promotion_rank) & enemies_bb
            for move in get_lsb_array(bb):
                yield Move(move - 9 * direction, move)
            bb = move_right_up_f(pawn_not_on_promotion_rank) & enemies_bb
            for move in get_lsb_array(bb):
                yield Move(move - 7 * direction, move)
            # en passant
            if self.ep_square_bb:
                ep_square = bitScanForward(self.ep_square_bb)
                bb = pawn_not_on_promotion_rank & PAWN_ATTACKS_BBS[ep_square][active_side]
                for move in get_lsb_array(bb):
                    yield Move(move, ep_square, move_type.EN_PASSANT)
            # rook moves
            for rook in get_lsb_array(rook_bb):
                for move in get_lsb_array(rook_moves(SQUARE_BBS[rook], friendlies_bb, enemies_bb)):
                    yield Move(rook, move)

            # bishop moves
            for bishop in get_lsb_array(bishop_bb):
                for move in get_lsb_array(bishop_moves(SQUARE_BBS[bishop], friendlies_bb, enemies_bb)):
                    yield Move(bishop, move)

            # queen moves
            for queen in get_lsb_array(queen_bb):
                for move in get_lsb_array(queen_moves(SQUARE_BBS[queen], friendlies_bb, enemies_bb)):
                    yield Move(queen, move)

            # king moves (there is always only one king)
            for knight in get_lsb_array(knight_bb):
                for move in get_lsb_array(KNIGHT_MOVE_BBS[knight] & not_friendlies_bb):
                    yield Move(knight, move)
        # king moves
        bb = KING_MOVES_BBS[king_square] & ~friendlies_bb
        for move in get_lsb_array(bb):
            yield Move(king_square, move, move_type.NORMAL)
        # castle
        if active_side:
            # check if sth is in the way, dont check if is legal to castle
            if self.castle_w_king_side and not (CASTLING_W_KING_SIDE_WAY & occupied_bb):
                yield Move(king_square, CASTLING_W_KING_SIDE_SQUARE, move_type.CASTLING)
            if self.castle_w_queen_side and not (CASTLING_W_QUEEN_SIDE_WAY & occupied_bb):
                yield Move(king_square, CASTLING_W_QUEEN_SIDE_SQUARE, move_type.CASTLING)
        else:
            if self.castle_b_king_side and not (CASTLING_B_KING_SIDE_WAY & occupied_bb):
                yield Move(king_square, CASTLING_B_KING_SIDE_SQUARE, move_type.CASTLING)
            if self.castle_b_queen_side and not (CASTLING_B_QUEEN_SIDE_WAY & occupied_bb):
                yield Move(king_square, CASTLING_B_QUEEN_SIDE_SQUARE, move_type.CASTLING)


    def legal_moves_generator(self, active_side = None):
        cdef dict previous_board_state
        if active_side is None:
            active_side = self.active_side
        for move in self.pseudo_legal_moves_generator(active_side):
            previous_board_state = self.store()
            if self.make_move(move):
                self.restore(previous_board_state)
                yield move

    # new move generation based on king blockers and attackers (WIP)
    # def new_legal_moves_generator(self):
    #     cdef u64 king_bb, blockers_bb, attackers_bb
    #     cdef int king_square
        
    #     pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(
    #         not self.active_side)
    #     true_king_bb = self.pieces['K' if self.active_side else 'k']
    #     king_square = bitScanForward(true_king_bb)
    #     blockers_bb = blockers(king_square, self.active_side, king_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb, pawn_bb, self.friendlies_bb)
    #     attackers_bb = attackers(king_square, 0, self.friendlies_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb, pawn_bb)
    #     self.print_bitboard(attackers_bb)
    #     if attackers_bb:
    #         for move in self._generate_evasions(king, attackers_bb, from_mask, to_mask):
    #             if self._is_safe(king, blockers, move):
    #                 yield move
    #     else:
    #         for move in self.pseudo_legal_moves_generator():
    #             if self._is_safe(king, blockers, move):
    #                 yield move
    #     pass


    def stalemate(self):
        king_attackers_bb = self.attackers(bitScanForward(self.pieces['K' if self.active_side else 'k']), self.active_side)
        has_legal_moves = len(list(self.legal_moves_generator()))
        return bool(not has_legal_moves and not king_attackers_bb)

    def checkmate(self):
        king_attackers_bb = self.attackers(bitScanForward(self.pieces['K' if self.active_side else 'k']), self.active_side)
        has_legal_moves = len(list(self.legal_moves_generator()))
        return bool(not has_legal_moves and king_attackers_bb)

    def get_moves_tree(self, depth: int):
        move_tree = {}
        if depth >= 1:
            legal_moves = list(self.legal_moves_generator())
            for move in legal_moves:
                cur_state = self.store()
                self.make_move(move)
                move_tree[move] = self.get_moves_tree(depth - 1)
                self.restore(cur_state)
        return move_tree

    cdef dict store(self):
        return {
            'pieces': self.pieces.copy(),
            'castle_w_king_side': self.castle_w_king_side,
            'castle_w_queen_side': self.castle_w_queen_side,
            'castle_b_king_side': self.castle_b_king_side,
            'castle_b_queen_side': self.castle_b_queen_side,
            'friendlies_bb': self.friendlies_bb,
            'enemies_bb': self.enemies_bb,
            'active_side': self.active_side,
            'full_moves': self.full_moves,
            'half_moves': self.half_moves,
            'ep_square_bb': self.ep_square_bb
        }

    cdef restore(self, dict restore_dict):
        self.reset_board()
        self.pieces = restore_dict['pieces']
        self.castle_w_king_side = restore_dict['castle_w_king_side']
        self.castle_w_queen_side = restore_dict['castle_w_queen_side']
        self.castle_b_king_side = restore_dict['castle_b_king_side']
        self.castle_b_queen_side = restore_dict['castle_b_queen_side']
        self.friendlies_bb = restore_dict['friendlies_bb']
        self.enemies_bb = restore_dict['enemies_bb']
        self.active_side = restore_dict['active_side']
        self.full_moves = restore_dict['full_moves']
        self.half_moves = restore_dict['half_moves']
        self.ep_square_bb = restore_dict['ep_square_bb']

    def make_move(self, move: Move):
        cdef u64 captured_pawn_bb, moved_upx2, moved_downx2, rook_square, target_square, origin_square_bb, target_square_bb
        cdef bint capture
        cdef dict stored_board
        stored_board = self.store()
        # track if capture for half_moves
        capture = False
        origin_square_bb = SQUARE_BBS[move.origin_square]
        target_square_bb = SQUARE_BBS[move.target_square]
        origin_piece = self.get_piece_on_square(origin_square_bb)
        target_piece = self.get_piece_on_square(target_square_bb)

        # update bitboards to represent change
        self.pieces[origin_piece] &= ~origin_square_bb
        self.pieces[origin_piece] |= target_square_bb
        # target piece only exists on capture
        if target_piece:
            self.pieces[target_piece] &= ~target_square_bb
            capture = True

        # en passant
        if origin_piece in ['P', 'p']:
            # promotion
            if move.promotion:
                # remove pawn
                self.pieces[origin_piece] &= ~target_square_bb
                # add promoted piece
                self.pieces[move.promotion] |= target_square_bb
            # complete ep move
            if self.ep_square_bb:
                if target_square_bb == self.ep_square_bb:
                    captured_pawn_bb = move_down(
                        self.ep_square_bb) if self.active_side else move_up(self.ep_square_bb)
                    captured_pawn = self.get_piece_on_square(captured_pawn_bb)
                    self.pieces[captured_pawn] &= ~captured_pawn_bb
                    self.enemies_bb &= ~captured_pawn_bb
                self.ep_square_bb = 0
                capture = True
            # check for resulting en passant
            moved_upx2 = origin_square_bb & R2 and target_square_bb & R4
            moved_downx2 = origin_square_bb & R7 and target_square_bb & R5
            if moved_upx2 or moved_downx2:
                left_square_piece = self.get_piece_on_square(
                    move_left(target_square_bb))
                right_square_piece = self.get_piece_on_square(
                    move_right(target_square_bb))
                enemy_pawn_key = 'p' if self.active_side else 'P'
                if left_square_piece == enemy_pawn_key or right_square_piece == enemy_pawn_key:
                    self.ep_square_bb = move_down(target_square_bb & R4) | move_up(
                        target_square_bb & R5)
        else:
            self.ep_square_bb = 0

        # castles
        # check rook moves
        if origin_piece == 'R' or target_piece == 'R':
            if origin_square_bb == (H & R1) or target_square_bb == (H & R1):
                self.castle_w_king_side = False
            elif origin_square_bb == (A & R1) or target_square_bb == (A & R1):
                self.castle_w_queen_side = False
        elif origin_piece == 'r' or target_piece == 'r':
            if origin_square_bb == (H & R8) or target_square_bb == (H & R8):
                self.castle_b_king_side = False
            elif origin_square_bb == (A & R8) or target_square_bb == (A & R8):
                self.castle_b_queen_side = False
        # king moves
        if origin_piece in ['K', 'k']:
            if self.active_side:
                self.castle_w_king_side = False
                self.castle_w_queen_side = False
            else:
                self.castle_b_king_side = False
                self.castle_b_queen_side = False
            # check if king move was castle
            if not (king_moves(origin_square_bb, self.friendlies_bb) & target_square_bb):
                # castle king side
                if target_square_bb & move_rightx2(origin_square_bb):
                    # get rook
                    rook_square = (H & R1) if self.active_side else (H & R8)
                    target_square = move_leftx2(rook_square)
                    rook_piece = self.get_piece_on_square(rook_square)
                    # move rook
                    self.pieces[rook_piece] &= ~rook_square
                    self.pieces[rook_piece] |= target_square
                    self.friendlies_bb &= ~rook_square
                    self.friendlies_bb |= target_square
                # castle queen side
                else:
                    # get rook
                    rook_square = (A & R1) if self.active_side else (A & R8)
                    target_square = move_rightx2(move_right(rook_square))
                    rook_piece = self.get_piece_on_square(rook_square)
                    # move rook
                    self.pieces[rook_piece] &= ~rook_square
                    self.pieces[rook_piece] |= target_square
                    self.friendlies_bb &= ~rook_square
                    self.friendlies_bb |= target_square

        # update board properties
        self.friendlies_bb &= ~origin_square_bb
        self.friendlies_bb |= target_square_bb
        if capture:
            self.half_moves = 0
            self.enemies_bb &= ~target_square_bb
        else:
            self.half_moves += 1
        if self.active_side:
            self.full_moves += 1
        # swap sides
        self.active_side = not self.active_side
        self.friendlies_bb, self.enemies_bb = self.enemies_bb, self.friendlies_bb
        # unmake move if it was illegal
        if self.attackers(bitScanForward(self.pieces['k' if self.active_side else 'K']), not self.active_side):
            self.restore(stored_board)
            return False
        return True
