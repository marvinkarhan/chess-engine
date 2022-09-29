# Chess engine

## History
---
This Chess engine was started as a small Project in a University class. Later a NNUE Evaluation was added as Part of a Bachelorthesis. The engine was called NoPy++ because the project started out as a pyhton engine, but quickly transitioned to C++ for obvious reasons. We're currently still looking for a new name.

## Backend
---
The backend consists of a UCI compatible chess engine, that can be compiled to Windows, Linux and WebAssembly. Currently the WebAssembly compilation is not working because recently added SIMD instructions are not jet supported for the WebAssembly target. Previous versions without NNUE can be used and combined with the frontend.

## Elo
---
Curren playing strength

```
STC (10+0.1s):  2670.1 +- 6.8
LTC (60+1s)  :  2685.7 +- 6.3
```

## Frontend
---
Provides a user interface for the WebAssambly chess engine comunication over UCI.