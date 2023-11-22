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



WINDOW *create_new_window(int row, int col, int ystart, int xstart);
void case_execution(char input_char, int PRy, int PRx, WINDOW *print_pointer, WINDOW *color_pointer, int write_fd, int read_fd);

int main(int argc, char *argv[]){
    int pipe_fd[2];

    for (int i = 1; i < argc; i++){
        pipe_fd[i-1] = atoi(argv[i]);
    }
    
    char input_char;

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


    // initialization row
    initscr(); cbreak(); raw(); noecho(); keypad(stdscr, TRUE);

    getmaxyx(stdscr, Srow, Scol);
    refresh();

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

    // case switch area
    while((input_char = getch())!= 'q'){
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
            default: case_execution('A', PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]); break;
        }
        // possibile aggiungere un controllo per vedere se i valori massimi della finestra sono stati modificati e rifare tutto il codice da sopra
        // direi che è oltre gli obbiettivi dell'assignment
    }

    // termination row
    endwin();

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
    wprintw(print_pointer, "valore controllo: %d", controllo);
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