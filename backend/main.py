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


board = board.Board()
start_time = time.time()
# profiler = cProfile.Profile()
# profiler.enable()
# tree = board.root_nega_max(4)
# profiler.disable()
# stats = pstats.Stats(profiler).sort_stats('tottime')
# stats.print_stats()
cProfile.runctx("board.root_nega_max(5)", globals(), locals(), "Profile.prof")

stats = pstats.Stats("Profile.prof").sort_stats('tottime')
stats.print_stats()
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