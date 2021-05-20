import board
import time
import move_helper
# import cProfile, pstats
# import pyximport
# pyximport.install()


# _board = board.Board()
_board = board.Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/4P3/2p2QPp/PPPBBP1P/R3K2R w KQkq - 0 1")

start_time = time.time()
# profiler = cProfile.Profile()
# profiler.enable()
# [best_moves, alpha] = _board.nega_max(3, -20000000, 20000000)
# print(best_moves)
# _board.make_move(move_helper.uci_to_Move('g2g3'))
# [best_moves, alpha] = _board.nega_max(2, -20000000, 20000000)
# profiler.disable()
# stats = pstats.Stats(profiler).sort_stats('tottime')
# stats.print_stats()
# cProfile.runctx("board.root_nega_max(5)", globals(), locals(), "Profile.prof")

# stats = pstats.Stats("Profile.prof").sort_stats('tottime')
# stats.print_stats()
# board.new_legal_moves_generator()
# board.print_moves()
# print(move_helper.bitScanForward(0b10000000))
# print(len(list(_board.pseudo_legal_moves_generator_2(1))))
print(list(_board.pseudo_legal_moves_generator(_board.get_active_side())))
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