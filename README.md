# NoPy++
```__    __            _______                                
|  \  |  \          |       \               __        __    
| $$\ | $$  ______  | $$$$$$$\ __    __    |  \      |  \   
| $$$\| $$ /      \ | $$__/ $$|  \  |  \ __| $$__  __| $$__ 
| $$$$\ $$|  $$$$$$\| $$    $$| $$  | $$|    $$  \|    $$  \
| $$\$$ $$| $$  | $$| $$$$$$$ | $$  | $$ \$$$$$$$$ \$$$$$$$$
| $$ \$$$$| $$__/ $$| $$      | $$__/ $$   | $$      | $$   
| $$  \$$$ \$$    $$| $$       \$$    $$    \$$       \$$   
 \$$   \$$  \$$$$$$  \$$       _\$$$$$$$                    
                              |  \__| $$                    
                               \$$    $$                    
                                \$$$$$$               
```

## History

This Chess engine was started as a small project in an university class. Later a NNUE Evaluation was added as part of a bachelor thesis. The engine was called NoPy++ because the project started out as a Python engine, but quickly transitioned to C++ for obvious reasons. We're currently still looking for a new name.

## Frontend

Provides a user interface for the WebAssembly chess engine, communication over UCI.

## Backend

The backend consists of a UCI compatible chess engine, that can be compiled to Windows, Linux and WebAssembly. Currently the WebAssembly compilation is not working because recently added SIMD instructions are not jet supported for the WebAssembly target. Previous versions without NNUE can be used and combined with the frontend.

## Elo

Current playing strength

```
STC (10+0.1s):  2670.1 +- 6.8
LTC (60+1s)  :  2685.7 +- 6.3
```
## Compile

Compile the engine using make on Windows or Linux

```
cd ./src
make
./uci-engine
```

## UCI settings

A list of settings that can be set via `setoption name <id> [value <x>]`:

* UseNNUE\
  toggle between NNUE and classical evaluation. Setting the value to "true" activates the NNUE evaluation

* EvalFile\
  sets the NNUE Network used via a file. Only compatible formats work, likely only those generated for this engine
## Custom commands

* print\
  prints the current position

* fen\
  prints the fen of the current position

* legalmoves\
  prints all legal moves

* move \<list of moves\>\
  plays moves specified in a list consecutively (like position ... moves does)

* unmakemove\
  unmakes the last move

* perft \<depth\> [\<fen\>]\
  runs a perft for a specified depth on either the current position or specified one

## Features

### Board Representation

* Bitboards

### Search

* Iterative Deepening
* Aspiration Windows
* Negamax Search
* Transposition Table
* Move Ordering
  * Killer Heuristic
  * MVV/LVA
  * SEE
* Quiescence Search

### Evaluation

* NNUE
  * HalfKP Input -> 2x256 -> 32 -> 32 -> 1
  * incremental updates
  * SIMD intrinsics up to AVX512
* Classical
  * Material
    * Point Values
    * Bishop Pair bonus
    * Knight Pair penalty
    * Rook Pair penalty
    * No Pawn penalty
  * Mobility