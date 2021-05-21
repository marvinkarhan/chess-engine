# websocket server

Simple **multithreaded** websocket for the chess engine backend. Built with oat++ (AKA oatpp) web framework.

See more:
- [Oat++ Website](https://oatpp.io/)

## Overview

### Project layout

```
|- CMakeLists.txt                         // projects CMakeLists.txt
|- src/
|    |- oatpp                             // The oatpp engine for code intellisense
     |- oatpp-websocket                   // The oatpp-websocket for code intellisense
     |- engine                            // The chess engine
|    |- controller/
|    |    |
|    |    |- MyController.hpp             // ApiController with websocket endpoint
|    |
|    |- websocket/
|    |    |
|    |    |- WSListener.hpp               // WebSocket listeners are defined here
|    |
|    |- AppComponent.hpp                  // Application config. 
|    |- App.cpp                           // main() is here
|
|- utility/install-oatpp-modules.sh       // utility script to install required oatpp-modules.  
```

## Build and Run

### Using CMake

**Requires:** [oatpp](https://github.com/oatpp/oatpp), and [oatpp-websocket](https://github.com/oatpp/oatpp-websocket) 
modules installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

After all dependencies satisfied:

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./websocket-server-exe       # - run application.
```
