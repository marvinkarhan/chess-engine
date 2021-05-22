cmake_minimum_required(VERSION 3.10)

set(project_name unit-test) ## rename your project here
project(${project_name})


file(GLOB CATCH_SRC
     "lib/catch/*.hpp"
     "lib/catch/*.cpp"
)
file(GLOB ENGINE_SRC
     "../../src/engine/*.hpp"
     "../../src/engine/*.cpp"
     "../../src/engine/*.h"
)
file(GLOB TEST_SRC
     "./*Test.cpp"
     "./*Test.h"
     "./*Test.hpp"
)

add_executable(${project_name} ${CATCH_SRC} ${ENGINE_SRC} ${TEST_SRC})


