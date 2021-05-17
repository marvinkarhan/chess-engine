

from flask.globals import request, session
import move_helper
from move import Move
from board import Board
from flask import Flask, render_template
from flask_socketio import SocketIO, emit, send
import random

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')


@socketio.on('test_message')
def on_test_message(message: str):
    print("Test-Message: " + message)

@socketio.on('make_move')
def on_make_move(uci_move: str):
    user_board: Board = session[request.sid]
    move = move_helper.uci_to_Move(uci_move)
    user_board.make_move(move)
    user_board.print_bitboard(user_board.all_pieces_bb())
    [next_move, score] = user_board.process_next_move(4, uci_move)
    print("neuer move", next_move, uci_move)
    user_board.make_move(next_move)
    user_board.print_bitboard(user_board.all_pieces_bb())
    moves = list(user_board.legal_moves_generator())
    uci_moves = [move.to_uci_string() for move in moves]
    emit('new_board_info', {'fen': user_board.to_fen_string(), 'moves': uci_moves, 'evaluation' : score})

@socketio.on('connect')
def on_connect(): 
    print('Created new Socket-Connection')

@socketio.on('new_board') 
def on_new_board():
    # user_board = Board('8/1k5P/8/8/8/8/8/7K w - - 0 1')
    user_board = Board()
    session[request.sid] = user_board
    moves = list(user_board.legal_moves_generator())
    uci_moves = [move.to_uci_string() for move in moves]
    emit('new_board_info', {'fen': user_board.to_fen_string(), 'moves': uci_moves, 'evaluation': user_board.evaluate()})

if __name__ == '__main__':
    socketio.run(app)

