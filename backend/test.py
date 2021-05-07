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

    # def test_depth0(self):
    #     board = Board()
    #     self.assertEqual(len(board.get_moves_tree(0)), 20)
    def test_perft1(self):
        board = Board()
        moves_tree = board.get_moves_tree(1)
        _count = count(moves_tree)
        print(_count)
        self.assertEqual(_count, 20)

    def test_perft2(self):
        board = Board()
        moves_tree = board.get_moves_tree(2)
        _count = count(moves_tree)
        print(_count)
        self.assertEqual(_count, 400)

    def test_perft3(self):
        board = Board()
        moves_tree = board.get_moves_tree(3)
        _count = count(moves_tree)
        print(_count)
        self.assertEqual(_count, 8902)


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
