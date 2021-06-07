#include <sstream>

#include "board.h"
#include "moveHelper.h"


void uciGo(Board &board);
void uciPosition(Board &board, std::istringstream &ss);
void uciLoop();