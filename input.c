#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <unistd.h> 
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <ncurses.h>

void sigusr1Handler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGUSR1){
        /*send a signal SIGUSR2 to watchdog */
        kill(info->si_pid, SIGUSR2);
    }
}

// dichiarazione delle funzioni utilizzate per le finestre 
WINDOW *create_new_window(int row, int col, int ystart, int xstart);
void case_execution(char input_char, int PRy, int PRx, WINDOW *print_pointer, WINDOW *color_pointer, int write_fd, int read_fd);


int main(int argc, char *argv[]) {

    int pipe_fd[2]; // recupero dall'argv i file descriptor delle pipe
    for (int i = 1; i < argc; i++){
        pipe_fd[i-1] = atoi(argv[i]);
    }
    printf("valore fd controllo(s,l): %d, %d\n", pipe_fd[1], pipe_fd[0]);
    fflush(stdout);
    
    char input_char; // definisco la variabile di input

    //write into logfile
    FILE *logfile = fopen("logfile.txt", "a");
    if(logfile < 0){ 
        //error opening log file
        perror("fopen: logfile");
        return 1;
    }else{
        //wtite in logfile
        time_t current_time;
        //obtain local time
        time(&current_time);
        fprintf(logfile, "%s => spawn INPUT with pid %d\n", ctime(&current_time), getpid());
        fclose(logfile);
    }
       

    //configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1){ 
        perror("sigaction");
        return -1;
    }

    // definizione delle variabili di ncurses ------
    int Srow, Scol;
    int Wrow, Wcol, Starty, Startx;
    int PRy, PRx;

    WINDOW *external_window;
    WINDOW *printing_window;

    int CBstarty, CBstartx;
    WINDOW *central_butt;

    WINDOW *up_butt;
    WINDOW *down_butt;
    WINDOW *left_butt;
    WINDOW *right_butt;

    WINDOW *up_left_butt;
    WINDOW *up_right_butt;
    WINDOW *down_left_butt;
    WINDOW *down_right_butt;

    // ncurses initialization row, attivo la modalità ncurses
    initscr(); cbreak(); raw(); noecho(); keypad(stdscr, TRUE);

    // definisco i limiti massimi della finsetra, refresh di stdscr
    getmaxyx(stdscr, Srow, Scol);
    refresh();

    // definizione delle finestre per i bottoni
    // external e printing wind creation
    Wrow = (int)(Srow*0.9); Wcol = (int)(Scol*0.8); Starty = 0; Startx = (int)((Scol - Wcol)/2);
    external_window = create_new_window(Wrow, Wcol, Starty, Startx);
    Wrow = (int)(Srow*0.1);Starty = Srow-Wrow;
    printing_window = create_new_window(Wrow, Wcol, Starty, Startx);
    getmaxyx(printing_window, PRy, PRx);

    // central button creation
    Wrow = (int)(Srow*0.1); Wcol = Wrow*2; Starty = (int)((Srow-Wrow)/2); Startx = (int)((Scol - Wcol)/2);
    CBstarty = Starty; CBstartx = Startx;
    central_butt = create_new_window(Wrow, Wcol, Starty, Startx);

    // up, down, left, right button creation
    down_butt = create_new_window(Wrow*2, Wcol, (CBstarty+Wrow), CBstartx);
    up_butt = create_new_window(Wrow*2, Wcol, (CBstarty-(Wrow*2)), CBstartx);
    left_butt = create_new_window(Wrow, Wcol*2, CBstarty, CBstartx-(Wcol*2));
    right_butt = create_new_window(Wrow, Wcol*2, CBstarty, CBstartx+(Wcol));

    // half way button creation
    up_left_butt = create_new_window(Wrow, Wcol, (CBstarty-Wrow), (CBstartx-Wcol));
    up_right_butt = create_new_window(Wrow, Wcol, (CBstarty-Wrow), (CBstartx+Wcol));
    down_left_butt = create_new_window(Wrow, Wcol, (CBstarty+Wrow), (CBstartx-Wcol));
    down_right_butt = create_new_window(Wrow, Wcol, (CBstarty+Wrow), (CBstartx+Wcol));
   
    // do while --> prendo input in modo continuativo
    do{
        input_char = getch();
        switch (input_char)
        {
            case 'w': case_execution(input_char, PRy, PRx, printing_window, up_left_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'e': case_execution(input_char, PRy, PRx, printing_window, up_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'r': case_execution(input_char, PRy, PRx, printing_window, up_right_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'f': case_execution(input_char, PRy, PRx, printing_window, right_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'v': case_execution(input_char, PRy, PRx, printing_window, down_right_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'c': case_execution(input_char, PRy, PRx, printing_window, down_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'x': case_execution(input_char, PRy, PRx, printing_window, down_left_butt, pipe_fd[1], pipe_fd[0]); break;
            case 's': case_execution(input_char, PRy, PRx, printing_window, left_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'd': case_execution(input_char, PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]); break;
            case 'q': case_execution(input_char, PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]);break;
            default: case_execution('A', PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]); break;
        }
        // possibile aggiungere un controllo per vedere se i valori massimi della finestra sono stati modificati e rifare tutto il codice da sopra
        // direi che è oltre gli obbiettivi dell'assignment
    }while(input_char != 'q');

    // termination row
    endwin();
    
    return 0;

}

WINDOW *create_new_window(int row, int col, int ystart, int xstart){
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    wrefresh(local_window);
    return local_window;
}

void case_execution(char input_char, int PRy, int PRx, WINDOW *print_pointer, WINDOW *color_pointer, int write_fd, int read_fd){
    // pipe section
    // write on pipe
    int controllo;
    close(read_fd);
    if ((controllo = write(write_fd, &input_char, 1))<0){
        perror("errore write");
    }
    //
    char string[30] = "hai premuto il tasto: ";
    strncat(string, &input_char, 1);
    mvwaddstr(print_pointer, PRy/2, ((PRx-strlen(string))/2), string);
    //wprintw(print_pointer, "valore controllo: %d", controllo);
    wrefresh(print_pointer);

    if(has_colors()==FALSE){
        mvwprintw(print_pointer, PRy/2, ((PRx-strlen("terminale non supporta colore"))/2), "terminale non supporta colore");
    }else{
        start_color();
        init_pair(2, COLOR_WHITE, COLOR_BLUE);
        init_pair(1, COLOR_WHITE, COLOR_BLACK);

        wbkgd(color_pointer, COLOR_PAIR(2));
        wrefresh(color_pointer);
        napms(50);
        // Return to the default color
        wbkgd(color_pointer, COLOR_PAIR(1));
        wrefresh(color_pointer);
    }
}