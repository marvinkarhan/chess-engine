from constants import FIELDS_TO_INDEX, ALGEBRAIC_TO_INDEX, START_POS_FEN
from flask.globals import request, session
from move_helper import set_bit_on_bb, uci_to_Move
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
    move = uci_to_Move(uci_move)
    user_board.make_move(move)
    moves = list(user_board.legal_moves_generator())
    uci_moves = [move.to_uci_string() for move in moves]
    emit('new_board_info', {'fen': user_board.to_fen_string(), 'moves': uci_moves, 'evaluation' : user_board.evaluate()})

@socketio.on('connect')
def on_connect(): 
    print('Created new Socket-Connection')

@socketio.on('new_board') 
def on_new_board():
    user_board = Board('r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0')
    session[request.sid] = user_board
    moves = list(user_board.legal_moves_generator())
    uci_moves = [move.to_uci_string() for move in moves]
    emit('new_board_info', {'fen': user_board.to_fen_string(), 'moves': uci_moves, 'evaluation': user_board.evaluate()})

if __name__ == '__main__':
    socketio.run(app)

