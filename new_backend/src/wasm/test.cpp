#include <stdio.h>
#include <emscripten/emscripten.h>
#include "../engine/movepicker.h"
#include "../engine/constants.h"
#include "../engine//move.h"
#include "../engine/board.h"
#include "../engine/movehelper.h"

int main() {
  initConstants();

  // Board board("rnbqkb1r/ppNppppp/5n2/8/8/1P6/P1PPPPPP/R1BQKBNR b KQkq - 0 4");
  // Board board(KIWI_PETE_POS_FEN);
  Board board;
  board.prettyPrint();
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

EMSCRIPTEN_KEEPALIVE void myFunction(int argc, char ** argv) {
    printf("MyFunction Called\n");
}

#ifdef __cplusplus
}
#endif