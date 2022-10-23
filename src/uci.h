#pragma once
#include <sstream>
#include <string>
#include "constants.h"

void uciGo(std::istringstream &ss);
void uciPosition(std::istringstream &ss);
void uciLegalMoves();
void uciMove(std::istringstream &ss);
std::string uciProcessCommand(std::string command);
void uciLoop();

#ifndef WASM
int main(int argc, char *argv[])
{
  initConstants();
  // run single command
  if (argc > 1) {
    std::string input = "";
    for (int i = 1; i < argc; i++)
    {
      input += argv[i];
      input += " ";
    }
    uciProcessCommand(input);
    return 0;
  }
  uciLoop();
  return 0;
}
#endif