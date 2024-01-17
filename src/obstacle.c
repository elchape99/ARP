#include <ncurses.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "arplib.h"
#include "../config/config.h"


//-- Functions header --------------------------------------------
void sigusr1Handler(int signum, siginfo_t *info, void *context);

int main(int argc, char *argv[])
{
    // variable used in for cycle
    int i;
    pid_t obstacle_pid = getpid();
    // write into logfile the pid
    writeLog("OBSTACLE create with pid %d ", obstacle_pid);

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("obstacle: sigaction");
        writeLog("==> ERROR ==> obstacle: sigaction %m ");
    }

    //// -- manage pipe ----------------------------------------------------------
    // Take the fd for comunicating with master, it's position is 1,2 in argv[]
    int fd5[2];
    for (i = 1; i < 3; i++)
    {
        fd5[i - 1] = atoi(argv[i]);
    }
    writeLog("OBSTACLE value of fd5 are: %d %d ", fd5[0], fd5[1]);

    // close the read file descriptor fd1[0]
    if (close(fd5[0]) < 0)
    {
        perror("obstacle: close fd5[1]");
        writeLog("==> ERROR ==> obstacle: close fd5[0], %m ");
    }
    // write the pid in the pipe
    if (write(fd5[1], &obstacle_pid, sizeof(obstacle_pid)) < 0)
    {
        perror("obstacle: write fd5[1],");
        writeLog("==> ERROR ==> obstacle, write fd5[1] %m ");
    }
    if (close(fd5[1]) < 0)
    {
        perror("obstacle: close fd5[1]");
        writeLog("==> ERROR ==> obstacle: close fd5[1], %m ");
    }

    //// pipe for communication between obstacle -> server, are in ositions 3, 4 of argv[]
    printf("print in screen the value of argv 3 and 4: %s, %s ", argv[3], argv[4]);

    int fdo_s[2];
    for (i = 3; i < 5; i++)
    {
        fdo_s[i - 3] = atoi(argv[i]);
    }
    writeLog("OBSTACLE value of fdo_s are: %d %d ", fdo_s[0], fdo_s[1]);

    // close the read file descriptor
    if (close(fdo_s[0]) == -1)
    {
        perror("obstacle: close fdo_s[0] ");
        writeLog("==> ERROR ==> obstacle: close fdo_s[0], %m ");
    }

    // initialize the time on random generator
    srand(time(NULL));

    double set_of_obstacle[MAX_OBST_ARR_SIZE][2];
    while (1)
    {
        for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
        {
            set_of_obstacle[i][0] = (double)rand() / RAND_MAX - 0.5;
            set_of_obstacle[i][1] = (double)rand() / RAND_MAX - 0.5;
        }

        if (write(fdo_s[1], set_of_obstacle, sizeof(double) * MAX_OBST_ARR_SIZE * 2) == -1)
        {
            perror("obstacle: error write fdo_s[1]");
            writeLog("==> ERROR ==> obstacle: write fdo_s[1], %m ");
        }
        for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
        {
            printf("%f, %f \n", set_of_obstacle[i][0], set_of_obstacle[i][1]);
            fflush(stdout);
        }
        // generat obstacle every seconds
        sleep(N);
    }

    // close the write file descriptor fdo_s
    if (close(fdo_s[1]) == -1)
    {
        perror("obstacle: close fdo_s[1] ");
        writeLog("==> ERROR ==> obstacle: close fdo_s[1], %m ");
    }
}

//// --- Function section -------------------------------------------------------------

// Inserire perror nella kill
void sigusr1Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR1)
    {
        /*send a signal SIGUSR2 to watchdog */
        if (kill(info->si_pid, SIGUSR2) == 0)
        {
            writeLog("OBSTACLE: pid %d, received signal from wd pid: %d ", getpid(), info->si_pid);
        }
        else
        {
            perror("obstacle: kill SIGUSR2 ");
            writeLog("==> ERROR ==> obstacle: kill SIGUSR2 %m ");
        }
    }
}