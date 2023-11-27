#include "window.h"


WINDOW *create_new_window(int row, int col, int ystart, int xstart){
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    wrefresh(local_window);
    return local_window;
}

void destroy_win(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); // remove the box and the window

    wrefresh(local_win); // show the changes
    delwin(local_win); // delete the window
}

void init_windows(int Srow, int Scol, WINDOW **external_window, WINDOW **printing_window, int *PRy, int *PRx,int *Startx, int *Starty,int *Wcol, int *Wrow) {
    
    *Wrow = (int)(Srow * 0.8);
    *Wcol = (int)(Scol * 0.8);
    *Starty = 0; 
    *Startx = (int)((Scol - (*Wcol))/2);
    *external_window = create_new_window(*Wrow, *Wcol, *Starty, *Startx);
    *Wrow = (int)(Srow * 0.1);
    *Starty = Srow - (*Wrow);
    *printing_window = create_new_window(*Wrow, *Wcol, *Starty, *Startx);
    getmaxyx(*printing_window, *PRy, *PRx);
    

    // central button creation
    *Wrow = (int)(Srow * 0.1); 
    *Wcol = (*Wrow) * 2; 
    *Starty = (int)((Srow - (*Wrow))/2); 
    *Startx = (int)((Scol - (*Wcol))/2);
    wrefresh(*external_window);
    wrefresh(*printing_window);   
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
