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
#include<sys/wait.h>
#include <signal.h>


int main(int argc, char *argv[]){

    int pipe_go[2];
    if((pipe(pipe_go)) < 0){
        perror("errore creazione pipe");
    }

    char str_pipe_fd[2][20];
    for (int i = 0; i < 2; i++)
    {
        sprintf(str_pipe_fd[i], "%d", pipe_go[i]);
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

    if (pid1 != 0 && pid2 != 0){
        char pidstring[2][10];
        sprintf(pidstring[0], "%d", pid1);
        sprintf(pidstring[1], "%d", pid2);

        char * arglist3[] = {"kill", "-s","SIGKILL",  pidstring[0],NULL};
        char * arglist4[] = {"kill", "-s","SIGKILL",  pidstring[1],NULL};

        int sig_cont;
        if (getc(stdin) == 'q'){
            execvp("kill", arglist3);
            execvp("kill", arglist4);
        }
        pid_t temp;
        while((temp = wait(NULL)) != -1){
            sleep(1);
        } 

    }
    
    

    return 0;
}