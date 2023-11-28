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
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char *argv[]){

    int pipe_fd[2];
    if((pipe(pipe_fd)) < 0){
        perror("errore creazione pipe");
    }

    char str_pipe_fd[2][20];
    for (int i = 0; i < 2; i++)
    {
        printf("valore pipe fd: %d\n", pipe_fd[i]);
        fflush(stdout);
        
        sprintf(str_pipe_fd[i], "%d", pipe_fd[i]);
    }


    
    char * arglist1[] = {"konsole", "-e","./sender", str_pipe_fd[0], str_pipe_fd[1], NULL};
    char * arglist2[] = {"konsole", "-e","./reciver", str_pipe_fd[0], str_pipe_fd[1], NULL};


    int pid1, pid2;
    pid1 = fork();
    if (pid1 < 0){
        printf("fork error 1");
    }else if(pid1 == 0){
        execvp("konsole", arglist1);
    }

    if (pid1 != 0){
        pid2 = fork();
        if (pid2 < 0){
            printf("fork error 2");
        }else if(pid2 == 0){
            execvp("konsole", arglist2);
        }
    }

    if (pid1 != 0 && pid2 != 0){ // mi trovo nel parent process
        char inp_ch;
        if ((inp_ch = getchar()) == 'q'){
            printf("sendinng signal to child\n");
            fflush(stdout);
            
            printf("sending to sender: %d\n", pid1);
            fflush(stdout);
            if(kill(pid1, SIGTERM) <0){
                perror("errore kill 1");
            }

            sleep(3);

            printf("sending to reciver: %d\n", pid2);
            fflush(stdout);
            if(kill(pid2, SIGTERM) <0){
                perror("errore kill 2");
            }

        }
    }


    return 0;
}
