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

int write_logfile (char *str){
    /*write into logfile*/
    FILE *logfile = fopen("logfile.txt", "a");
    if(logfile < 0){ 
        //error opening log file
        perror("fopen: logfile");
        return -1;
    }else{
        //wtite in logfile
        time_t current_time;
        //obtain local time
        time(&current_time);  
        fprintf(logfile, "%s", ctime(&current_time));
        fprintf(logfile,str);
        fclose(logfile);
        return 0;
    }

}

/*  when signal arrive counter --
    when wd send kill counter ++ */
int counter = 0;

void sigusr2Handler(int signum, siginfo_t *info, void *context) {
    if(signum == SIGUSR2){
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
            fprintf(logfile, "%s => watchdog receive signal from %d\n", ctime(&current_time), info->si_pid);
            fclose(logfile);
            counter --;
        }
    }    
}


int main(int argc, char *argv[])  
{

    pid_t pids[argc];
    int i;

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
        fprintf(logfile, "%s => spawn watchdog with pid %d\n", ctime(&current_time), getpid());
        fclose(logfile);
    }
   

    /*configure the handler for sigusr2*/
    struct sigaction sa_usr2;
    sa_usr2.sa_handler = sigusr2Handler;
    sa_usr2.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR2, &sa_usr2, NULL) == -1){ 
        perror("sigaction");
        return -1;
    }
   
    for (i = 0; i< argc; i++){  
        argv[i] = atoi(argv[i]);
        printf("pid of the %i process is: %i\n",i, pids[i]);
    }

    while(1){
        /*send a signal to all process */
        for (i = 0; i< argc; i++){  
            printf("%d", pids[i]);

            kill(argv[i], SIGUSR1); // This send a signal to a parent process, In parent process I need to put a handler_signals
            
            sleep(1);
        }
        

        
    }

    return 0;
}