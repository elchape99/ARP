#include "window.h"


WINDOW *create_new_window(int row, int col, int ystart, int xstart){
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    wrefresh(local_window);
    return local_window;
}

void destroy_win(WINDOW **local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); // remove the box and the window

    wrefresh(local_win); // show the changes
    delwin(local_win); // delete the window
}

void init_windows(int Srow, int Scol, WINDOW **ext_win, WINDOW **print_win, int *PRy, int *PRx,int *Startx, int *Starty,int *Wcol, int *Wrow) {
    
    *Wrow = (int)(Srow * 0.8);
    *Wcol = (int)(Scol * 0.8);
    *Starty = 0; 
    *Startx = (int)((Scol - (*Wcol))/2);
    *ext_win = create_new_window(*Wrow, *Wcol, *Starty, *Startx);
    *Wrow = (int)(Srow * 0.1);
    *Starty = Srow - (*Wrow);
    *print_win = create_new_window(*Wrow, *Wcol, *Starty, *Startx);
    getmaxyx(*print_win, *PRy, *PRx);
    

    // central button creation
    *Wrow = (int)(Srow * 0.1); 
    *Wcol = (*Wrow) * 2; 
    *Starty = (int)((Srow - (*Wrow))/2); 
    *Startx = (int)((Scol - (*Wcol))/2);
    wrefresh(*ext_win);
    wrefresh(*print_win);   
}

void print_btt_windows(WINDOW **win, char ch) {
    int maxX, maxY;
    getmaxyx(*win, maxY, maxX);
    start_color();
    
    if (ch == 'Q') {
        init_pair(2, COLOR_RED, COLOR_BLACK);
        wattron(*win, COLOR_PAIR(2) | A_BOLD);
    }
    else {
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        wattron(*win, COLOR_PAIR(1) | A_BOLD);
    }
    
    mvwaddch(*win,(int) ((maxY) / 2), (int) ((maxX - 1) / 2), ch);
    
    if (ch == 'Q') {
        wattroff(*win, COLOR_PAIR(2) | A_BOLD);
    }
    else {
        wattroff(*win, COLOR_PAIR(1) | A_BOLD);
    }
    wrefresh(*win);
}

void boxCreation(WINDOW **win, int *maxY, int *maxX) {
    getmaxyx(*win, *maxY, *maxX);
    box(*win, 0, 0);
    wrefresh(*win);
}

void squareCreation (WINDOW **win, int height, int width,int *hg, int *wg)
{
    int heightq = height / 3;
    int widthq = width / 3;

    // Q subsquare
    box(win[NUMWINDOWS], 0, 0);
    mvwprintw(win[NUMWINDOWS], heightq / 2, widthq / 2, "Q");
    wrefresh(win[NUMWINDOWS]);

    // Subsquare
    int y, x;
    for (int i = 0; i < NUMWINDOWS - 1; ++i) {
        y = (i / 3) * heightq + heightq;
        x = (i % 3) * widthq;
        win[i] = newwin(heightq, widthq, y, x);
        box(finestre[i], 0, 0);
        wrefresh(finestre[i]);
    }
    *hg = heightq;
    *wg = widthq;
}

void lightWindow(WINDOW **win, chtype attr, int ind) {
    
    wattron(win[ind], attr); 
    wrefresh(win[ind]); 
    //usleep(1000000); 


    wattroff(win[ind], attr); 
    wrefresh(win[ind]);
}

void printCounter(WINDOW *win, int num) {
    int h, w;
    getmaxyx(win, h, w);

    // Calcola le coordinate per centrare il carattere e il numero
    int y = h / 2;
    int x = (w - 2) / 2; // Sottrai 2 per far spazio al carattere e al numero

    // Stampa il carattere e il numero al centro della finestra
    mvwaddch(win, y, x, 'x');
    mvwprintw(win, y, x + 1, "%d", num);
    wrefresh(win);
}