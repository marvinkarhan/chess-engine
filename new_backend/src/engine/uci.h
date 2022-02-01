#include <sstream>
#include <string>
#include "board.h"
#include "movehelper.h"

void uciGo(std::istringstream &ss);
void uciPosition(std::istringstream &ss);
void uciLegalMoves();
void uciMove(std::istringstream &ss);
void uciUnmakeMove();
std::string uciProcessCommand(std::string command);
void uciLoop();