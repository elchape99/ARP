# compile library
cd library/
gcc -c window.c 
# gcc -shared -o  libwindow.so window.o
cd ..
# Compile the main files
gcc -c description.c 

# compile the master
gcc master.c -o master

# compile the server
gcc server.c -o server

# compile the drone
gcc drone.c -o drone

# compile the input
gcc input.c -o input

# Compile the descriptions files
gcc -o des description.o library/window.o -lncurses

# Run the master
./master
