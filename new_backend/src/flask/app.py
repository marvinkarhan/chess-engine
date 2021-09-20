from flask.globals import session
from flask import Flask
from flask.templating import render_template_string
from flask_socketio import SocketIO, disconnect, emit
from pexpect import popen_spawn, exceptions
import os
import signal
import re
import platform

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, cors_allowed_origins='*')

cwd = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
fileName = 'chess-server-uci-exe.exe' if platform.system() == 'Windows' else 'chess-server-uci-exe'
uciEnginePath = os.path.join(cwd, f'./../../build/{fileName}')


@socketio.on('connect')
def connect():
    uciEngine = popen_spawn.PopenSpawn(uciEnginePath)
    uciEngine.expect('uciLoop starting')
    session['uciEngine'] = uciEngine
    print('Created new Socket-Connection')


@socketio.on('disconnect')
def disconnect():
    uciEngine: popen_spawn.PopenSpawn = session['uciEngine']
    uciEngine.sendline('stop')
    uciEngine.kill(signal.SIGTERM)
    print('Closed connection')


@socketio.on('new_board')
def newBoard(fen: str):
    uciEngine: popen_spawn.PopenSpawn = session['uciEngine']
    if (fen != ''):
        uciEngine.sendline(f'position fen {fen}')
    emit('board_info', getBoardInfo(uciEngine))


@socketio.on('unmake_move')
def unmakeMove():
    uciEngine: popen_spawn.PopenSpawn = session['uciEngine']
    uciEngine.sendline('unmakemove')
    readLines(uciEngine, 'unmademove', 1)
    emit('board_info', getBoardInfo(uciEngine))

@socketio.on('new_engine_move')
def newEngineMove():
    uciEngine: popen_spawn.PopenSpawn = session['uciEngine']
    movetime = session.get('movetime')
    uciEngine.sendline(f'go movetime {movetime}' if movetime else 'go')
    searchInfo: list[str] = readLines(
        uciEngine, '.*bestmove.*', movetime or -1).splitlines()
    # get engine move from information string
    engineMoveMatch = re.match(r'bestmove \S*', searchInfo[-1])
    if engineMoveMatch:
        engineMove = searchInfo[-1].replace('bestmove', '').strip()
        # if game is not over
        if engineMove != '(none)':
            makeMoveInEngine(uciEngine, engineMove)
    lastSearchInfo = searchInfo[-2]
    # get evaluation from information string
    evaluation = 0
    evaluationMatch = re.search(r'cp -?\d*', lastSearchInfo)
    if evaluationMatch:
        evaluation = int(evaluationMatch.group(0).replace('cp ', ''))
    # get aiMoves from information
    aiMoves = ['']
    aiMovesMatch = re.search(r"pv (\S{4}(\s|$))+", lastSearchInfo)
    if aiMovesMatch:
        aiMoves = aiMovesMatch.group(0).replace('pv ', '').strip().split(' ')
    emit('board_info', getBoardInfo(uciEngine, evaluation, aiMoves))


@socketio.on('make_move')
def makeMove(uciMove: str):
    uciEngine: popen_spawn.PopenSpawn = session['uciEngine']
    makeMoveInEngine(uciEngine, uciMove)
    emit('board_info', getBoardInfo(uciEngine, None))


@socketio.on('change_time')
def changeTime(time: int):
    session['movetime'] = time


def readLines(uciEngine: popen_spawn.PopenSpawn, search: str, timeout: int) -> str:
    try:
        uciEngine.expect(search, timeout=timeout)
        return uciEngine.after.decode().strip()
    except exceptions.TIMEOUT:
        emit(f'Timed out searching {timeout}s for {search}')
        return 'Timed out'


def getBoardInfo(uciEngine: popen_spawn.PopenSpawn, cpEvaluation=None, aiMoves=None):
    return {'fen': getFen(uciEngine), 'moves': getMoves(uciEngine), 'evaluation': cpEvaluation / 100 if cpEvaluation else None, 'aiMoves': aiMoves}


def getFen(uciEngine: popen_spawn.PopenSpawn):
    uciEngine.sendline('fen')
    return readLines(uciEngine, 'fen.*', 1).replace('fen ', '')


def getMoves(uciEngine: popen_spawn.PopenSpawn):
    uciEngine.sendline('legalmoves')
    movesStr = readLines(uciEngine, 'legalmoves.*', 1).replace('legalmoves ', '')
    return movesStr.split(' ')


def makeMoveInEngine(uciEngine: popen_spawn.PopenSpawn, uciMove: str):
    uciEngine.sendline(f'move {uciMove}')


if __name__ == '__main__':
    socketio.run(app)
