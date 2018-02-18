#!/bin/bash

set -e

# compile the program

cc -std=c99 -Wall main.c lib/mpc.c -ledit -lm -o slip

# run the lisp

./slip
