import unittest
from board import *

## Performance test to test the move generation algorithm


class TestFullGame(unittest.TestCase):

    def test_shortest_game(self):
        board = Board()
        board.parse_FEN_string(START_POS_FEN)
        board.make_move(uci_to_Move('f2f3'))
        board.make_move(uci_to_Move('e7e6'))

        board.make_move(uci_to_Move('g2g4'))
        board.make_move(uci_to_Move('d8h4'))

        self.assertTrue(board.checkmate())


class Perft(unittest.TestCase):
    
    def test_perft1_one_move(self):
        board = Board('rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprrrrp/nbbpnPnN/knbbnKn1 b - - 0 1')
        moves_tree = board.get_moves_tree(2)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 1)

    def test_perft1_checkmate(self):
        board = Board('rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprr1rp/nbbpnrnN/knbbnKn1 w - - 0 2')
        moves_tree = board.get_moves_tree(2)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 0)

    def test_perft1_stalemate(self):
        board = Board('rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprrrrp/nbbpnrnN/knbbnrnK b - - 0 1')
        moves_tree = board.get_moves_tree(2)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 0)

    def test_perft2_promotion(self):
        board = Board('8/1k5P/8/8/8/8/8/7K w - - 0 1')
        moves_tree = board.get_moves_tree(2)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 50)

    def test_perft1(self):
        board = Board()
        moves_tree = board.get_moves_tree(1)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 20)

    def test_perft2(self):
        board = Board()
        moves_tree = board.get_moves_tree(2)
        print(moves_tree)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 400)

    def test_perft3(self):
        board = Board()
        moves_tree = board.get_moves_tree(3)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 8902)

    def test_perft4(self):
        board = Board()
        moves_tree = board.get_moves_tree(4)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 197281)

    def test_perft5(self):
        board = Board()
        moves_tree = board.get_moves_tree(5)
        for key, value in moves_tree.items():
            print(F'{key}: {count_root(value)}')
        _count = count_root(moves_tree)
        self.assertEqual(_count, 4865609)

    def test_perft2_Kiwipete(self):
        board = Board('r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        moves_tree = board.get_moves_tree(2)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 2039)

    def test_perft3_Kiwipete(self):
        board = Board('r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        moves_tree = board.get_moves_tree(3)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 97862)

    def test_perft4_Kiwipete(self):
        board = Board('r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        moves_tree = board.get_moves_tree(4)
        _count = count_root(moves_tree)
        self.assertEqual(_count, 4085603)


def count_root(d: dict):
    sum = 0
    for k in d:
        if type(d[k]) is dict:
            sum += count(d[k])
    return sum


def count(d: dict):
    if d:
        sum = 0
        for k in d:
            sum += count(d[k])
        return sum
    else:
        return 1

if __name__ == '__main__':
    unittest.main(verbosity=2)
