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

    u64 CASTLING_W_KING_SIDE_WAY
    u64 CASTLING_W_KING_SIDE_SQUARE
    u64 CASTLING_W_QUEEN_SIDE_WAY
    u64 CASTLING_W_QUEEN_SIDE_SQUARE
    u64 CASTLING_B_KING_SIDE_WAY
    u64 CASTLING_B_KING_SIDE_SQUARE
    u64 CASTLING_B_QUEEN_SIDE_WAY
    u64 CASTLING_B_QUEEN_SIDE_SQUARE
    u64[4][2] CASTELING_ARR

    u64 debruijn64
    int[64] debruijn64_index64

    FIELDS_TO_INDEX
    dict ALGEBRAIC_TO_INDEX
    dict PIECE_VALUES
    dict PIECE_SQUARE_TABLES
    list PROMOTION_OPTIONS_W
    list PROMOTION_OPTIONS_B

    # precalc move bb arrays
    u64[64] HORIZONTAL_MOVE_BBS
    u64[64] VERTICAL_MOVE_BBS
    u64[64] KNIGHT_MOVE_BBS
    u64[64] ROOK_MOVE_BBS
    u64[64] BISHOP_MOVE_BBS
    u64[64] QUEEN_MOVE_BBS
    list PAWN_MOVE_BBS
    list PAWN_ATTACKS_BBS
    u64[64] KING_MOVES_BBS
    u64[64] SQUARE_BBS
    u64[64][64] REY_BBS
    u64[64][64] LINE_BBS

    dict OPENING_TABLE
    u64[781] ZOBRIST_TABLE
    dict OPENING_TABLE
    dict EVALUATE_TABLE
