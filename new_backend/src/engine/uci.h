#include <sstream>

#include "board.h"
#include "moveHelper.h"


void uciGo(Board &board, PVariation &pVariation);
void uciPosition(Board &board, std::istringstream &ss);
void uciLoop();