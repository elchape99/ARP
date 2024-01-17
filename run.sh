#!/bin/bash

# Crea la cartella build_process se non esiste
mkdir -p bin
mkdir -p lib

# Compila la libreria arplib
gcc -c -o lib/arplib.o src/arplib.c
ar rcs lib/libarplib.a lib/arplib.o

# Compila i processi con le relative dipendenze
gcc -o bin/master src/master.c -L./lib -larplib
gcc -o bin/server src/server.c -L./lib -larplib -lncurses -lm
gcc -o bin/input src/input.c -L./lib -larplib -lncurses
gcc -o bin/drone src/drone.c -L./lib -larplib
gcc -o bin/target src/target.c -L./lib -larplib
gcc -o bin/obstacle src/obstacle.c -L./lib -larplib
gcc -o bin/wd src/wd.c 

# Cambia la directory in build_process
cd bin

# Esegui il processo master
./master
