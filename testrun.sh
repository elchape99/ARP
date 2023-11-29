cd library/
gcc -c window.c 
# gcc -shared -o  libwindow.so window.o
cd ..
# Compile the main files
gcc -c inputcou.c
gcc -o inputc inputcou.o library/window.o -lncurses