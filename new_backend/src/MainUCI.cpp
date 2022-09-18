#include <iostream>
#include <chrono>
#include <string>

#include "engine/constants.h"
#include "engine/uci.h"

int main()
{
  initConstants();

  uciLoop();
  return 0;
}