#include "move.h"
#include <string>

int originSquare;
int targetSquare;
char promotion;
MoveType type;

Move::Move()
{
    originSquare = 0;
    targetSquare = 0;
    type = NORMAL;
}

Move::Move(int originSquare, int targetSquare, MoveType type /*=NORMAL*/, char promotion /*=0*/)
{
    originSquare = 0;
    targetSquare = 0;
    promotion = promotion;
    type = type;
}

bool Move::operator==(const Move &other)
{
    return (originSquare == other.originSquare &&
            targetSquare == other.targetSquare &&
            promotion == other.promotion);
}

std::string Move::to_uci_string()
{
    return SQUARE_TO_ALGEBRAIC[originSquare] + SQUARE_TO_ALGEBRAIC[targetSquare] + (promotion ? "" + promotion : "");
}
