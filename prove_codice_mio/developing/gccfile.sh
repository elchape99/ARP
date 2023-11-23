#!/bin/bash

gcc father.c -o father
gcc input.c -lncurses -o input
gcc output.c -o output

gcc input_noscr.c -lncurses -o input_noscr
gcc drone.c -o drone