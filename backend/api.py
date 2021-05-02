from flask import Flask, render_template
from flask_socketio import SocketIO, send

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

@socketio.on('message')
def onMessage(message: str):
    print(message)

@socketio.on('test_message')
def on_test_message(message: str):
    print("Test-Message: " + message);

@socketio.on('connect')
def on_connect(): 
    print('Created new Socket-Connection')

if __name__ == '__main__':
    socketio.run(app)