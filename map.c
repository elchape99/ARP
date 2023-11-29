#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>
#include "library/window.h"


int main (int argc, char* argv[])
{
    int pipe[argc];
    int i;

    for (i = 0; i < argc; i++) {
        pipe[i] = atoi(argv[i]); // converts from the string to the integer
    }
    close(pipe[1]);

    // From now I have to read all the data of the structure of poition  
    //while (x != -1 && y != -1) {
        // computa posizione avendo posizione assoluta e trasformandola in relativa
        // getmax(win princ
        // getmax(win map)
        // pos = x/maxprinc *maxmap
    // passo da server posizione e dimensione della mappa
    // fscanf(stdin, "%d", &ch);
    //devo capire come muovere il drone sulla mappa
    // }


    fclose(stdin);
    return 0;
}