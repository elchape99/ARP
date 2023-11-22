#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
//"--fullscreen",
int main(){
    char * arglist[] = {"konsole", "-e" ,"./input", "&", NULL};

    int pidc;
    pidc = fork();
    if (pidc == -1){
        printf("fork error");
    }

    if (pidc == 0){
        execvp("konsole", arglist);
    }

    return 0;
}