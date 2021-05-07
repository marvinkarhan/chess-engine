import unittest
from unittest.case import TestCase
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

    def test_perft1(self):
        board = Board()
        moves_tree = board.get_moves_tree(1)
        _count = count(moves_tree)
        self.assertEqual(_count, 20)

    def test_perft2(self):
        board = Board()
        moves_tree = board.get_moves_tree(2)
        _count = count(moves_tree)
        self.assertEqual(_count, 400)

    def test_perft3(self):
        board = Board()
        moves_tree = board.get_moves_tree(3)
        _count = count(moves_tree)
        self.assertEqual(_count, 8902)

    # takes to long 08.05 (>100s)
    # def test_perft4(self):
    #     board = Board()
    #     moves_tree = board.get_moves_tree(4)
    #     _count = count(moves_tree)
    #     self.assertEqual(_count, 197281)

    def test_perft2_Kiwipete(self):
        board = Board('r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
        moves_tree = board.get_moves_tree(2)
        _count = count(moves_tree)
        self.assertEqual(_count, 2039)


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
