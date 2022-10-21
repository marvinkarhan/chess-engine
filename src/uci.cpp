#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "board.h"
#include "time.h"
#include "uci.h"
#include "move.h"
#include "movehelper.h"
#include "nnue/halfkp.h"
#include "nnue/init.h"

// implement First Use Idiom
Board &getBoard()
{
  static Board *ans = new Board();
  return *ans;
}

void uciGo(std::istringstream &ss)
{
  std::string token;
  int depth = 64;
  int movetime = 0, wtime = 0, btime = 0; // in ms

  // collect options
  while (ss >> token)
  {
    // TODO: searchmoves
    // TODO: ponder
    // TODO: wtime
    if (token == "wtime")
      ss >> wtime;
    // TODO: btime
    if (token == "btime")
      ss >> btime;
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
  getBoard().evaluateNextMove((float)movetime / 1000, (float)wtime / 1000, (float)btime / 1000, depth);
}

void uciPosition(std::istringstream &ss)
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
  getBoard().parseFenString(fen);

  // parse moves if there are moves
  while (ss >> token && (move = uciToMove(token, getBoard())) != 0)
  {
    getBoard().makeMove(move);
  }
}

void uciLegalMoves()
{
  std::cout << "legalmoves ";
  for (Move move : MoveList<LEGAL_MOVES>(getBoard(), getBoard().activeSide))
  {
    const string value = toUciString(move);
    std::cout << value.c_str() << " ";
  }
  std::cout << std::endl;
}

void uciMove(std::istringstream &ss)
{
  Move move;
  std::string token;

  // parse moves if there are moves
  while (ss >> token && (move = uciToMove(token, getBoard())) != 0)
  {
    getBoard().makeMove(move);
  }
}

void uciUnmakeMove()
{
  if (getBoard().state->move)
  {
    getBoard().unmakeMove(getBoard().state->move);
  }
  std::cout << "unmademove" << std::endl;
}

void uciPerft(std::istringstream &ss)
{
  std::string token, fen;

  ss >> token;
  int depth = std::stoi(token);

  while (ss >> token)
    fen += token + " ";
  if (!fen.empty())
    getBoard().parseFenString(fen);

  auto startTime = std::chrono::system_clock::now();
  u64 nodes = getBoard().perft(depth);
  auto endTime = std::chrono::system_clock::now();
  auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() + 1;
  std::cout << "info " << "nodes " << nodes << " nps " << nodes * 1000 / timePassed << " time " << timePassed << "ms" << std::endl;
}

void setOption(std::istringstream &ss)
{
  std::string token, id, value;

  ss >> token;
  // parse special case
  if (token == "name")
    while (ss >> token && token != "value")
      id += token + " ";
  else
  {
    std::cout << "Not an option" << std::endl;
    return;
  }
  while (ss >> token)
    value += token + " ";
  // remove suffixed spaces
  id.erase(id.find_last_not_of(' ') + 1);
  value.erase(value.find_last_not_of(' ') + 1);

  if (id == "UseNNUE" && (value == "true" || value == "false"))
  {
    getBoard().useNNUE = value == "true";
    std::cout << "Using NNUE: " << value << std::endl;
  }
  else if (id == "EvalFile" && value.length() > 0)
  {
    getBoard().useNNUE = NNUE::loadFile(value);
  }
  else
    std::cout << "Not an option" << std::endl;
}

void printOptions()
{
  std::cout << "option name nnue type check default true" << std::endl;
  std::cout << "option name nnueFile type string default <empty>" << std::endl;
}

std::string uciProcessCommand(std::string command)
{
  std::string token;
  // convert command line to stream to process
  std::istringstream ss(command);
  // fill token
  ss >> skipws >> token;

  // commands
  if (token == "uci")
  {
    // general info about the engine (and options currently there are none)
    std::cout << "id name NoPy++" << std::endl;
    printOptions();
    std::cout << "uciok" << std::endl;
  }
  else if (token == "isready")
    std::cout << "readyok" << std::endl;
  else if (token == "ucinewgame")
    getBoard().parseFenString(START_POS_FEN);
  else if (token == "position")
    uciPosition(ss);
  else if (token == "go")
    uciGo(ss);
  else if (token == "stop")
  {
    getBoard().endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    getBoard().stopSearch = true;
    std::vector<Move> moves = getBoard().getPV();
    std::cout << "bestmove " << toUciString(moves[0]) << std::endl;
  }
  // following is part of the UCI format but not jet implemented by NoPy++
  else if (token == "setoption")
    setOption(ss);
  // else if (token == "ponderhit")
  // else if (token == "register")
  // some custom commands
  else if (token == "print")
    getBoard().prettyPrint();
  else if (token == "fen")
    std::cout << "fen " << getBoard().toFenString() << std::endl;
  else if (token == "legalmoves")
    uciLegalMoves();
  else if (token == "move")
    uciMove(ss);
  else if (token == "unmakemove")
    uciUnmakeMove();
  else if (token == "perft")
    uciPerft(ss);

  return token;
}

void uciLoop()
{
  std::cout << "uciLoop starting" << std::endl;
  std::string command, token;
  getBoard();

  do
  {
    // getline or quit on EOF
    if (!getline(cin, command))
      command = "quit";
    // process instructions and get command in case the user quits
    token = uciProcessCommand(command);
  } while (token != "quit");
  std::cout << "uciLoop finished" << std::endl;
}