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

void sigusr1Handler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGUSR1){
        /*send a signal SIGUSR2 to watchdog */
        kill(info->si_pid, SIGUSR2);
    }
}


int main(int argc, char *argv[]) {

    //write into logfile
    FILE *logfile = fopen("logfile.txt", "a");
    if(logfile < 0){ 
        //error opening log file
        perror("fopen: logfile");
        return 1;
    }else{
        //wtite in logfile
        time_t current_time;
        //obtain local time
        time(&current_time);
        fprintf(logfile, "%s => spawn INPUT with pid %d\n", ctime(&current_time), getpid());
        fclose(logfile);
    }
       

    //configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1){ 
        perror("sigaction");
        return -1;
    }

   
    while(1){        
        /* write the code of the server here*/
        sleep(1);

    }
    
    return 0;

}