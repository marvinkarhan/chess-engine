from random import choice, random
from math import pi, trunc
from re import M
from constants cimport *
from move_helper import *
from move cimport *
import time
import cProfile, pstats
from functools import lru_cache



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
def get_lsb_array(u64 bb):
    cdef list bbs = []
    cdef u64 new_bb
    while bb:
        index = bitScanForward(bb)
        new_bb = 1 << index
        bbs.append(new_bb)
        bb &= bb - 1
    return bbs



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
            return self.root_nega_max(depth)
    
    cpdef list root_nega_max(self, int depth):
        cdef int alpha, beta, score
        best_moves = None
        alpha = -20000000
        beta =  20000000
        for move in self.legal_moves_generator():
            backup = self.store() 
            self.make_move(move)
            [moves, score] = self.nega_max(depth-1, -beta,-alpha)
            score = -score 
            self.restore(backup)
            if score >= beta:
                return [move, beta]
            if score > alpha:
                alpha = score
                moves.append(move)
                best_moves = moves
        return [best_moves,alpha]

    cdef list nega_max(self, int depth, int alpha, int beta):
        cdef int score
        cdef dict backup
        cdef list best_moves, moves
        best_moves = []
        if(depth == 0):
            return [[],self.evaluate()]
        for move in self.legal_moves_generator():
            backup = self.store() 
            self.make_move(move)
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
        cdef u64 amount, position
        cdef int side_to_move, score, moves_white, moves_black
        cdef list lsb_array
        side_to_move = 1 if self.active_side else -1
        score = 0
        for piece, amount in self.pieces.items():
            lsb_array = get_lsb_array(amount)
            score += PIECE_VALUES[piece] * len(lsb_array)
            for position in lsb_array:
                index = 63 - bitScanForward(position) # index from behind
                #print(index, piece, PIECE_SQUARE_TABLES[piece][index] )

                score += PIECE_SQUARE_TABLES[piece][index] * (1 if piece.isupper() else -1)

        moves_white = len(list(self.pseudo_legal_moves_generator(1)))
        moves_black = len(list(self.pseudo_legal_moves_generator(0)))
        score += 10 * (moves_white - moves_black)
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

    cdef u64 attacked_squares(self, bint active_side):
        cdef u64 attacked_bb, friendlies_bb, enemies_bb, empty_bb, pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb
        cdef list active_pieces
        attacked_bb = 0

        # swap friendlies and enemies locally
        friendlies_bb = self.enemies_bb if active_side != self.active_side else self.friendlies_bb
        enemies_bb = self.friendlies_bb if active_side != self.active_side else self.enemies_bb
        empty_bb = ~(friendlies_bb | enemies_bb) & FULL_BB_MASK

        active_pieces = self.get_active_pieces(active_side)
        # filter pieces in case they got removed (eg. in legal_moves_generator)
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = [
            x & friendlies_bb for x in active_pieces]

        attacked_bb |= king_moves(king_bb, friendlies_bb)
        attacked_bb |= pawn_attacks(pawn_bb, active_side, friendlies_bb)
        attacked_bb |= knight_moves(knight_bb, friendlies_bb)

        for pieces_bb, move_func in zip([rook_bb, bishop_bb, queen_bb], SLIDING_MOVES):
            for piece_bb in get_lsb_array(pieces_bb):
                attacked_bb |= move_func(piece_bb, friendlies_bb, enemies_bb)
        
        return attacked_bb

    def pseudo_legal_moves_generator(self, bint active_side):
        cdef u64 attacked_squares_bb, empty_bb, pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb, pawn, rook, bishop, queen, knight, way_bb, way_rook_bb, move
        attacked_squares_bb = self.attacked_squares(not active_side)
        empty_bb = ~(self.friendlies_bb | self.enemies_bb) & FULL_BB_MASK
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(
            active_side)
        # pawn moves
        for pawn in get_lsb_array(pawn_bb):
            for move in get_lsb_array(pawn_attacks(pawn, active_side, self.friendlies_bb) & self.enemies_bb | pawn_moves(pawn, active_side, self.friendlies_bb, self.enemies_bb)):
                # check for promotion
                if move & R1:
                    for promotion in PROMOTION_OPTIONS_B:
                        yield Move(pawn, move, promotion)
                elif move & R8:
                    for promotion in PROMOTION_OPTIONS_W:
                        yield Move(pawn, move, promotion)
                else:
                    yield Move(pawn, move)
            # en passant
            if self.ep_square_bb:
                move = pawn_attacks(pawn, active_side,
                                    self.friendlies_bb) & self.ep_square_bb
                if move:
                    yield Move(pawn, move)

        # rook moves
        for rook in get_lsb_array(rook_bb):
            for move in get_lsb_array(rook_moves(rook, self.friendlies_bb, self.enemies_bb)):
                yield Move(rook, move)

        # bishop moves
        for bishop in get_lsb_array(bishop_bb):
            for move in get_lsb_array(bishop_moves(bishop, self.friendlies_bb, self.enemies_bb)):
                yield Move(bishop, move)

        # queen moves
        for queen in get_lsb_array(queen_bb):
            for move in get_lsb_array(queen_moves(queen, self.friendlies_bb, self.enemies_bb)):
                yield Move(queen, move)

        # knight moves
        for knight in get_lsb_array(knight_bb):
            for move in get_lsb_array(knight_moves(knight, self.friendlies_bb)):
                yield Move(knight, move)

        # king moves (there is always only one king)
        for move in get_lsb_array(king_moves(king_bb, self.friendlies_bb) & ~attacked_squares_bb):
            yield Move(king_bb, move)

        # king castle
        if king_bb & ~attacked_squares_bb:
            if active_side and self.castle_w_king_side or not active_side and self.castle_b_king_side:
                way_bb = (move_right(king_bb) | move_rightx2(king_bb))
                # check if there is sth in the way of the king
                if (way_bb & ~self.friendlies_bb & ~attacked_squares_bb & ~self.enemies_bb) == way_bb:
                    yield Move(king_bb, move_rightx2(king_bb))
            if active_side and self.castle_w_queen_side or not active_side and self.castle_b_queen_side:
                way_bb = (move_left(king_bb) | move_leftx2(king_bb))
                way_rook_bb = move_leftx3(king_bb)
                # check if there is sth in the way of the king and if there is sth in the way of the rook
                if (way_bb & ~self.friendlies_bb & ~attacked_squares_bb & ~self.enemies_bb) == way_bb and (way_rook_bb & ~self.friendlies_bb & ~self.enemies_bb) == way_rook_bb:
                    yield Move(king_bb, move_leftx2(king_bb))

    def legal_moves_generator(self, active_side = None):
        cdef dict previous_board_state
        if active_side is None:
            active_side = self.active_side
        for move in self.pseudo_legal_moves_generator(active_side):
            previous_board_state = self.store()
            if self.make_move(move):
                self.restore(previous_board_state)
                yield move

    def stalemate(self):
        is_king_attacked = self.attacked_squares(
            not self.active_side) & self.pieces['K' if self.active_side else 'k']
        has_legal_moves = len(list(self.legal_moves_generator()))
        return bool(not has_legal_moves and not is_king_attacked)

    def checkmate(self):
        is_king_attacked = self.attacked_squares(
            not self.active_side) & self.pieces['K' if self.active_side else 'k']
        has_legal_moves = len(list(self.legal_moves_generator()))
        return bool(not has_legal_moves and is_king_attacked)

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
        cdef u64 captured_pawn_bb, moved_upx2, moved_downx2, rook_square, target_square
        cdef bint capture
        cdef dict stored_board
        stored_board = self.store()
        # track if capture for half_moves
        capture = False
        # print(move)
        origin_piece = self.get_piece_on_square(move.origin_square_bb)
        target_piece = self.get_piece_on_square(move.target_square_bb)

        # update bitboards to represent change
        self.pieces[origin_piece] &= ~move.origin_square_bb
        self.pieces[origin_piece] |= move.target_square_bb
        # target piece only exists on capture
        if target_piece:
            self.pieces[target_piece] &= ~move.target_square_bb
            capture = True

        # en passant
        if origin_piece in ['P', 'p']:
            # promotion
            if move.promotion:
                # remove pawn
                self.pieces[origin_piece] &= ~move.target_square_bb
                # add promoted piece
                self.pieces[move.promotion] |= move.target_square_bb
            # complete ep move
            if self.ep_square_bb:
                if move.target_square_bb == self.ep_square_bb:
                    captured_pawn_bb = move_down(
                        self.ep_square_bb) if self.active_side else move_up(self.ep_square_bb)
                    captured_pawn = self.get_piece_on_square(captured_pawn_bb)
                    self.pieces[captured_pawn] &= ~captured_pawn_bb
                    self.enemies_bb &= ~captured_pawn_bb
                self.ep_square_bb = 0
                capture = True
            # check for resulting en passant
            moved_upx2 = move.origin_square_bb & R2 and move.target_square_bb & R4
            moved_downx2 = move.origin_square_bb & R7 and move.target_square_bb & R5
            if moved_upx2 or moved_downx2:
                left_square_piece = self.get_piece_on_square(
                    move_left(move.target_square_bb))
                right_square_piece = self.get_piece_on_square(
                    move_right(move.target_square_bb))
                enemy_pawn_key = 'p' if self.active_side else 'P'
                if left_square_piece == enemy_pawn_key or right_square_piece == enemy_pawn_key:
                    self.ep_square_bb = move_down(move.target_square_bb & R4) | move_up(
                        move.target_square_bb & R5)
        else:
            self.ep_square_bb = 0

        # castles
        # check rook moves
        if origin_piece == 'R' or target_piece == 'R':
            if move.origin_square_bb == (H & R1) or move.target_square_bb == (H & R1):
                self.castle_w_king_side = False
            elif move.origin_square_bb == (A & R1) or move.target_square_bb == (A & R1):
                self.castle_w_queen_side = False
        elif origin_piece == 'r' or target_piece == 'r':
            if move.origin_square_bb == (H & R8) or move.target_square_bb == (H & R8):
                self.castle_b_king_side = False
            elif move.origin_square_bb == (A & R8) or move.target_square_bb == (A & R8):
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
            if not (king_moves(move.origin_square_bb, self.friendlies_bb) & move.target_square_bb):
                # castle king side
                if move.target_square_bb & move_rightx2(move.origin_square_bb):
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
        self.friendlies_bb &= ~move.origin_square_bb
        self.friendlies_bb |= move.target_square_bb
        if capture:
            self.half_moves = 0
            self.enemies_bb &= ~move.target_square_bb
        else:
            self.half_moves += 1
        if self.active_side:
            self.full_moves += 1
        # swap sides
        self.active_side = not self.active_side
        self.friendlies_bb, self.enemies_bb = self.enemies_bb, self.friendlies_bb
        # unmake move if it was illegal
        if self.attacked_squares(self.active_side) & self.pieces['K' if not self.active_side else 'k']:
            self.restore(stored_board)
            return False
        return True
