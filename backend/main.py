# Import the extension module hello.
import move_helper
import _move_helper
import _board
import board
import time
import cProfile

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
uci_move = 'e2e4'
move = move_helper.uci_to_Move('e2e4')
board.make_move(move)
board.print_bitboard(board.all_pieces_bb())
[next_move, score] = board.process_next_move(4, uci_move)
print("neuer move", next_move, uci_move)
print(f'--- total runtime: {time.time() - start_time} seconds ---')

# results:
# board.root_nega_max(4) - 2.4s
# board.root_nega_max(5) - 50.1s
# after turning pseudo_legal_moves_generator to not be a generator
# board.root_nega_max(4) - 2.6s
# board.root_nega_max(5) - 71.1s
# ---> reverting change