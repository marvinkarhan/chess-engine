ctypedef unsigned long long u64

cdef:
    START_POS_FEN

    u64 FULL_BB_MASK

    # masks for lines
    u64 A
    u64 B
    u64 C
    u64 D
    u64 E
    u64 F
    u64 G
    u64 H
    u64 NOT_H
    u64 NOT_A
    # masks for ranks
    u64 R1
    u64 R2
    u64 R3
    u64 R4
    u64 R5
    u64 R6
    u64 R7
    u64 R8

    u64 debruijn64
    int[64] debruijn64_index64

    FIELDS_TO_INDEX
    dict ALGEBRAIC_TO_INDEX
    dict PIECE_VALUES
    dict PIECE_SQUARE_TABLES
    list PROMOTION_OPTIONS_W
    list PROMOTION_OPTIONS_B
    dict OPENING_TABLE