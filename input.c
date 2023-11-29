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

//managing signal
void handle_sigusr(int sig, siginfo_t *siginfo, void *context)
{
    printf("Signal %d received from process %d\n", sig, siginfo->si_pid);
    if (sig == SIGUSR1) {
        watchdog = siginfo->si_pid;
        kill(siginfo->si_pid, SIGUSR2) //send signal to the watchdog
    }
    
}


int main (int argc, char* argv[])
{
    //pipe creation
    int inpfd[2];
    pid_t cid;
    int ch;
    char realchar = '\0';
    int counter[NUMMOTIONS];
    // opening pipe
    if (pipe(inpfd) < 0) {
        perror("pipe input ncurses");
        return 2;
    }

    if ((cid = fork()) == -1) {
        perror("fork");
        return 1;
    }
    char * argcou[] = {"konsole", "-e","./inputc", NULL};
    if (cid == 0) {
        close(inpfd[0]);

        dup2(inpfd[1], STDOUT_FILENO);
        execvp("konsole",argcou);
        printf("error in exec of input\n");
        return -2;
    }
    // closing write
    close(inpfd[1]);
    // Doing input process
    int inpdrone[2];
    FILE *logfile = fopen("logfile.txt", "a");
    FILE *inputfile = fopen("input.txt", "w");

    if (logfile < 0) { //if problem opening file, send error
        perror("fopen: logfile");
        return 1;
    }
    if (inputfile < 0) { //if problem opening file, send error
        perror("fopen: inputfile");
        return 2;
    }
    else {
        //wtite in logfile
        time_t ctime;
        time(&ctime);
        fprintf(logfile, "Input process created at %s\n with pid %n", ctime(&ctime),getpid());
        fprintf(inputfile, "Input process created at %s\n with pid %n", ctime, getpid());
        fclose(logfile);
    }
    struct sigaction sa;
    sa.sa_sigaction  = handle_sigusr;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 3;
    }

    // while to get char
    while(ch != 'Q') {
        if ((read(inpfd[0], &ch, sizeof(char))) < 0) {
            perror("read input ncurses");
            return 3;
        }
        if ((fprintf(inputfile, "Pressed char: %c\n", ch)) < 0) {
            perror("fprintf input ncurses");
            return 4;
        }
    }
    fclose(inputfile);
    // closing read
    close(inpfd[0]);

    // Managing signal for the watchdog //work in progress
    


    wait(NULL); // wait for the child to terminate
    return 0;
}
