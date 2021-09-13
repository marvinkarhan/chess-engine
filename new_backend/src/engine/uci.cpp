#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "time.h"
#include "uci.h"
#include "move.h"

void uciGo(Board &board, std::istringstream &ss)
{
  std::string token;
  int depth = 64;
  int movetime = 0; // in ms

  // collect options
  while (ss >> token)
  {
    // TODO: searchmoves
    // TODO: ponder
    // TODO: wtime
    // TODO: btime
    // TODO: winc
    // TODO: binc
    // TODO: movestogo
    if (token == "depth")
      ss >> depth;
    // TODO: nodes
    // TODO: mate
    else if (token == "movetime")
      ss >> movetime;
    // TODO: infinite
  }
  std::cout << "movetime: " << (float) movetime / 100 << std::endl;
  board.evaluateNextMove((float) movetime / 100, depth);
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

void uciLegalMoves(Board &board)
{
  std::cout << "legalmoves ";
  for (Move move : MoveList<LEGAL_MOVES>(board, board.activeSide))
  {
    const string value = toUciString(move);
    std::cout << value.c_str() << " ";
  }
  std::cout << std::endl;
}

void uciMove(Board &board, std::istringstream &ss)
{
  Move move;
  std::string token;

  // parse moves if there are moves
  while (ss >> token && (move = uciToMove(token, board)) != 0)
  {
    board.makeMove(move);
  }
}

void uciUnmakeMove(Board &board)
{
  if (board.state->move)
  {
    board.unmakeMove(board.state->move);
  }
  std::cout << "unmademove" << std::endl;
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
      uciGo(board, ss);
    else if (token == "stop")
    {
      board.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      board.stopSearch = true;
      std::vector<Move> moves = board.getPV();
      std::cout << "bestmove " << toUciString(moves[0]) << std::endl;
    }
    // following is part of the UCI format but not jet implemented by NoPy++
    // else if (token == "setoption")
    // else if (token == "ponderhit")
    // else if (token == "register")
    // some custom commands
    else if (token == "print")
      board.prettyPrint();
    else if (token == "fen")
      std::cout << "fen " << board.toFenString() << std::endl;
    else if (token == "legalmoves")
      uciLegalMoves(board);
    else if (token == "move")
      uciMove(board, ss);
    else if (token == "unmakemove")
      uciUnmakeMove(board);
  } while (token != "quit");
  std::cout << "uciLoop finished" << std::endl;
}