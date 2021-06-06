#include <iostream>
#include <chrono>
#include <string>

#include "engine/constants.h"
#include "engine/uci.h"

int main(int argc, char *argv[])
{
  initConstants();

  uciLoop();
  return 0;
}