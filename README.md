# Optimization-Solver
Code for the optimization model to find maximize pilot preferences.

This code relies on the Gurobi for C++ solver, which is included as "gurobi_c++.h".

The variables in the code shown (the vectors and the initial integers) are not initialized for use, they change every time that the code is run and require more code to take them in from a file and to prepare them for the program. Similarly, the ending of the program which prints out the results needs to be reformatted to write the results to a file which can then be sent to the server.
