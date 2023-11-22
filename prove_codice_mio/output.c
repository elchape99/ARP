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
        printf("valore argv: %s\n", argv[i]);
        fflush(stdout);
        
        pipe_fd[i-1] = atoi(argv[i]);
        printf("valore passato da atoi: %d\n", pipe_fd[i-1]);
        fflush(stdout);
    }

    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(pipe_fd[0], &rfds);
    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    printf("ma sta cazzo di finestra funziona, dio **************\n");
    fflush(stdout);
    

    // char ch;
    // int controllo;

    // do
    // {
    //     close(pipe_fd[1]);
    //     retval = select(1, &rfds, NULL, NULL, &tv);
    // /* Don’t rely on the value of tv now! */

    //     if(retval == -1){
    //         perror("errore select");
    //     }else if(retval){
    //         printf("presenti dati sulla pipe\n");
    //         fflush(stdout);
    //         if ((controllo = read(pipe_fd[0], &ch, 1))<0){
    //             perror("errore read");
    //         }else{
    //             printf("byte letti: %d, valore ricevuto: %c\n", controllo, ch);
    //             fflush(stdout);
    //         }
    //     }else{
    //         continue;
    //     }

    // } while (TRUE);

    int controllo;
    char ch;
    close(pipe_fd[1]);

    do{
        controllo = read(pipe_fd[0], &ch, 1);
        if (controllo < 0){
            perror("errore lettura");
        }else if(controllo == 0){
            printf("errore nessun valore letto\n");
            fflush(stdout);
        }else if(controllo > 0){
            printf("valore letto dalla pipe: %c\n", ch);
            fflush(stdout);
        }
    }while(ch != 'q');
    
    return 0;
}