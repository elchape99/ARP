#include <stdio.h>
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

#define PROCESS_NUMBER 3;

void writeLog(const char *format, ...)
{

    FILE *logfile = fopen("logfile.txt", "a");
    if (logfile < 0)
    {
        perror("Error opening logfile");
    }
    va_list args;
    va_start(args, format);

    time_t current_time;
    time(&current_time);

    fprintf(logfile, "%s => ", ctime(&current_time));
    vfprintf(logfile, format, args);

    va_end(args);
    fflush(logfile);
    if (fclose(logfile) < 0)
    {
        perror("fclose logfile:");
    }
}

/*  when signal arrive counter --
    when wd send kill counter ++ */
int counter = 0;
/*signal hadler function*/
void sigusr2Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR2)
    {
        writeLog("WATCHDOG received signal from %d ", info->si_pid);
        counter--;
    }
}

int main(int argc, char *argv[])
{

    pid_t wd_pid = getpid();
    // write into logfile
    writeLog("WATCHDOG is create with pid %d ", wd_pid);
    // In this array I will put all the proces pid converted in int
    int num_ps = PROCESS_NUMBER;
    pid_t pids_from_master[num_ps];
    pid_t pids_from_process[num_ps];
    // declared for all the for cycle
    int i; 

    /*configure the handler for sigusr2*/
    struct sigaction sa_usr2;
    sa_usr2.sa_sigaction = sigusr2Handler;
    sa_usr2.sa_flags = SA_SIGINFO; // I need also the info foruse the pid of the process for unde
    if (sigaction(SIGUSR2, &sa_usr2, NULL) == -1)
    {
        perror("wd: sigaction");
        writeLog("ERROR ==> wd: sigaction %m ");
    }

    // exctract the pids coming from mster They are in positions 7, 8 and 9 of argv[]
    // pids coming from master are different from pidz comin from pipe because the process are execute with konsole so the pids are different
    for (i = 7; i < 10; i++)
    {
        pids_from_master[i] = atoi(argv[i]);
    }

    // manage pipe -------------------------------------------------------------

    // read all the pid from pipee fd1 = server, fd2 = input, fd3 = drone
    // fd1 stand in position 1 and 2
    int fd1[2];
    for (int i = 1; i < 3; i++)
    {
        fd1[i - 1] = atoi(argv[i]);
    }
    // fd2 stand in position 3 and 4
    int fd2[2];
    for (int i = 3; i < 5; i++)
    {
        fd2[i - 3] = atoi(argv[i]);
    }
    // fd3 stand in position 5 and 6
    int fd3[2];
    for (int i = 5; i < 7; i++)
    {
        fd3[i - 5] = atoi(argv[i]);
    }
    writeLog("The file descriptor fd1 received from master to watchdog are: %d %d ", fd1[0], fd1[1]);
    writeLog("The file descriptor fd2 received from master to watchdog are: %d %d ", fd2[0], fd2[1]);
    writeLog("The file descriptor fd3 received from master to watchdog are: %d %d ", fd3[0], fd3[1]);

    // part for the pipe fd1 (server) -------------------------------------
    // wtchdog need to read, close the write fd -> fd1[1]
    if (close(fd1[1]) < 0)
    {
        perror("close fd[1] wd");
        writeLog("ERROR ==> close fd1[1] wd %m ");
    }
    if (read(fd1[0], &pids[0], sizeof(pid_t)) < 0)
    {
        perror("read fd1[0] wd");
        writeLog(" ERROR ==> read fd1[0] wd %m ");
    }
    if (close(fd1[0]) < 0)
    {
        perror("close fd1[0] wd");
        writeLog(" ERROR ==> close fd1[0] wd %m ");
    }

    // part for the pipe fd2 (input) -----------------------------
    // wtchdog need to read, close the write fd -> fd2[1]
    if (close(fd2[1]) < 0)
    {
        perror("close fd2[1] wd");
        writeLog("ERROR ==> close fd2[1] wd %m ");
    }
    // read the pid value from pipe, the pid of input
    if (read(fd2[0], &pids[1], sizeof(pid_t)) < 0)
    {
        perror("read fd2[0] wd");
        writeLog(" ERROR ==> read fd2[0] wd %m ");
    }
    // close the read file descriptor fd2[0]
    if (close(fd2[0]) < 0)
    {
        perror("close fd3[0] wd");
        writeLog(" ERROR ==> close fd3[0] wd %m ");
    }

    // part of the pipe fd3 (drone) -------------------------------------------
    // close the write file descriptor fd3[1]
    if (close(fd3[1]) < 0)
    {
        perror("close fd3[0] wd");
        writeLog(" ERROR ==> close fd3[0] wd %m ");
    }

    // read the pid value from pipe, the pid of drone
    if (read(fd3[0], &pids[2], sizeof(pid_t)) < 0)
    {
        perror("read fd3[0] wd");
        writeLog(" ERROR ==> read fd3[0] wd %m ");
    }
    // close the read file descriptor fd[0]
    if (close(fd3[0]) < 0)
    {
        perror("close fd3[0] wd");
        writeLog(" ERROR ==> close fd3[0] wd %m ");
    }

    writeLog("Watchdog read server pid: %d ", pids[0]);
    writeLog("Watchdog read input pid: %d ", pids[1]);
    writeLog("Watchdog read drone pid: %d ", pids[2]);
    while (1)
    {
        counter = 0; // Inizialize the counter every time enter in the loop
        /* send a signal to all process */
        for (i = 0; i < num_ps; i++)
        {
            /* send signal to all process*/
            if (kill(pids[i], SIGUSR1) != 0)
            {
                writeLog("ERROR ==> kill signal SIGUSR1 wd %m ");
            }
            /* increment the counter when send the signal*/
            counter++;
            sleep(1);
            printf("%d", counter);
            fflush(stdout);
            if (counter == 0)
            {
                /* in this case the proccess is alive*
                /* write into logfile*/
                writeLog("Counter == 0 ");
                writeLog("Process %d is alive ", pids[i]);
            }
            else
            {
                /*The proces doesn't work*/
                /*kill all process*/
                for (int j = 0; j < num_ps; j++)
                {
                    if (kill(pids_from_master[j], SIGKILL) == 0)
                    {
                        /*write into logfile that wd close the process*/
                        writeLog("process %d is closed by WATCHDOG ", pids_from_master[j]);
                    }
                    else
                    {
                        writeLog("ERROR ==> kill signal SIGKILL wd %m ");
                    }
                }
                for (int k = 0; k < num_ps; k++)
                {
                    if (kill(pids_from_master[k], SIGKILL) == 0)
                    {
                        /*write into logfile that wd close the process*/
                        writeLog("process %d is closed by WATCHDOG ", pids_from_master[k]);
                    }
                    else
                    {
                        writeLog("ERROR ==> kill signal SIGKILL wd %m ");
                    }
                }

            }
            sleep(1);
        }
    }
    return 0;
}