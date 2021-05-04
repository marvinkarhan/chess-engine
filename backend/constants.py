
START_POS_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

FULL_BB_MASK = 0xffffffffffffffff

# masks for lines
A = 0x8080808080808080
B = A >> 1
C = B >> 1
D = C >> 1
E = D >> 1
F = E >> 1
G = F >> 1
H = G >> 1
# masks for ranks
R1 = 0xff
R2 = R1 << 8
R3 = R2 << 8
R4 = R3 << 8
R5 = R4 << 8
R6 = R5 << 8
R7 = R6 << 8
R8 = R7 << 8

FIELDS_TO_INDEX = [
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
] = range(63, -1, -1)

ALGEBRAIC_TO_INDEX = {
    'a8': 63, 'b8': 62, 'c8': 61, 'd8': 60, 'e8': 59, 'f8': 58, 'g8': 57, 'h8': 56,
    'a7': 55, 'b7': 54, 'c7': 53, 'd7': 52, 'e7': 51, 'f7': 50, 'g7': 49, 'h7': 48,
    'a6': 47, 'b6': 46, 'c6': 45, 'd6': 44, 'e6': 43, 'f6': 42, 'g6': 41, 'h6': 40,
    'a5': 39, 'b5': 38, 'c5': 37, 'd5': 36, 'e5': 35, 'f5': 34, 'g5': 33, 'h5': 32,
    'a4': 31, 'b4': 30, 'c4': 29, 'd4': 28, 'e4': 27, 'f4': 26, 'g4': 25, 'h4': 24,
    'a3': 23, 'b3': 22, 'c3': 21, 'd3': 20, 'e3': 19, 'f3': 18, 'g3': 17, 'h3': 16,
    'a2': 15, 'b2': 14, 'c2': 13, 'd2': 12, 'e2': 11, 'f2': 10, 'g2': 9, 'h2': 8,
    'a1': 7, 'b1': 6, 'c1': 5, 'd1': 4, 'e1': 3, 'f1': 2, 'g1': 1, 'h1': 0,
}

PROMOTION_OPTIONS = ['N', 'n', 'B', 'b', 'Q', 'q', 'R', 'r']
