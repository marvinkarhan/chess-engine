ctypedef unsigned long long u64

cdef enum move_type:
    NORMAL
    PROMOTION
    EN_PASSANT
    CASTLING

cdef class Move:
    cdef int _origin_square, _target_square
    cdef str _promotion
    cdef move_type _type