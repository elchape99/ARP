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
    close(pipe_fd[1]); // chiudo scrittura
    char ch;

    int retVal_sel, retVal_read;
    fd_set read_fd;
    struct timeval sel_time;

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = singnalHandlerFunction;

    if(sigaction(SIGTERM, &sa_usr1, NULL) == -1){
        perror("errore sigaction: ");
    }

    while(TRUE){
        // reinizializzo ad ogni ciclo per evitare undef value per retVal_sel == 0
        FD_ZERO(&read_fd);
        FD_SET(pipe_fd[0], &read_fd);

        sel_time.tv_sec = 1.0;
        sel_time.tv_usec = 0.0;

        retVal_sel = select(pipe_fd[0]+1, &read_fd,  NULL, NULL, &sel_time);
        
        perror("controllo forzato della select: ");
        printf("\n");
        fflush(stdout);
        
        if(retVal_sel == 0){
            printf("nessun dato sulla pipe\n");
            fflush(stdout);
        }else{
            retVal_read = read(pipe_fd[0], &ch, 1);
            if (retVal_read < 0){
                perror("errore lettura: ");
            }else{
                printf("controllo byte letti: %d, controllo carattere letto: %c\n", retVal_read, ch);
                fflush(stdout);
            }
        }
    }
    
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
