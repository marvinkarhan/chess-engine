from constants import *


class Move:

    origin_square_bb = 0
    target_square_bb = 0

    def __init__(self, origin_square_bb: int, target_square_bb: int) -> None:
        self.origin_square_bb = origin_square_bb
        self.target_square_bb = target_square_bb
