#include <stdio.h>
#include "../engine/constants.h"
#include "../engine/board.h"
#include "../engine/uci.h"

int main() {
  initConstants();
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void processCommand(char* command) {
  uciProcessCommand(command);
}

#ifdef __cplusplus
}
#endif