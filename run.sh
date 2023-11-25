# compile library
cd library/
gcc -c window.c 
# gcc -shared -o  libwindow.so window.o
cd ..
# Compile the main files
gcc -c description.c 

# Compile the main files
gcc -o des description.o library/window.o -lncurses

# Link all the object files
# gcc -o des description.o library/window.o

