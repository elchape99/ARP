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

/*
Acquired all the char and sends to the drone through pipes
*/

#define DEBUG 1

//v#ifndef DEBUG
//managing signal
void sigusr1Handler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGUSR1){
        /*send a signal SIGUSR2 to watchdog */
        // wd = info->si_pid;
        kill(info->si_pid, SIGUSR2);
    }
}
// #endif

int main (int argc, char* argv[])
{
    //pipe creation
    int inpfd[2];
    char pidstr[2][70];
    pid_t cid;
    int ch;
    char realchar = '\0';
    int counter[NUMMOTIONS];
    FILE *inputfile;

    inputfile = fopen("input.txt", "w");
    if (inputfile == NULL)
    {
        perror("Error opening file! input\n");
        return 1;
        //exit(1);
    }
    fprintf(inputfile, "input created\n");
    fclose(inputfile);

    //configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1){ 
        perror("sigaction");
        return -1;
    }

    //create input file

// #ifndef DEBUG
    // opening pipe
    if (pipe(inpfd) < 0) {
        perror("pipe input ncurses");
        return 2;
    }
    for (int i = 0; i < 2; i++) {
        sprintf(pidstr[i], "%d", inpfd[i]);
    }

    if ((cid = fork()) == -1) {
        perror("fork");
        return 1;
    }
    // here to pass the pipe
    char * argcou[] = {"konsole", "-e","./inputc",pidstr[0], pidstr[1], NULL};
    if (cid == 0) {
        // dup2(inpfd[1], STDOUT_FILENO);
        execvp("konsole",argcou);
        printf("error in exec of input\n");
        return -2;
    }
    // closing write
    close(inpfd[1]);
    // Doing input process
    int pinpdrone[2];
    FILE *logfile = fopen("logfile.txt", "a");
    inputfile = fopen("input.txt", "a");

    if (logfile < 0) { //if problem opening file, send error
        perror("fopen: logfile");
        return 1;
    }
    /*if (inputfile < 0) { //if problem opening file, send error
        perror("fopen: inputfile");
        return 2;
    }/*/
    else {
        //wtite in logfile
        time_t crttime;
        time(&crttime);
        fprintf(logfile, "Input process created at %s\n with pid %d\n", ctime(&crttime),getpid());
        fflush(logfile);
        fprintf(inputfile, "Input process created at %s\n with pid %d\n", ctime(&crttime), getpid());
        fflush(inputfile);
        fclose(logfile);
    }
    /*
    struct sigaction sa;
    sa.sa_sigaction  = handle_sigusr;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 3;
    }*/
    // opening pipe
    for (int i = 0; i < 2; i++) {
        pinpdrone[i] = atoi(argv[i]); // converts from the string to the integer
    }
    close(pinpdrone[0]);

    // while to get char
    while(ch != 'Q') {
        if ((read(inpfd[0], &ch, sizeof(char))) < 0) {
            perror("read input ncurses");
            return 3;
        }
        if ((write(pinpdrone[1], &ch, sizeof(char))) < 0) {
            perror("write input ncurses");
            return 3;
        }
        if ((fprintf(inputfile, "Pressed char: %c\n", ch)) < 0) {
            perror("fprintf input ncurses");
            return 4;
        }
        else{
            fflush(inputfile);
        }
    }
    fclose(inputfile);
    // closing read
    close(inpfd[0]);
    close(pinpdrone[1]);

    // Managing signal for the watchdog //work in progress
    


    wait(NULL); // wait for the child to terminate
// #endif
    return 0;
}
