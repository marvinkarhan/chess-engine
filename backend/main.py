# Import the extension module hello.
import board
import time
import cProfile, pstats
import pyximport
pyximport.install()

# start_time = time.time()
# print('run cython rookmoves 1000000 times:')
# for i in range(0, 1000000):
#   board.get_lsb_array(i)
# print(f'--- total runtime: {time.time() - start_time} seconds ---')

# print('run python rook moves 1000000 times:')
# start_time = time.time()
# for i in range(0, 1000000):
#   _board.get_lsb_array(i)
# print(f'--- total runtime: {time.time() - start_time} seconds ---')


my_board = board.Board()
start_time = time.time()
#profiler = cProfile.Profile() 
#profiler.enable()
tree = my_board.nega_max(5,-200000,200000)
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

#profiler.disable()
#stats = pstats.Stats(profiler).sort_stats('tottime')
#stats.print_stats()
# cProfile.runctx("my_board.nega_max(5,-200000,200000)", globals(), locals(), "Profile.prof")

# stats = pstats.Stats("Profile.prof").sort_stats('tottime')
# stats.print_stats()
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