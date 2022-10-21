#include <stdio.h>
#include "../constants.h"
#include "../board.h"
#include "../uci.h"

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