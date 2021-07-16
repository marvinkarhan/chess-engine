#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "time.h"
#include "uci.h"
#include "move.h"

void uciGo(Board &board)
{
  int eval = board.evaluateNextMove();
  // int eval = board.iterativeDeepening(5);
}

void uciPosition(Board &board, std::istringstream &ss)
{
  Move move;
  std::string token, fen;

  ss >> token;
  // parse special case
  if (token == "startpos")
  {
    fen = START_POS_FEN;
    ss >> token; // get moves
  }
  else if (token == "fen")
  {
    // collect fen
    while (ss >> token && token != "moves")
      fen += token + " ";
  }
  else
    return; // no valid command given

  // parse fen
  board.parseFenString(fen);

  // parse moves if there are moves
  while (ss >> token && (move = uciToMove(token, board)) != 0)
  {
    board.makeMove(move);
  }
}

void uciLoop()
{
  std::cout << "uciLoop starting" << std::endl;
  Board board;
  std::string command, token;

  do
  {
    // getline or quit on EOF
    if (!getline(cin, command))
      command = "quit";

    // convert command line to stream to process
    std::istringstream ss(command);
    // clear and then fill token
    token.clear();
    ss >> skipws >> token;

    // commands
    if (token == "uci")
    {
      // general info about the engine (and options currently there are none)
      std::cout << "id name NoPy++" << std::endl;
      std::cout << "uciok" << std::endl;
    }
    else if (token == "isready")
      std::cout << "readyok" << std::endl;
    else if (token == "ucinewgame")
      board.parseFenString(START_POS_FEN);
    else if (token == "position")
      uciPosition(board, ss);
    else if (token == "go")
      uciGo(board);
    else if (token == "stop")
    {
      board.endTime = time(NULL);
      board.stopSearch = true;
      std::vector<Move> moves = board.getPV();
      std::cout << "bestmove " << toUciString(moves[0]) << std::endl;
    }
      // following is part of the UCI format but not jet implemented by NoPy++
    // else if (token == "setoption")
    // else if (token == "ponderhit")
    // else if (token == "register")
    // some custom debugging commands
    else if (token == "print")
      board.prettyPrint();

  } while (token != "quit");
  std::cout << "uciLoop finished" << std::endl;
}