from constants cimport *

cdef class Move:
    
    def __init__(self, int o_s, int t_s, move_type move_type=move_type.NORMAL, prmtn=""):
        self.origin_square = o_s
        self.target_square = t_s
        self.promotion = prmtn if prmtn else ""
        self.type = move_type

    def __eq__(self, other):
        return (self.origin_square == other.origin_square and
                self.target_square == other.target_square and
                self.promotion == other.promotion and
                self.move_type == other.move_type)

    def to_uci_string(self):
        keys = list(ALGEBRAIC_TO_INDEX.keys())
        return "{f}{t}{p}".format(
            # Keys inverts positions in ALGEBRAIC_TO_INDEX
            f=keys[-self.origin_square - 1],
            t=keys[-self.target_square - 1],
            p=self.promotion if self.promotion else ""
        )
        
    def __repr__(self):
        return self.to_uci_string()
        # return f'{self.origin_square} {self.target_square}'

    def __hash__(self):
        return hash((self.origin_square, self.target_square, self.promotion))

    def __copy__(self):
        return type(self)(self.origin_square, self.target_square, self.promotion)
