#pragma once
#include <string>
#include <iostream>

using namespace std;

namespace environment
{
  const string __PROJECT_NAME__ = "new_backend";

  inline string getProjectDirectory()
  {
    string file = __FILE__;
    int pos = file.find(__PROJECT_NAME__);
    string split = file.substr(0, pos);
    return split + __PROJECT_NAME__ + "/";
  }

  const string __OPENING_JSON__ = getProjectDirectory() + "opening-extractor/output/openings.json";
}
