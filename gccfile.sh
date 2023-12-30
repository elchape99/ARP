#!/bin/bash

mkdir -p build_process

gcc -o build_process/master master.c
gcc -o build_process/server server.c -lncurses
gcc -o build_process/input input.c -lncurses
gcc -o build_process/drone drone.c -lncurses
gcc -o build_process/wd wd.c

cd build_process
./master


