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

void singnalHandlerFunction(int signum);
 

int main(int argc, char *argv[]){
    int pipe_fd[2];

    for (int i = 1; i < argc; i++){
        pipe_fd[i-1] = atoi(argv[i]);
        printf("valore pipe fd: %d\n", pipe_fd[i-1]);
        fflush(stdout);
    }
    close(pipe_fd[0]);

    int wait_time;
    srand(time(NULL));

    int write_control;

    char ch = 'K';

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = singnalHandlerFunction;

    if(sigaction(SIGTERM, &sa_usr1, NULL) == -1){
        perror("errore sigaction: ");
    }

    while (TRUE)
    {
        wait_time = rand()% 5000;
        usleep(wait_time);

        write_control = write(pipe_fd[1], &ch, 1);
        if (write_control < 0){
            perror("errore write: ");
            fflush(stdout);
        }else{
            printf("tutto ok, scritto byte: ( %d ), carattere scritto: ( %c )", write_control, ch);
            fflush(stdout);
        }
        printf("aspetto per scrivere: ( %d+1sec )\n", wait_time);
        fflush(stdout);
        sleep(1);
    }

    return 0;
}

void singnalHandlerFunction(int signum){
    if (signum == SIGTERM){
        printf("Ricevuto segnale di stop\n");
        fflush(stdout);
        sleep(3);

        exit(EXIT_SUCCESS);
    }else{
        printf("Segnale diverso da SIGKILL, continuo esecuzione\n");
        fflush(stdout);
    }
}