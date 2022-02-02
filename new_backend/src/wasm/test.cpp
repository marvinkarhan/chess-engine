#include <stdio.h>
#include <emscripten/emscripten.h>
#include "../engine/constants.h"
#include "../engine/board.h"
#include "../engine/uci.h"

int main() {
  initConstants();

  // Board board("rnbqkb1r/ppNppppp/5n2/8/8/1P6/P1PPPPPP/R1BQKBNR b KQkq - 0 4");
  // Board board(KIWI_PETE_POS_FEN);
  Board board;
  board.prettyPrint();
  uciProcessCommand("go");
  printf("HALLO WELT!\n");
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void processCommand(char* command) {
  uciProcessCommand(command);
}

void testCommand() {
  printf("Hallo\n");
}

#ifdef __cplusplus
}
#endif