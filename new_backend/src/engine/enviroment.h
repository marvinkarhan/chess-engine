#pragma once
#include <string>

using namespace std;

namespace enviroment
{
  const string __PROJECT_NAME__ = "chess-engine";

  inline string getProjectDirectory()
  {
    string file = __FILE__;
    int pos = ((string)__FILE__).find(__PROJECT_NAME__);
    string split = file.substr(0, pos);
    return split + __PROJECT_NAME__ + "/";
  }

  const string __OPENING_JSON__ = getProjectDirectory() + "new_backend/opening-extractor/output/openings.json";
}
