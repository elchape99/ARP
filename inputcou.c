#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>
#include "library/window.h"

#define MAX 6
void counterImplementation (int *cnt, int *c, int lung);

int main (int argc, char* argv[])
{
    WINDOW *cwin;
    int counter[4]; // s, e, f, c
    char realchar = '\0';
    int lng = 4;
    int ch;
    int i;
    int index = 0;

    initscr();
    cbreak();
    noecho();

    int hight, width;
    int shight, swidth;
    getmaxyx(stdscr, hight, width);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    // Calcola le dimensioni per i sottoquadrati
    squareCreation(&cwin, hight, width, &shight, &swidth);


    // while to get char
    while (1) {
        ch = getch();

        switch (ch)
        {
        case 'w':
            realchar = 'w'; // up left
            index = BTTW;
            break;
        case 'e':
            realchar = 'e'; // up
            index = BTTE;

            break;
        case 'r':
            realchar = 'r'; // up right
            index = BTTR;

            break;
        case 's':
            index = BTTS;
            break;
        case 'd':
            realchar = 'd'; // delete forces
            index = BTTD;

            break;
        case 'f':
            realchar = 'f'; // right
            index = BTTF;

            break;
        case 'x':
            realchar = 'x'; // Down left
            index = BTTX;

        case 'c':
            realchar = 'c'; // Down
            index = BTTC;

            break;
        case 'v':
            realchar = 'v'; // down Right
            index = BTTV;

            break;
        case 'p':
            realchar = 'p'; // pause command
            break;
        case 'b':
            realchar = 'b'; // break
            break;
        case 'q':
            realchar = 'Q'; // Termina il programma
            mvprintw(0, 0, "Closing the program\n");
            sleep(3);
            break;
        default:
            realchar = '\0'; // Comando non valido
            break;
        }

        counterImplementation(counter, &ch, lng);
        lightWindow(&cwin, COLOR_PAIR(1) | A_BOLD, index);
        printCounter(cwin[BTTS], counter[0]);
        printCounter(cwin[BTTE], counter[1]);
        printCounter(cwin[BTTF], counter[2]);
        printCounter(cwin[BTTC], counter[3]);
        // pipe construction
        if (realchar != '\0') {
            //write in the pipe
            fprintf(stdout, "%c", realchar);
            if (realchar == 'Q') {
                clear();
                printw("Closing the program\n");
                refresh();
                sleep(3);
                break;
            }
        } 
    }
    // closing all the pipes
    fclose(stdout);
    for (i = 0; i < NUMWINDOWS; i++) {
        destroy_win(&cwin[i]);
    }   
    endwin();
    return 0;
}


void counterImplementation (int *cnt, int *c, int lung)
{
    char ct;
    ct = (char) *c;
    if (ct == 'w' || ct == 'e'|| ct == 'r')
    {
        if (cnt[1] < MAX )
        {
            cnt[1]++;
        }
        if (cnt[3] > 0)
        {
            cnt[3]--;
        }
    }
    if (ct == 'v' || ct == 'f'|| ct == 'r')
    {
        if (cnt[2] < MAX )
        {
            cnt[2]++;
        }
        if (cnt[0] > 0)
        {
            cnt[0]--;
        }
    }
    if (ct == 'v' || ct == 'c'|| ct == 'x')
    {
        if (cnt[3] < MAX )
        {
            cnt[3]++;
        }
        if (cnt[1] > 0)
        {
            cnt[1]--;
        }
    }
    if (ct == 'w' || ct == 's'|| ct == 'x')
    {
        if (cnt[0] < MAX )
        {
            cnt[0]++;
        }
        if (cnt[2] > 0)
        {
            cnt[2]--;
        }
    }
    if (ct == 'd' || 'b')
    {
       for (int i = 0; i < lung; i++)
        {
            counter[i] = 0;
        }
    }
}