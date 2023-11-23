#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <string.h> 
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>


int main(int argc, char *argv[]){
    int pipe_fd[2];

    for (int i = 1; i < argc; i++){
        pipe_fd[i-1] = atoi(argv[i]);
    }
    close(pipe_fd[1]); // chiudo scrittura

    int retval;
    fd_set rfds;
    struct timeval  tv;

    FD_ZERO(&rfds);
    FD_SET(pipe_fd[0], &rfds);

    int read_control;
    char read_ch;

    while(TRUE){
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval < 0){
            perror("errore su select: ");
        }else if(retval == 0){
            printf("no data to read\n");
            fflush(stdout);
        }else{
            read_control = read(pipe_fd[0], &read_ch, 1);
            if (read_control < 0){
                printf("errore lettura pipe\n");
                fflush(stdout);
            }else{
                printf("lettura avvenuta, conteggio byte: %d\nvalore letto: %c", read_control, read_ch);
                fflush(stdout);
            }
        }
        
    }

        
    
}