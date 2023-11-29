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

#define T 0.01 // Define a time constant
double x = 0, y = 0;
double vx = 0, vy = 0;

// Initialize forces
double fx = 0, fy = 0;

// Fai partire input da server e qua fai la mappa (quindi inverti)

void handle_sigusr(int sig, siginfo_t *siginfo, void *context)
{
    printf("Signal %d received from process %d\n", sig, siginfo->si_pid);
    if (sig == SIGUSR1) {
        watchdog = siginfo->si_pid;
        kill(siginfo->si_pid, SIGUSR2) //send signal to the watchdog
    }
    
}
// Necessaria implementare una pipe per mandare il carattere al drone
// Necessario sistemare il watchdog

int main (int argc, char* argv[])
{

}