ctypedef unsigned long long u64

cdef enum move_type:
    NORMAL
    PROMOTION
    EN_PASSANT
    CASTLING

cdef class Move:
    cdef int origin_square, target_square
    cdef str promotion
    cdef move_type type