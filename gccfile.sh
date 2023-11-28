#!/bin/bash

gcc drone.c -o drone
gcc input.c -lncurses -o input
gcc master.c -o master
gcc pippo.c -o pippo
gcc server.c -lncurses -o server
gcc spawn.c -o spawn

