#include "move.h"
#include <string>

class Move {

    int originSquare;
    int targetSquare;
    char promotion;
    MoveType type;

    Move() {
        originSquare = 0;
        targetSquare= 0;
        type = NORMAL;
    }

    Move(int originSquare, int targetSquare, MoveType type = NORMAL, char promotion = 0) {
        originSquare = 0;
        targetSquare= 0;
        promotion = promotion;
        type = type;
    }

    bool operator==(const Move& other) {
        return (originSquare == other.originSquare &&
                targetSquare == other.targetSquare &&
                promotion == other.promotion);
    }

    std::string to_uci_string() {
        return SQUARE_TO_ALGEBRAIC[originSquare] + SQUARE_TO_ALGEBRAIC[targetSquare] + (promotion ? "" + promotion : "");
    }
};