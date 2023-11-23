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
    for(int i=1; i<argc; i++){
        pipe_fd[i-1] = atoi(argv[i]); // converto gli argv da stringhe in integer
    }
    close(pipe_fd[0]);

    char input_ch='\0';
    int wrt_control;

    initscr();raw();cbreak();keypad(stdscr, TRUE);

    do{
        input_ch = getch();
        if(input_ch != '\0'){
            wrt_control = write(pipe_fd[1], &input_ch, 1);
            if(wrt_control < 0){
                printw("errore write pipe\n");
            }
        }
    }while(input_ch != 'q');

    endwin();
}