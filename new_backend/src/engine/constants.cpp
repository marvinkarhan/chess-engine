#include "constants.h"
#include "moveHelper.h"


void initArrRectangular() {
    Direction leftUp[1] = {LEFT_UP};
    Direction leftDown[1] = {LEFT_DOWN};
    Direction rightUp[1] = {RIGHT_UP};
    Direction rightDown[1] = {RIGHT_DOWN};
    Direction left[1] = {LEFT};
    Direction right[1] = {RIGHT};
    Direction up[1] = {UP};
    Direction down[1] = {DOWN};
    for (int i = 0; i <= 63; i++) {
        BB bb_1 = SQUARE_BBS[i];
        for (int j = 0; j <= 63; j++) {
            BB bb_2 = SQUARE_BBS[j];
            if (BISHOP_MOVE_BBS[i] & bb_2) {
                BB way_bb_1 = traverse_bb(bb_1, leftUp, 0, bb_2);
                BB way_bb_2 = traverse_bb(bb_1, leftDown, 0, bb_2);
                BB way_bb_3 = traverse_bb(bb_1, rightUp, 0, bb_2);
                BB way_bb_4 = traverse_bb(bb_1, rightDown, 0, bb_2);
                if (way_bb_1 & bb_2)
                        REY_BBS[i][j] = way_bb_1 & ~bb_2;
                else if (way_bb_2 & bb_2)
                        REY_BBS[i][j] = way_bb_2 & ~bb_2;
                else if (way_bb_3 & bb_2)
                        REY_BBS[i][j] = way_bb_3 & ~bb_2;
                else if (way_bb_4 & bb_2)
                        REY_BBS[i][j] = way_bb_4 & ~bb_2;
            } else if (HORIZONTAL_MOVE_BBS[i] & bb_2) {
                BB way_bb_1 = traverse_bb(bb_1, left, 0, bb_2);
                BB way_bb_2 = traverse_bb(bb_1, right, 0, bb_2);
                if (way_bb_1 & bb_2)
                    REY_BBS[i][j] = way_bb_1 & ~bb_2;
                else if (way_bb_2 & bb_2)
                    REY_BBS[i][j] = way_bb_2 & ~bb_2;
            } else if (VERTICAL_MOVE_BBS[i] & bb_2) {
                BB way_bb_1 = traverse_bb(bb_1, up, 0, bb_2);
                BB way_bb_2 = traverse_bb(bb_1, down, 0, bb_2);
                if (way_bb_1 & bb_2)
                    REY_BBS[i][j] = way_bb_1 & ~bb_2;
                else if (way_bb_2 & bb_2)
                    REY_BBS[i][j] = way_bb_2 & ~bb_2;
            } else
                REY_BBS[i][j] = 0;
        }
    }
}

void initArrRectangularLines() {
    for (int i = 0; i <= 63; i++) {
        BB bb_1 = SQUARE_BBS[i];
        for (int j = 0; j <= 63; j++) {
            BB bb_2 = SQUARE_BBS[j];
            if (BISHOP_MOVE_BBS[i] & bb_2)
                LINE_BBS[i][j] = BISHOP_MOVE_BBS[i] & BISHOP_MOVE_BBS[j] | bb_1 | bb_2;
            else if (HORIZONTAL_MOVE_BBS[i] & bb_2)
                LINE_BBS[i][j] = HORIZONTAL_MOVE_BBS[i] & HORIZONTAL_MOVE_BBS[j] | bb_1 | bb_2;
            else if (VERTICAL_MOVE_BBS[i] & bb_2)
                LINE_BBS[i][j] = VERTICAL_MOVE_BBS[i] & VERTICAL_MOVE_BBS[j] | bb_1 | bb_2;
            else
                LINE_BBS[i][j] = 0;
        }
    }
}

void initConstants() {
    for (int i = 0; i <= 63; i++) {
        SQUARE_BBS[i] = 4;
        printf("%d",SQUARE_BBS[i]);

    }
    
    for (int i = 0; i <= 63; i++)
        HORIZONTAL_MOVE_BBS[i] =  traverse_bb(SQUARE_BBS[i], HORIZONTAL_MOVES, 0, 0);

    for (int i = 0; i <= 63; i++)
        VERTICAL_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], VERTICAL_MOVES, 0, 0);

    for (int i = 0; i <= 63; i++)
        ROOK_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], HORIZONTAL_VERTICAL_MOVES, 0, 0);

    for (int i = 0; i <= 63; i++)
        BISHOP_MOVE_BBS[i] = traverse_bb(SQUARE_BBS[i], DIAGONALS_MOVES, 0, 0);

    for (int i = 0; i <= 63; i++)
        QUEEN_MOVE_BBS[i] = ROOK_MOVE_BBS[i] | BISHOP_MOVE_BBS[i];

    for (int i = 0; i <= 63; i++)
        KNIGHT_MOVE_BBS[i] = knight_moves(SQUARE_BBS[i], 0);

    for (int i = 0; i <= 63; i++)
        KING_MOVES_BBS[i] = king_moves(SQUARE_BBS[i], 0);

    for (int i = 0; i <= 63; i++) {
        for (int j = 0; j <= 1; j++)
            PAWN_ATTACKS_BBS[i][j] = pawn_attacks(SQUARE_BBS[i], j, 0);
    }

    initArrRectangular();

    initArrRectangularLines();
}