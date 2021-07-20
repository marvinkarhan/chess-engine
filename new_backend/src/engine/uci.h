#include <sstream>

#include "board.h"
#include "movehelper.h"


void uciGo(Board &board);
void uciPosition(Board &board, std::istringstream &ss);
void uciLoop();