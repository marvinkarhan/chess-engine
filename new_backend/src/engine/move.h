#include "constants.h"

enum MoveType {
    NORMAL,
    PROMOTION,
    EN_PASSANT,
    CASTLING
};

class Move{
    public:
        int originSquare;
        int targetSquare;
        char promotion;
        MoveType type;
        std::string to_uci_string();
};