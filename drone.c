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
typedef struct {
    float x;
    float y;
} position;

typedef struct {
    float fx;
    float fy;
} strength;

typedef struct {
    float vx;
    float vy;
} velocity;

float weight = 0;
float screw = 0; // attrito

// metti strutture e sistema shared memory e fi watchdog

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
    // crea pieps da master e mantieni tra input e drone soltanto
    int shmmid; 
    strength *force;
    velocity *vel;
    position *pos;

    if ((shmmid = shmget(IPC_PRIVATE, sizeof(position), 0666)) < 0) {
        perror("shmget drone side");
        return 1;
    }

    // closing the file descriptor

    
    // Deallocating the shared memory
    shmdt(pos);
    shmdt(vel);
    shmdt(force);
    return 0;
}