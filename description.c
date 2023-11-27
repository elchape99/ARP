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




#include "library/window.h"




int main (int argc, char* argv[])
{
    WINDOW *external_window;
    WINDOW *printing_window;
    WINDOW *arrw[NUMWINDOWS];
    
    int Srow, Scol;
    int Wrow, Wcol, Starty, Startx;
    char ch;
    int CBstarty, CBstartx;
    int PRy, PRx;

    initscr(); // start curses mode
    raw();
    noecho();
    start_color();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, Srow, Scol);
    refresh();
    
    // Windows initialization
    init_windows(Srow, Scol, &external_window, &printing_window,&PRy,&PRx,&Startx,&Starty,&Wcol,&Wrow);
    CBstarty = Starty; 
    CBstartx = Startx;
    refresh();    
    printw("Description of the game: \n");
    printw("\nThis game is a simple game of a drone control, where the user can press \nthis buttons to drive the robot to avoid the obstacles and reach the targets.\n");
    // wrefresh(external_window);
    refresh();
    wrefresh(printing_window);

    for (int i = 0; i < NUMWINDOWS; i++){
        switch (i)
        {
            case BTTW:
                arrw[i] = create_new_window(Wrow, Wcol, (CBstarty-Wrow), (CBstartx-Wcol));
                print_btt_windows(&arrw[i], 'W');
                break;
            case BTTE:
                arrw[i] = create_new_window(Wrow*2, Wcol, (CBstarty-(Wrow*2)), CBstartx);
                print_btt_windows(&arrw[i], 'E');
                break;
            case BTTR:  
                arrw[i] = create_new_window(Wrow, Wcol, (CBstarty-Wrow), (CBstartx+Wcol));
                print_btt_windows(&arrw[i], 'R');
                break;
            case BTTS:
                arrw[i] = create_new_window(Wrow, Wcol*2, CBstarty, CBstartx-(Wcol*2));
                print_btt_windows(&arrw[i], 'S');
                break;
            case BTTD:
                arrw[i] = create_new_window(Wrow, Wcol, Starty, Startx);
                break;
            case BTTF:
                arrw[i] = create_new_window(Wrow, Wcol*2, CBstarty, CBstartx+(Wcol));
                print_btt_windows(&arrw[i], 'F');
                break;
            case BTTX:
                arrw[i] = create_new_window(Wrow, Wcol, (CBstarty+Wrow), (CBstartx-Wcol));
                print_btt_windows(&arrw[i], 'X');
                break;
            case BTTC:
                arrw[i] = create_new_window(Wrow*2, Wcol, (CBstarty+Wrow), CBstartx);
                print_btt_windows(&arrw[i], 'C');
                break;
            case BTTV:
                arrw[i] = create_new_window(Wrow, Wcol, (CBstarty+Wrow), (CBstartx+Wcol));
                print_btt_windows(&arrw[i], 'V');
                break;
            case BTTQ:
                arrw[i] = create_new_window(Wrow, Wcol, (CBstarty-(Wrow*2)), CBstartx-(Wcol*2));
                print_btt_windows(&arrw[i], 'Q');
                break;
            default:
                printf("Error in the switch case\n");
                break;
        }
    }

    printw("\nPress any key to exit\n");
    refresh();
    wgetch(printing_window);
    wrefresh(printing_window);
    endwin();

    return 0;
}
    


