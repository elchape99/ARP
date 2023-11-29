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

void handle_sigusr(int sig, siginfo_t *siginfo, void *context)
{
    printf("Signal %d received from process %d\n", sig, siginfo->si_pid);
    if (sig == SIGUSR1) {
        watchdog = siginfo->si_pid;
        kill(siginfo->si_pid, SIGUSR2) //send signal to the watchdog
    }
    
}
float weight = 0;
float screw = 0; // attrito

int main (int argc, char* argv[])
{
    pid_t mpid;
    int mfd[2];
    float forcex = 0;
    float forcey = 0;

    if ((pipe(mfd)) < 0) {
        perror("pipe map ncurses");
        return 2;
    }
    
    if ((mpid = fork()) == -1) {
        perror("fork map");
        return 1;
    }
    if (mpid == 0) {
        char* argvm[] = {"konsole", "-e","./map", NULL};
        // Add maybe some arguments to argvm
        close(mfd[1]);
        dup2(mfd[0], STDIN_FILENO);
        execvp("konsole",argvm);
        printf("error in exec of map\n");
        return -2;
    }
    close(mfd[0]);


    
    close(mfd[1]);
    wait(NULL);
    return 0;
}