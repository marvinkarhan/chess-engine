import unittest
from board import *

# Performance test to test the move generation algorithm


class TestFullGame(unittest.TestCase):

    def test_shortest_game(self):
        board = Board()
        board.make_move(uci_to_Move('f2f3'))
        board.make_move(uci_to_Move('e7e6'))

        board.make_move(uci_to_Move('g2g4'))
        board.make_move(uci_to_Move('d8h4'))

        self.assertTrue(board.checkmate())


class Perft(unittest.TestCase):

    def test_perft1_one_move(self):
        board = Board(
            'rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprrrrp/nbbpnPnN/knbbnKn1 b - - 0 1')
        self.assertEqual(board.perft(1), 1)

    def test_perft1_checkmate(self):
        board = Board(
            'rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprr1rp/nbbpnrnN/knbbnKn1 w - - 0 2')
        self.assertEqual(board.perft(2), 0)

    def test_perft1_stalemate(self):
        board = Board(
            'rrrrrrrr/rrrrrrrr/rrrrrrrr/rrrrrrrr/rbrrrrrr/pbprrrrp/nbbpnrnN/knbbnrnK b - - 0 1')
        self.assertEqual(board.perft(2), 0)

    def test_perft2_promotion(self):
        board = Board('8/1k5P/8/8/8/8/8/7K w - - 0 1')
        self.assertEqual(board.perft(2), 50)

    def test_perft1(self):
        board = Board()
        self.assertEqual(board.perft(1), 20)

    def test_perft2(self):
        board = Board()
        self.assertEqual(board.perft(2), 400)

    def test_perft3(self):
        board = Board()
        self.assertEqual(board.perft(3), 8902)

    def test_perft4(self):
        board = Board()
        self.assertEqual(board.perft(4), 197281)

    def test_perft5(self):
        board = Board()
        # moves_tree = board.get_moves_tree(5)
        # for key, value in moves_tree.items():
        #     print(F'{key}: {count_root(value)}')
        # _count = count_root(moves_tree)
        # self.assertEqual(_count, 4865609)
        self.assertEqual(board.perft(5), 4865609)

    def test_perft2_Kiwipete(self):
        board = Board(
            'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        self.assertEqual(board.perft(2), 2039)

    def test_perft3_Kiwipete(self):
        board = Board(
            'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        self.assertEqual(board.perft(3), 97862)

    def test_perft4_Kiwipete(self):
        board = Board(
            'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        self.assertEqual(board.perft(4), 4085603)

    def test_perft_iter_Kiwipete(self):
        board = Board(
            'r3k2r/p1ppqpb1/bn2pQp1/3PN3/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 0 1')
        self.assertEqual(board.perft(3), 77838)

    def test_perft_discover_promotions(self):
        board = Board('n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1')
        self.assertEqual(board.perft(5), 3605103)


if __name__ == '__main__':
    unittest.main(verbosity=2)
