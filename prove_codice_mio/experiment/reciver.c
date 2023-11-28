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

#define SIGH_MSG "segnale kill ricevuto\n"

void signalHandler(int signum);


int main(int argc, char *argv[]){
    int pipe_fd[2];

    for (int i = 1; i < argc; i++){
        pipe_fd[i-1] = atoi(argv[i]);
        printf("valore pipe fd: %d\n", pipe_fd[i-1]);
        fflush(stdout);
    }
    close(pipe_fd[1]); // chiudo scrittura

    int retval;
    fd_set rfds;
    struct timeval  tv;

    int retval1;
    fd_set rfds_ps;
    struct timespec tv_ps;

    FD_ZERO(&rfds_ps);
    FD_SET(0, &rfds_ps);

    tv_ps.tv_sec = 0;
    tv_ps.tv_nsec = 0;

    int read_control;
    char read_ch;

    struct sigaction kill_hand;
    memset(&kill_hand, 0, sizeof(kill_hand));
    kill_hand.sa_handler = signalHandler;
    kill_hand.sa_flags = SA_RESTART;

    

    while((retval1 = pselect(1, &rfds_ps, NULL, NULL, &tv_ps, NULL)) >= 0){
        printf("valore ritorno pselect: %d\n", retval1);
        fflush(stdout);
        
        FD_ZERO(&rfds);
        FD_SET(pipe_fd[0], &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        retval = select(pipe_fd[0] + 1, &rfds, NULL, NULL, &tv);
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
                printf("lettura avvenuta, conteggio byte: %d, valore letto: %c\n", read_control, read_ch);
                fflush(stdout);
            }
        }


        FD_ZERO(&rfds_ps);
        FD_SET(0, &rfds_ps);

        tv_ps.tv_sec = 0;
        tv_ps.tv_nsec = 0;
    }
    if (retval1 < 0){
        perror("errore con pselect");
    }else if(retval == EINTR){
        if(sigaction(retval1, &kill_hand, NULL)<0){
                perror("errore ricezione segnale");               
            }
    }
        
    
}

void signalHandler(int signum){
    if (signum == SIGKILL){
        write(STDERR_FILENO, SIGH_MSG, sizeof(SIGH_MSG));
        exit(EXIT_SUCCESS);
    }else{
        write(STDERR_FILENO, "errore ricezione, o messaggio sbagliato", sizeof("errore ricezione, o messaggio sbagliato"));
    }
}
