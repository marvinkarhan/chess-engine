import os
from datetime import datetime
from typing import List

DEFAULT_ENGINE_SAVE = '../build/bin/uci-engine'

# using c-chess-cli: https://github.com/lucasart/c-chess-cli

class Engine:
  def __init__(self, name = DEFAULT_ENGINE_SAVE):
    self.name = name

  def to_string(self):
    return f' -engine cmd={self.name} '

class Tournament:
  def __init__(self, engines: List[Engine], games: int, time: int, increment: int):
    self.engines = engines
    self.games = games
    self.tc = f'{time}+{increment}' if increment else f'{time}'

  def start(self):
    cmd = (
      f'./c-chess-cli -each tc={self.tc} '
      f'{"".join([e.to_string() for e in self.engines])} '
      f'-games {self.games} '
      f'-pgn tournaments/{datetime.now().strftime("%d-%m-%Y_%H-%M-%S")}.pgn '
      # f'-draw number=40 count=8 score=10 '
      f'-log '
    )
    print(cmd)
    os.system(cmd)

def main():
  engines = [Engine(), Engine()]
  tournament = Tournament(engines, 1, 20, 0)
  tournament.start()

if __name__ == '__main__':
  main()