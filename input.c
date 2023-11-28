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

int main (int argc, char* argv[])
{
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
        fclose(inputfile);
    }
    // Managing signal for the watchdog
    
    return 0;
}
