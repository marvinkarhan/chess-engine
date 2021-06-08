cmake_minimum_required(VERSION 3.1)

set(project_name chess-server) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")
# set(CMAKE_CXX_FLAGS "-g") #debugging
  # set(CMAKE_CXX_FLAGS "-pg")

include_directories(src)


file(GLOB ENGINE_SRC
     "src/engine/*.hpp"
     "src/engine/*.cpp"
     "src/engine/*.h"
)
add_library(${project_name}-lib
        src/AppComponent.hpp
        src/controller/
        src/websocket/WSListener.cpp
        src/websocket/WSListener.hpp
        ${ENGINE_SRC}
)

## link libs

find_package(nlohmann_json      3.2.0 REQUIRED PATHS src/vendor/)
find_package(oatpp              1.2.5 REQUIRED PATHS src/vendor/)
find_package(oatpp-websocket    1.2.5 REQUIRED  PATHS src/vendor/)

target_link_libraries(${project_name}-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
        PUBLIC oatpp::oatpp-websocket
        PRIVATE nlohmann_json::nlohmann_json
)

## add executables

## web server exe
add_executable(${project_name}-exe
        src/App.cpp
)

target_link_libraries(${project_name}-exe ${project_name}-lib)
add_dependencies(${project_name}-exe ${project_name}-lib)


## Test exe
add_executable(${project_name}-test
        test/tests.cpp
        test/WSTest.cpp
        test/WSTest.hpp
)
target_link_libraries(${project_name}-test ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)


## Engine exe
add_executable(${project_name}-engine-exe
        src/MainEngine.cpp
)

target_link_libraries(${project_name}-engine-exe ${project_name}-lib)
add_dependencies(${project_name}-engine-exe ${project_name}-lib)


## UCI exe
add_executable(${project_name}-uci-exe
        src/MainUCI.cpp
)

target_link_libraries(${project_name}-uci-exe ${project_name}-lib)
add_dependencies(${project_name}-uci-exe ${project_name}-lib)


set_target_properties(${project_name}-lib ${project_name}-exe ${project_name}-test ${project_name}-engine-exe ${project_name}-uci-exe PROPERTIES
# set_target_properties(${project_name}-lib ${project_name}-engine-exe PROPERTIES
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        LINKER_LANGUAGE CXX
)

enable_testing()
add_test(tests ${project_name}-test)
