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
            writeLog("TARGET");
        }
    }
}


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
        perror("input: sigaction");
        writeLog("==> ERROR ==> target: sigaction input %m ");
    }

    // Take the fd for comunicating with master, it's position is 1,2 in argv[]
    int fd4[2];
    for (i = 1; i < 3; i++)
    {
        fd4[i - 1] = atoi(argv[i]);
    }
    writeLog("TARGET value of fd are: %d %d ", fd4[0], fd4[1]);

    // close the read fiel descriptor fd1[0]
    if (close(fd4[0]) < 0)
    {
        perror("server: close fd1[1]");
        writeLog("ERROR ==> server: close fd1[0], %m ");
    }
    // write the pid in the pipe
    if (write(fd4[1], &target_pid, sizeof(target_pid)) < 0)
    {
        perror("server: write fd4[1],");
        writeLog("ERROR ==> server, write fd4[1] %m ");
    }    
    if (close(fd4[1]) < 0)
    {
        perror("server: close fd4[1]");
        writeLog("ERROR ==> server: close fd4[1], %m ");
    }
    while(1)
    {
        sleep(1);
    }


}