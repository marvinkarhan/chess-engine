from constants import *
from move_helper import *
from move import *
import time


START_POS_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'


def get_msb_generator(bb: int):
    while bb > 0:
        msb_index = bb.bit_length() - 1
        bb = set_bit_on_bb(bb, msb_index, 0)
        yield set_bit_on_bb(0, msb_index, 1)


class Board:

    pieces = {
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

    castle_w_king_side = True
    castle_w_queen_side = True
    castle_b_king_side = True
    castle_b_queen_side = True

    # 0: black, 1: white
    active_side = 1
    full_moves = 0
    half_moves = 0

    ep_square_bb = 0

    def __init__(self) -> None:
        self.parse_FEN_string(
            # 'rnb1kbnr/1ppp2p1/p3pQ1p/1B3p2/4Pq2/7P/PPPP1PPR/RNB1K1N1 w Q - 0 9')
            'rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2')
        # for x in self.pseudo_legal_moves_generator(self.active_side):
        #     print(x)
        self.print_bitboard(self.pieces['Q'])
        print(len([x for x in self.pseudo_legal_moves_generator(self.active_side)]))
        pass

    def print_bitboard(self, bb: int):
        print('\n'.join(['{0:064b}'.format(bb)[i:i + 8]
              for i in range(0, 64, 8)]))

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

    def parse_FEN_string(self, fen: str):
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

    def attacked_squares(self, active_side, enemies_bb=0):
        attacked_bb = 0
        friendlies_bb = self.w_pieces_bb() if active_side else self.b_pieces_bb()
        if not enemies_bb:
            enemies_bb = self.b_pieces_bb() if active_side else self.w_pieces_bb()
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(
            active_side)

        attacked_bb |= king_moves(king_bb, friendlies_bb) & enemies_bb
        attacked_bb |= pawn_attacks(
            pawn_bb, active_side, friendlies_bb, enemies_bb)
        attacked_bb |= knight_moves(knight_bb, friendlies_bb) & enemies_bb

        for pieces_bb, move_func in zip([rook_bb, bishop_bb, queen_bb], SLIDING_MOVES):
            for piece_bb in get_msb_generator(pieces_bb):
                attacked_bb |= move_func(
                    piece_bb, friendlies_bb, enemies_bb) & enemies_bb
        return attacked_bb

    def pseudo_legal_moves_generator(self, active_side):
        friendlies_bb = self.w_pieces_bb() if active_side else self.b_pieces_bb()
        enemies_bb = self.b_pieces_bb() if active_side else self.w_pieces_bb()
        pawn_bb, rook_bb, knight_bb, bishop_bb, queen_bb, king_bb = self.get_active_pieces(
            active_side)
        move_bbs = []
        # pawn moves
        for pawn in get_msb_generator(pawn_bb):
            for move in get_msb_generator(pawn_attacks(pawn, active_side, friendlies_bb, enemies_bb) | pawn_moves(pawn, active_side, friendlies_bb, enemies_bb)):
                yield Move(pawn, move)

        # TODO: en passant

        # rook moves
        for rook in get_msb_generator(rook_bb):
            for move in get_msb_generator(rook_moves(rook, friendlies_bb, enemies_bb)):
                yield Move(rook, move)

        # bishop moves
        for bishop in get_msb_generator(bishop_bb):
            for move in get_msb_generator(bishop_moves(bishop, friendlies_bb, enemies_bb)):
                yield Move(bishop, move)

        # queen moves
        for queen in get_msb_generator(queen_bb):
            for move in get_msb_generator(queen_moves(queen, friendlies_bb, enemies_bb)):
                yield Move(queen, move)

        # knight moves
        for knight in get_msb_generator(knight_bb):
            for move in get_msb_generator(knight_moves(knight, friendlies_bb)):
                yield Move(knight, move)

        # king moves (there is always only one king)
        for move in get_msb_generator(king_moves(king_bb, friendlies_bb)):
            yield Move(king_bb, move)

        # TODO: Castle

        # yield moves

        pass

    def stalemate(self, active_side):
        pass

    def checkmate(self, active_side):
        pass

    def make_move(self, move):
        """
        checks:
        - checking if move is legal
        - checking castling rights
        - change pos of target piece
        - replace hostile pieces (if any)
        - check for ep and castle
        If illegal reset to init state
        """
        pass


start_time = time.time()
Board()
print(f'--- total runtime: {time.time() - start_time} seconds ---')
