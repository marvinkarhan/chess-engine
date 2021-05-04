from constants import START_POS_FEN
from flask.globals import request, session
from move_helper import uci_to_Move
from board import Board
from flask import Flask, render_template
from flask_socketio import SocketIO, emit, send

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

@socketio.on('message')
def onMessage(message: str):
    print(message)

@socketio.on('test_message')
def on_test_message(message: str):
    print("Test-Message: " + message);

@socketio.on('get_board_moves')
def on_get_board_moves():
    user_board = session[request.sid]
    moves = list(user_board.legal_moves_generator(user_board.active_side))
    uci_moves = [move.to_uci_string() for move in moves]
    emit('new_board_moves', uci_moves)
    print(uci_moves)

@socketio.on('connect')
def on_connect(): 
    print('Created new Socket-Connection')

@socketio.on('new_board') 
def on_new_board():
    session[request.sid] = Board(START_POS_FEN)
    emit('board_initialized', START_POS_FEN)

if __name__ == '__main__':
    socketio.run(app)