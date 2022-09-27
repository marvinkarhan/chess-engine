import os
import argparse
import os
from datetime import datetime
from typing import List

DEFAULT_ENGINE = '../build/bin/uci-engine'
HCE_ENGINE = './engines/HCE'
CLASSIC_ENGINE = './engines/classic'
OPENING_BOOK_UHO = './books/UHO_V3_6mvs_+090_+099.epd'
OPENING_8MVS_V3 = './books/8mvs_big_+80_+109.epd'
STOCKFISH = './engines/stockfish_11_x64'
MAVERICK ='./engines/maverick-10-lin32-ja_2500'

# using cutechess (cli): https://github.com/cutechess/cutechess
ORDO = './ordo/ordo-linux64'

class Engine:
  def __init__(self, file = DEFAULT_ENGINE, name = '', options=[]):
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
    self.out_file = f'tournaments/{datetime.now().strftime("%d-%m-%Y_%H-%M-%S")}.pgn'
   
  def start(self):
    cmd = (
      f'./cutechess-cli/cutechess-cli -each proto=uci tc={self.tc} dir={os.path.dirname(os.path.realpath(__file__))} '
      f'{"".join([e.to_string() for e in self.engines])} '
      f'-event {"_vs_".join(["_".join(e.name.split()) for e in self.engines])} '
      f'-games 2 '
      f'-rounds {self.rounds} '
      f'-pgnout {self.out_file} '
      f'-openings file={OPENING_BOOK_UHO} format=epd order=random -repeat '
      f'-concurrency 64 '
      f'-resign movecount=3 score=1000 '
      f'-draw movenumber=40 movecount=8 score=10 '
      f'-recover '
    )
    print(cmd)
    os.system(cmd)

  def estimate_elo_with_ordo(self, relative_to = 'NNUE'):
    ordo_file = self.out_file.replace('.pgn', '_ordo.txt')
    cmd = (
      f'{ORDO} '
      f'-q ' # quiet
      f'-a 0 ' # avg to 0 in order to diff to master
      f'-A {relative_to} ' # show result relative to master
      f'-D ' # adjust for draw rate
      f'-W ' # adjust for white advantage
      f'-s 100 ' # use 100 simulations to estimate error
      f'-p {self.out_file} '
      f'-o {ordo_file} '
    )
    print(cmd)
    return os.system(cmd)

def main():
  parser = argparse.ArgumentParser(description='Plays a gauntlet tournament comparing the master to a chess engine.', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--engine', type=str, help='Set the path enemy to test against')
  parser.add_argument('--rounds', default=200, type=int, help='Number of rounds to play.')
  parser.add_argument('--tc', default=10, type=int, help='Number of Seconds for each Game.')
  args = parser.parse_args()
  enemy_name = os.path.split(os.path.normpath(args.engine))[-1]
  print(args.engine, enemy_name)
  engines = [Engine(name='NNUE'), Engine(args.engine, name=enemy_name)]
  tournament = Tournament(engines, args.rounds, args.tc, 0.1)
  tournament.start()
  tournament.estimate_elo_with_ordo(enemy_name)

if __name__ == '__main__':
  main()