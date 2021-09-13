#include <sstream>
#include "board.h"
#include "movehelper.h"


void uciGo(Board &board, std::istringstream &ss);
void uciPosition(Board &board, std::istringstream &ss);
void uciLegalMoves(Board &board);
void uciMove(Board &board, std::istringstream &ss);
void uciUnmakeMove(Board &board);
void uciLoop();