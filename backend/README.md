# chess_engine

chess_engine is a in Python written Chess Engine backend application.

## Installation

Run the powershell script that automatically installs all required packages and runs the flask server locally.

```bash
.\start.ps1
```

## Compile with cython

Cythonize the Project.

```bash
python setup.py build_ext --inplace
```

Run main.

```bash
python .\main
```