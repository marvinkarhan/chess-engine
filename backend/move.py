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

    def to_uci_string(self):
        keys = list(ALGEBRAIC_TO_INDEX.keys())
        return "{f}{t}{p}".format(
            f=keys[self.origin_square_bb.bit_length() - 1],
            t=keys[self.target_square_bb.bit_length() - 1],
            p=self.promotion if self.promotion else ""
        )
