# Optimization-Solver

The primary purpose of the code is to optimize a group of individuals' preferences to create the most optimal assignement of individuals so that everyone is as satisfied as possible with the people they are working with, the project/group they are working on and the role they are performing in that group.

The code is based on the paper which is a part of the repository called "Tractable Model for the Optimization of Group Assignments Based on Preferences Over Positions and Other Individuals". This paper is written by the creator of this repository and is an original concept.

This code relies on the Gurobi for C++ solver, which is included as "gurobi_c++.h".

# Logistics

This code has an input and output header file which contain the code for the ways that the input for the program, which are the groups, the roles within those groups and the individuals that are intended to populate those groups. These are inputted in the format of a .json file and the output is outputted in the form of a .json list as well so that this program can be seamlessly put into the middle of a python program if the input and output formats are respected.



