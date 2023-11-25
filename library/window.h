/*
Assicurati che il compilatore possa trovare la tua libreria e i file header. Quando compili il tuo programma, potresti dover 
specificare il percorso della cartella delle librerie usando l'opzione -I seguita dal percorso della cartella contenente i 
file header. Ad esempio, se stai usando il compilatore GCC, il comando potrebbe assomigliare a questo:

gcc -o output_file src/main.c -I./libreria

Questo comando dice al compilatore di cercare i file header nella cartella libreria mentre compila il file main.c e di 
generare il file eseguibile output_file.
*/

#ifndef WINDOW_H
#define WINDOW_H
#include <ncurses.h>
#include <curses.h>

# define NUMWINDOWS 10
# define BTTW 0
# define BTTE 1
# define BTTR 2
# define BTTS 3
# define BTTD 4
# define BTTF 5
# define BTTX 6
# define BTTC 7
# define BTTV 8
# define BTTQ 9




/*
Remember to put the windows for row in the array that you'll create
*/
/*
WINDOW *central_butt; // central button

WINDOW *up_butt; 
WINDOW *down_butt;
WINDOW *left_butt;
WINDOW *right_butt;

WINDOW *up_left_butt;
WINDOW *up_right_butt;
WINDOW *down_left_butt;
WINDOW *down_right_butt;
*/


WINDOW *create_new_window(int , int , int , int );
void destroy_win(WINDOW *local_win);
void init_windows(int, int, WINDOW**, WINDOW**, int*, int*, int*, int*,int*, int*);
void print_btt_windows(WINDOW**, char);


#endif