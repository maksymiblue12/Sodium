#!/bin/sh
clang++ -std=c++26 -Wall src/main.cpp -o sodium
echo "Built ./sodium"
./sodium test.na
./test
echo "Exit Code:" $?