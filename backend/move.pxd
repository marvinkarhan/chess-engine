ctypedef unsigned long long u64


cdef class Move:
    cdef u64 origin_square_bb, target_square_bb
    cdef str promotion