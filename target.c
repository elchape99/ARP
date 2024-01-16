//// This process generate 20 target. generate 20 random value from 0 to 1. And send all the value by pipe to server

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

#define MAX_TARG_ARR_SIZE 10

// Function header
void writeLog(const char *format, ...);
void sigusr1Handler(int signum, siginfo_t *info, void *context);

int main(int argc, char *argv[])
{
    // variable used in for cycle
    int i;
    pid_t target_pid = getpid();
    // write into logfile the pid
    writeLog("TARGET create with pid %d ", target_pid);

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("target: sigaction");
        writeLog("==> ERROR ==> target: sigaction input %m ");
    }

    // Take the fd for comunicating with master, it's position is 1,2 in argv[]
    int fd4[2];
    for (i = 1; i < 3; i++)
    {
        fd4[i - 1] = atoi(argv[i]);
    }
    writeLog("TARGET value of fd4 are: %d %d ", fd4[0], fd4[1]);

    // close the read fiel descriptor fd1[0]
    if (close(fd4[0]) < 0)
    {
        perror("targtet: close fd1[1]");
        writeLog("==> ERROR ==> target: close fd1[0], %m ");
    }
    // write the pid in the pipe
    if (write(fd4[1], &target_pid, sizeof(target_pid)) < 0)
    {
        perror("target: write fd4[1],");
        writeLog("==> ERROR ==> target, write fd4[1] %m ");
    }
    if (close(fd4[1]) < 0)
    {
        perror("target: close fd4[1]");
        writeLog("==> ERROR ==> target: close fd4[1], %m ");
    }

    //// pipe for comunication between target->server, are in position 3, 4
    int fdt_s[2];
    for (i = 3; i < 5; i++)
    {
        fdt_s[i - 3] = atoi(argv[i]);
    }
    writeLog("TARGET value of fdt_s are: %d %d ", fdt_s[0], fdt_s[1]);

    // close the read file descriptor, target only write data in the pipe
    if (close(fdt_s[0]) == -1)
    {
        perror("target: close fdt_s[0] ");
        writeLog("==> ERROR ==> target: close fdt_s[0], %m ");
    }

    double set_of_target[MAX_TARG_ARR_SIZE][2];
    for (int i = 0; i < MAX_TARG_ARR_SIZE; i++)
    {
        set_of_target[i][0] = ((double)rand() / RAND_MAX) - 0.5;
        set_of_target[i][1] = ((double)rand() / RAND_MAX) - 0.5;
    }

    if (write(fdt_s[1], set_of_target, sizeof(double) * MAX_TARG_ARR_SIZE * 2) == -1)
    {
        perror("obstacle: error write fdo_s[1]");
        writeLog("==> ERROR ==> obstacle: write fdo_s[1], %m ");
    }
    for (i = 0; i < MAX_TARG_ARR_SIZE; i++)
    {
        printf("%f, %f \n", set_of_target[i][0], set_of_target[i][1]);
        fflush(stdout);
    }

    while (1)
    {
        sleep(1);
    }
    // close the write file descriptor, target only write data in the pipe
    if (close(fdt_s[1]) == -1)
    {
        perror("target: close fdt_s[1] ");
        writeLog("==> ERROR ==> target: close fdt_s[1], %m ");
    }
}

////---- Functions section -----------------------------------------------------------
/* function for write in logfile*/
void writeLog(const char *format, ...)
{

    FILE *logfile = fopen("logfile.txt", "a");
    if (logfile == NULL)
    {
        perror("server: error opening logfile");
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, format);

    time_t current_time;
    time(&current_time);

    fprintf(logfile, "%s => ", ctime(&current_time));
    vfprintf(logfile, format, args);

    va_end(args);
    fflush(logfile);
    if (fclose(logfile) == -1)
    {
        perror("fclose logfile");
        writeLog("ERROR ==> server: fclose logfile");
    }
}

// Inserire perror nella kill
void sigusr1Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR1)
    {
        /*send a signal SIGUSR2 to watchdog */
        // printf("SERVER sig handler");
        if (kill(info->si_pid, SIGUSR2) == 0)
        {
            writeLog("TARGET: pid %d, received signal from wd pid: %d ", getpid(), info->si_pid);
        }
        else
        {
            perror("target: kill SIGUSR2 ");
            writeLog("==> ERROR ==> target: kill SIGUSR2 %m");
        }
    }
}
