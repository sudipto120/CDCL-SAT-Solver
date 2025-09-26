# CDCL-SAT-Solver
To run the program use $ g++ -std=c++17 -o sudoku_solver main.cpp solver.cpp and then $ ./sudoku_solver

Code is modular, and you can figure out where a specific function is defined by searching in declarations.

A CDCL based SAT solver is implemented in solver.h and solver.cpp,
testcases of sudoku puzzles are used to check the correctness and efficiency of the solver.
