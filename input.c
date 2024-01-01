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
#include <stdarg.h>
#include <sys/wait.h>
#include <ncurses.h>

// function for write in logfile
void writeLog(const char *format, ...);
// signals handler functions
void sigusr1Handler(int signum, siginfo_t *info, void *context);

// declarations of functions used for windows
WINDOW *create_new_window(int row, int col, int ystart, int xstart, char icon);
void destroy_win(WINDOW *local_win);
void print_on_button(WINDOW *pointer, char icon, int row, int col);
void case_execution(char input_char, int PRy, int PRx, WINDOW *print_pointer, WINDOW *color_pointer, int write_fd, int read_fd);

// global variables
pid_t wd;

int main(int argc, char *argv[])
{
    int i;
    pid_t input_pid = getpid();
    // write into logfile the pid
    writeLog("INPUT create with pid %d ", input_pid);

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("input: sigaction");
        writeLog("==> ERROR ==> input: sigaction input %m ");
    }

    // Manage pipe ----------------------------------------------------------
    // pipe from master: fd2 are in position 1 and 2 of argv
    int fd2[2];
    for (i = 1; i < 3; i++)
    {
        fd2[i - 1] = atoi(argv[i]);
    }
    writeLog("INPUT value of fd2 are: %d, %d ", fd2[0], fd2[1]);
    // pipe_fd are in position 3 and 4 of argv[]
    int pipe_fd[2]; // recupero dall'argv i file descriptor delle pipe
    for (i = 3; i < 5; i++)
    {
        pipe_fd[i - 3] = atoi(argv[i]);
    }
    writeLog("INPUT value of pipe_fd are: %d, %d ", pipe_fd[0], pipe_fd[1]);

    printf("valore fd controllo(s,l): %d, %d\n", pipe_fd[1], pipe_fd[0]);
    fflush(stdout);

  
    // nput need to write the pid inside the pipe
    // close the read file descriptor fd2[0]
    if (close(fd2[0]) < 0)
    {
        perror("input: close fd2[0] ");
        writeLog("ERROR ==> input: close fd2[0] %m ");
    }
    // write the pid inside the pipe
    if (write(fd2[1], &input_pid, sizeof(input_pid)) < 0)
    {
        perror("write fd2[1] input");
        writeLog("ERROR ==> input: write fd2[1] %m ");
    }
    // close the write file descriptor fd2[1]
    if (close(fd2[1]) < 0)
    {
        perror("close fd2[1] input");
        writeLog("ERROR ==> input: close fd2[1] %m ");
    }

    char input_char; // definisco la variabile di input

    // definizione delle variabili di ncurses ------
    int Srow, Scol, SrowNew, ScolNew;
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

    int times_pressed_vect[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // ncurses initialization row, attivo la modalità ncurses
    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // definisco i limiti massimi della finsetra, refresh di stdscr
    getmaxyx(stdscr, Srow, Scol);
    refresh();

    // definizione delle finestre per i bottoni
    // external e printing wind creation
    Wrow = (int)(Srow * 0.9);
    Wcol = (int)(Scol * 0.8);
    Starty = 0;
    Startx = (int)((Scol - Wcol) / 2);
    external_window = create_new_window(Wrow, Wcol, Starty, Startx, '-');
    Wrow = (int)(Srow * 0.1);
    Starty = Srow - Wrow;
    printing_window = create_new_window(Wrow, Wcol, Starty, Startx, '-');
    getmaxyx(printing_window, PRy, PRx);

    // central button creation
    Wrow = (int)(Srow * 0.1);
    Wcol = Wrow * 2;
    Starty = (int)((Srow - Wrow) / 2);
    Startx = (int)((Scol - Wcol) / 2);
    CBstarty = Starty;
    CBstartx = Startx;
    central_butt = create_new_window(Wrow, Wcol, Starty, Startx, 'D');

    // up, down, left, right button creation
    down_butt = create_new_window(Wrow * 2, Wcol, (CBstarty + Wrow), CBstartx, 'C');
    up_butt = create_new_window(Wrow * 2, Wcol, (CBstarty - (Wrow * 2)), CBstartx, 'E');
    left_butt = create_new_window(Wrow, Wcol * 2, CBstarty, CBstartx - (Wcol * 2), 'S');
    right_butt = create_new_window(Wrow, Wcol * 2, CBstarty, CBstartx + (Wcol), 'F');

    // half way button creation
    up_left_butt = create_new_window(Wrow, Wcol, (CBstarty - Wrow), (CBstartx - Wcol), 'W');
    up_right_butt = create_new_window(Wrow, Wcol, (CBstarty - Wrow), (CBstartx + Wcol), 'R');
    down_left_butt = create_new_window(Wrow, Wcol, (CBstarty + Wrow), (CBstartx - Wcol), 'X');
    down_right_butt = create_new_window(Wrow, Wcol, (CBstarty + Wrow), (CBstartx + Wcol), 'V');

    // do while --> prendo input in modo continuativo
    while (input_char != 'q')
    {
        input_char = getch();
        switch (input_char)
        {
        case 'w':
            case_execution(input_char, PRy, PRx, printing_window, up_left_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'e':
            case_execution(input_char, PRy, PRx, printing_window, up_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'r':
            case_execution(input_char, PRy, PRx, printing_window, up_right_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'f':
            case_execution(input_char, PRy, PRx, printing_window, right_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'v':
            case_execution(input_char, PRy, PRx, printing_window, down_right_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'c':
            case_execution(input_char, PRy, PRx, printing_window, down_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'x':
            case_execution(input_char, PRy, PRx, printing_window, down_left_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 's':
            case_execution(input_char, PRy, PRx, printing_window, left_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'd':
            case_execution(input_char, PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]);
            break;
        case 'q':
            case_execution(input_char, PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]);
            break;
        default:
            case_execution('A', PRy, PRx, printing_window, central_butt, pipe_fd[1], pipe_fd[0]);
            break;
        }
        getmaxyx(stdscr, SrowNew, ScolNew);
        if (SrowNew != Srow || ScolNew != Scol){
            Scol = ScolNew;
            Srow = SrowNew;

            clear();

            destroy_win(external_window);
            destroy_win(printing_window);

            destroy_win(central_butt);

            destroy_win(down_butt);
            destroy_win(up_butt);
            destroy_win(left_butt);
            destroy_win(right_butt);

            destroy_win(down_left_butt);
            destroy_win(down_right_butt);
            destroy_win(up_left_butt);
            destroy_win(up_right_butt);

            refresh();

            // definizione delle finestre per i bottoni
            // external e printing wind creation
            Wrow = (int)(Srow * 0.9);
            Wcol = (int)(Scol * 0.8);
            Starty = 0;
            Startx = (int)((Scol - Wcol) / 2);
            external_window = create_new_window(Wrow, Wcol, Starty, Startx, '-');
            Wrow = (int)(Srow * 0.1);
            Starty = Srow - Wrow;
            printing_window = create_new_window(Wrow, Wcol, Starty, Startx, '-');
            getmaxyx(printing_window, PRy, PRx);

            // central button creation
            Wrow = (int)(Srow * 0.1);
            Wcol = Wrow * 2;
            Starty = (int)((Srow - Wrow) / 2);
            Startx = (int)((Scol - Wcol) / 2);
            CBstarty = Starty;
            CBstartx = Startx;
            central_butt = create_new_window(Wrow, Wcol, Starty, Startx, 'D');

            // up, down, left, right button creation
            down_butt = create_new_window(Wrow * 2, Wcol, (CBstarty + Wrow), CBstartx, 'C');
            up_butt = create_new_window(Wrow * 2, Wcol, (CBstarty - (Wrow * 2)), CBstartx, 'E');
            left_butt = create_new_window(Wrow, Wcol * 2, CBstarty, CBstartx - (Wcol * 2), 'S');
            right_butt = create_new_window(Wrow, Wcol * 2, CBstarty, CBstartx + (Wcol), 'F');

            // half way button creation
            up_left_butt = create_new_window(Wrow, Wcol, (CBstarty - Wrow), (CBstartx - Wcol), 'W');
            up_right_butt = create_new_window(Wrow, Wcol, (CBstarty - Wrow), (CBstartx + Wcol), 'R');
            down_left_butt = create_new_window(Wrow, Wcol, (CBstarty + Wrow), (CBstartx - Wcol), 'X');
            down_right_butt = create_new_window(Wrow, Wcol, (CBstarty + Wrow), (CBstartx + Wcol), 'V');
        }
    }

    // termination row
    endwin();
    //close the pipe file descriptor
    if (close(pipe_fd[0]) < 0)
    {
        perror("close pipe_fd[0] ");
        writeLog("ERROR ==> close pipe_fd[0] input %m ");
    }
    if (close(pipe_fd[1]) < 0)
    {
        perror("close pipe_fd[1] ");
        writeLog("ERROR ==> close pipe_fd[1] input %m ");
    }

    return 0;
}

/* Function for write into logfile */
void writeLog(const char *format, ...)
{

    FILE *logfile = fopen("logfile.txt", "a");
    if (logfile < 0)
    {
        perror("Error opening logfile");
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, format);

    time_t current_time;
    time(&current_time);

    fprintf(logfile, "%s => ", ctime(&current_time));
    vfprintf(logfile, format, args);

    va_end(args);
    fflush(logfile);
    if (fclose(logfile) < 0)
    {
        perror(" fclose logfile: ");
    }
}

void sigusr1Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR1)
    {
        /*send a signal SIGUSR2 to watchdog */
        kill(info->si_pid, SIGUSR2);
        writeLog("input, pid: %d, received signal from wd pid: %d ", getpid(), info->si_pid);
    }
}

WINDOW *create_new_window(int row, int col, int ystart, int xstart, char icon)
{
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    if (icon != '-')
    {
        print_on_button(local_window, icon, row, col);
    }

    wrefresh(local_window);
    return local_window;
}

void destroy_win(WINDOW *local_win)
{
    box(local_win, ' ', ' '); 
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}

void case_execution(char input_char, int PRy, int PRx, WINDOW *print_pointer, WINDOW *color_pointer, int write_fd, int read_fd)
{
    // pipe section
    // write on pipe
    int controllo;
    close(read_fd);
    if ((controllo = write(write_fd, &input_char, 1)) < 0)
    {
        perror("errore write");
    }
    //
    char string[30] = "hai premuto il tasto: ";
    strncat(string, &input_char, 1);
    mvwaddstr(print_pointer, PRy / 2, ((PRx - strlen(string)) / 2), string);
    // wprintw(print_pointer, "valore controllo: %d", controllo);
    wrefresh(print_pointer);

    if (has_colors() == FALSE)
    {
        mvwprintw(print_pointer, PRy / 2, ((PRx - strlen("terminale non supporta colore")) / 2), "terminale non supporta colore");
    }
    else
    {
        start_color();
        init_pair(2, COLOR_RED, COLOR_BLUE);
        init_pair(1, COLOR_WHITE, COLOR_BLACK);

        wbkgd(color_pointer, COLOR_PAIR(2));
        wrefresh(color_pointer);
        napms(50);
        // Return to the default color
        wbkgd(color_pointer, COLOR_PAIR(1));
        wrefresh(color_pointer);
    }
}

void print_on_button(WINDOW *pointer, char icon, int row, int col)
{
    char string[50];
    sprintf(string, "%c", icon);
    mvwaddstr(pointer, (row / 2), ((col - strlen(string)) / 2), string);
}