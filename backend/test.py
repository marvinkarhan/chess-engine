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


if __name__ == '__main__':
    unittest.main(verbosity=2)
