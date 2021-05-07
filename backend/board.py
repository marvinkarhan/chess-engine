from random import random
from constants import *
from move_helper import *
from move import *
import time


def get_msb_generator(bb: int):
    while bb > 0:
        msb_index = bb.bit_length() - 1
        bb = set_bit_on_bb(bb, msb_index, 0)
        yield set_bit_on_bb(0, msb_index, 1)


class Board:

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

    def print_bitboard(self, bb: int):
        print('\n'.join(['{0:064b}'.format(bb)[i:i + 8]
              for i in range(0, 64, 8)]))

    def evaluate(self):
        bb = 1 << 63
        score = 0
        for x in range(0, 64, 1):
            piece = self.get_piece_on_square(bb)
            if(piece):
                score += PIECE_VALUES[piece]
            bb >>= 1
        print(score)
        return score

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
        if not castle_rights:
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

    def w_pieces_bb(self):
        bb = 0
        for k, v in self.pieces.items():
            if k.isupper():
                bb |= v
        return bb

    def b_pieces_bb(self):
        bb = 0
        for k, v in self.pieces.items():
            if k.islower():
                bb |= v
        return bb

    def all_pieces_bb(self):
        return self.w_pieces_bb() | self.b_pieces_bb()

    def get_active_pieces(self, active_side):
        return [v for k, v in self.pieces.items() if (k.isupper() if active_side else k.islower())]

    def get_piece_on_square(self, bb):
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
            self.ep_square_bb = set_bit_on_bb(
                0, ALGEBRAIC_TO_INDEX[ep_square], 1)

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

    def attacked_squares(self, active_side):
        attacked_bb = 0

        # swap friendlies and enemies locally
        friendlies_bb = self.enemies_bb if active_side != self.active_side else self.friendlies_bb
        enemies_bb = self.friendlies_bb if active_side != self.active_side else self.enemies_bb

        active_pieces = self.get_active_pieces(active_side)
        # filter pieces in case they got removed (eg. in legal_moves_generator)
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = [
            x & friendlies_bb for x in active_pieces]

        attacked_bb |= king_moves(
            king_bb, friendlies_bb) & enemies_bb
        attacked_bb |= pawn_attacks(
            pawn_bb, active_side, friendlies_bb, enemies_bb)
        attacked_bb |= knight_moves(
            knight_bb, friendlies_bb) & enemies_bb

        for pieces_bb, move_func in zip([rook_bb, bishop_bb, queen_bb], SLIDING_MOVES):
            for piece_bb in get_msb_generator(pieces_bb):
                attacked_bb |= move_func(
                    piece_bb, friendlies_bb, enemies_bb) & enemies_bb
        return attacked_bb

    def pseudo_legal_moves_generator(self, active_side):
        attacked_squares_bb = self.attacked_squares(not active_side)
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(
            active_side)
        # pawn moves
        for pawn in get_msb_generator(pawn_bb):
            for move in get_msb_generator(pawn_attacks(pawn, active_side, self.friendlies_bb, self.enemies_bb) | pawn_moves(pawn, active_side, self.friendlies_bb, self.enemies_bb)):
                yield Move(pawn, move)
            # en passant
            if self.ep_square_bb:
                move = pawn_attacks(
                    pawn, active_side, self.friendlies_bb, self.ep_square_bb) & self.ep_square_bb
                if move:
                    yield Move(pawn, move)

        # rook moves
        for rook in get_msb_generator(rook_bb):
            for move in get_msb_generator(rook_moves(rook, self.friendlies_bb, self.enemies_bb)):
                yield Move(rook, move)

        # bishop moves
        for bishop in get_msb_generator(bishop_bb):
            for move in get_msb_generator(bishop_moves(bishop, self.friendlies_bb, self.enemies_bb)):
                yield Move(bishop, move)

        # queen moves
        for queen in get_msb_generator(queen_bb):
            for move in get_msb_generator(queen_moves(queen, self.friendlies_bb, self.enemies_bb)):
                yield Move(queen, move)

        # knight moves
        for knight in get_msb_generator(knight_bb):
            for move in get_msb_generator(knight_moves(knight, self.friendlies_bb)):
                yield Move(knight, move)

        # king moves (there is always only one king)
        for move in get_msb_generator(king_moves(king_bb, self.friendlies_bb) & ~attacked_squares_bb):
            yield Move(king_bb, move)

        # king castle
        if king_bb & ~attacked_squares_bb:
            if active_side and self.castle_w_king_side or not active_side and self.castle_b_king_side:
                way_bb = (move_right(king_bb) | move_rightx2(king_bb))
                if (way_bb & ~self.friendlies_bb & ~attacked_squares_bb) == way_bb:
                    yield Move(king_bb, move_rightx2(king_bb))
            if active_side and self.castle_w_queen_side or not active_side and self.castle_b_queen_side:
                way_bb = (move_left(king_bb) | move_leftx2(king_bb))
                if (way_bb & ~self.friendlies_bb & ~attacked_squares_bb) == way_bb:
                    yield Move(king_bb, move_leftx2(king_bb))

    def legal_moves_generator(self):
        king_bb = self.pieces['K' if self.active_side else 'k']
        for pseudo_legal_move in self.pseudo_legal_moves_generator(self.active_side):
            king_bb_copy = king_bb
            # check if piece moving is king and update his position (other pieces are in this case the same)
            if pseudo_legal_move.origin_square_bb == king_bb_copy:
                king_bb_copy = pseudo_legal_move.target_square_bb
            friendlies_bb_copy = self.friendlies_bb
            enemies_bb_copy = self.enemies_bb
            # make move on copy
            self.friendlies_bb &= ~pseudo_legal_move.origin_square_bb
            self.friendlies_bb |= pseudo_legal_move.target_square_bb
            self.enemies_bb &= ~pseudo_legal_move.target_square_bb
            # check if king is attacked on changed board, if not move is valid
            if not (self.attacked_squares(not self.active_side) & king_bb_copy):
                self.friendlies_bb = friendlies_bb_copy
                self.enemies_bb = enemies_bb_copy
                yield pseudo_legal_move
            self.friendlies_bb = friendlies_bb_copy
            self.enemies_bb = enemies_bb_copy

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
            for move in self.legal_moves_generator():
                cur_state = self.store_board()
                self.make_move(move)
                # self.print_bitboard(self.w_pieces_bb() | self.b_pieces_bb())
                move_tree[move] = self.get_moves_tree(depth - 1)
                self.restore_board(cur_state)
                # print('-------------------')
        return move_tree

    def store_board(self):
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

    def restore_board(self, restore_dict: dict):
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
        if move not in list(self.legal_moves_generator()):
            print('Invalid Move: ', move)
            print('Valid moves are: ', list(
                self.legal_moves_generator()))
            return

        # track if capture for half_moves
        capture = False

        origin_piece = self.get_piece_on_square(move.origin_square_bb)
        target_piece = self.get_piece_on_square(move.target_square_bb)

        # update bitboards to represent change
        self.pieces[origin_piece] &= ~move.origin_square_bb
        self.pieces[origin_piece] |= move.target_square_bb
        self.friendlies_bb &= ~move.origin_square_bb
        self.friendlies_bb |= move.target_square_bb
        # target piece only exists on capture
        if target_piece:
            self.pieces[target_piece] &= ~move.target_square_bb
            capture = True

        # en passant
        if origin_piece in ['P', 'p']:
            # complete ep move
            if self.ep_square_bb:
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

        # castles
        # check rook moves
        if origin_piece == 'R':
            if move.origin_square_bb == (H & R1):
                self.castle_w_king_side = False
            elif move.origin_square_bb == (A & R1):
                self.castle_w_queen_side = False
        elif origin_piece == 'r':
            if move.origin_square_bb == (H & R8):
                self.castle_b_king_side = False
            elif move.origin_square_bb == (A & R8):
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
            if not king_moves(move.origin_square_bb, self.friendlies_bb) & move.target_square_bb:
                # castle king side
                if move.target_square_bb & move_rightx2(move.origin_square_bb):
                    # get rook
                    rook_square = (H & R1) if self.active_side else (H & R8)
                    target_square = move_leftx2(move_right(rook_square))
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


if __name__ == '__main__':
    start_time = time.time()
    board = Board(
        'rnb1kbnr/pppp1ppp/8/4p3/4P3/5Pq1/PPPP3P/RNBQKBNR w KQkq - 0 4')
    moves_tree = board.get_moves_tree(1)
    print(moves_tree)
    print(f'--- total runtime: {time.time() - start_time} seconds ---')
