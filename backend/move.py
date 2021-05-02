from constants import *


class Move:
    def __init__(self, origin_square_bb: int, target_square_bb: int, promotion=False) -> None:
        self.origin_square_bb = origin_square_bb
        self.target_square_bb = target_square_bb
        self.promotion = promotion

    def __eq__(self, other):
        return (self.origin_square_bb == other.origin_square_bb and
                self.target_square_bb == other.target_square_bb and
                self.promotion == other.promotion)
