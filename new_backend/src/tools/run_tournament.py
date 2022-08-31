import os
from datetime import datetime
from typing import List

DEFAULT_ENGINE_SAVE = '../build/bin/uci-engine'
CLASSIC_ENGINE = '../../../../saved_engines/wsl/classic'
STOCKFISH_WITH_MY_NET = './engines/stockfish_mse_55_epoch'
OPENING_BOOK_EPD = './books/UHO_V3_6mvs_+090_+099.epd'
NNUE_CENTROPY_111_EPOCH = './engines/centropy_111_epoch'
STOCKFISH = './engines/stockfish_11_x64'
QS_CHECKS = './engines/withQSChecks'

# using cutechess (cli): https://github.com/cutechess/cutechess

class Engine:
  def __init__(self, file = DEFAULT_ENGINE_SAVE, name = '', options=[]):
    self.file = file
    self.name = name
    self.option = "".join([f'option.{o[0]}={o[1]} ' for o in options])

  def to_string(self):
    string = f' -engine cmd={self.file} '
    if self.name:
      string += f'name={self.name} '
    if self.option:
      string += self.option
    return string

class Tournament:
  def __init__(self, engines: List[Engine], rounds: int, time: int, increment = 0):
    self.engines = engines
    self.rounds = rounds
    self.tc = f'{time}+{increment}' if increment else f'{time}'
   
  def start(self):
    cmd = (
      f'./cutechess-cli/cutechess-cli -each proto=uci tc={self.tc} '
      f'{"".join([e.to_string() for e in self.engines])} '
      f'-event {"_vs_".join(["_".join(e.name.split()) for e in self.engines])} '
      f'-games 2 '
      f'-rounds {self.rounds} '
      f'-pgnout tournaments/{datetime.now().strftime("%d-%m-%Y_%H-%M-%S")}.pgn '
      f'-openings file={OPENING_BOOK_EPD} format=epd -repeat '
      f'-concurrency 6'
      # f'-draw movenumber=40 movecount=8 score=10 '
    )
    print(cmd)
    os.system(cmd)

def main():
  engines = [Engine(name='NNUE'), Engine(QS_CHECKS, name='WITH_QS_CHECKS')]
  tournament = Tournament(engines, 6, 60)
  tournament.start()

if __name__ == '__main__':
  main()