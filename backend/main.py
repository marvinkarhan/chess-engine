import board
import time
import move_helper
import cProfile
import pstats
# import pyximport
# pyximport.install()


_board = board.Board()
# _board = board.Board("8/2k5/8/8/8/6pb/8/7K w - - 0 1")


my_board = board.Board()
start_time = time.time()
#profiler = cProfile.Profile()
# profiler.enable()
[move, score] = my_board.nega_max(5, -200000, 200000)
# for n in range(100):
#   start_time = time.time()
#   [move, score] = my_board.nega_max(3,-200000,200000)
#   my_board.make_move(move[-1])
#   print(f'--- total runtime: {time.time() - start_time} seconds ---')
# fen_list = ["rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1","rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1",
# "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1",
# "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1",
# "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1",
# "rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1",
# "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1"]
# for fen in fen_list:
#     new_board = board.Board(fen)
#     key = board.hash_board(new_board)
#     print(key)
#     key ^= board.get_zobrist()[768]
#     print(key)
#     key ^= board.get_zobrist()[768]
#     print(key)

# profiler.disable()
#stats = pstats.Stats(profiler).sort_stats('tottime')
# stats.print_stats()
# cProfile.runctx("my_board.nega_max(5,-200000,200000)", globals(), locals(), "Profile.prof")

# stats = pstats.Stats("Profile.prof").sort_stats('tottime')
# stats.print_stats()
# board.new_legal_moves_generator()
# board.print_moves()
# print(move_helper.bitScanForward(0b10000000))
# _board.print_bitboard(0x38)
# print(list(_board.legal_moves_generator()))
# print(move_helper.uci_to_Move('e7e8') in list(_board.legal_moves_generator()))
print(f'--- total runtime: {time.time() - start_time} seconds ---')

# results:
# board.root_nega_max(4) - 2.4s
# board.root_nega_max(5) - 50.1s
# after turning pseudo_legal_moves_generator to not be a generator
# board.root_nega_max(4) - 2.6s
# board.root_nega_max(5) - 71.1s
# ---> reverting change
# after move to cython
# board.root_nega_max(4) - 1.9s
# board.root_nega_max(5) - 30.1s
