export enum ChessApiEvents {
    BOARD_INITIALIZED = "board_initialized",
    NEW_BOARD_MOVES = "new_board_moves",
    NEW_BOARD_INFORMATION = 'new_board_info',

}
export enum ChessApiEmits {
    NEW_BOARD = "new_board",
    GET_BOARD_MOVES = "get_board_moves",
    MAKE_MOVE = "make_move",
    UNMAKE_MOVE = "unmake_move",
}
