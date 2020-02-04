#!/bin/bash
g++ -std=c++17 -DTHREADS -DTESTING main.cpp
nice -n -20 ./a.out 25 1
g++ -std=c++17 -DPTHREAD -DTESTING main.cpp
nice -n -20 ./a.out 25 1
g++ -std=c++17 -DASYNC  -DTESTING main.cpp
nice -n -20 ./a.out 25 1
exit 0
