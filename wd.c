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

void writeLogFile(char *contenuto) {
    // Open file in append way
    FILE *logfile = fopen("logfile.txt", "a");

    // Check the file opening
    if (logfile < 0) {
        perror("fopen: logfile");
        return;
    }

    // Write on file
    time_t current_time;
    //obtain local time
    time(&current_time);  
    fprintf(logfile, "%s", ctime(&current_time));
    fprintf(logfile, "%s", contenuto);

    // Close file descriptor
    fclose(file);
}

/*  when signal arrive counter --
    when wd send kill counter ++ */

int counter = 0;

/*signal hadler function*/
void sigusr2Handler(int signum, siginfo_t *info, void *context) {
    if(signum == SIGUSR2){

        //write into logfile
        FILE *logfile = fopen("logfile.txt", "a");
        if(logfile < 0){ 
            //error opening log file
            perror("fopen: logfile");
        }else{
            //wtite in logfile
            time_t current_time;
            //obtain local time
            time(&current_time);  
            fprintf(logfile, "%s => watchdog receive signal from %d\n", ctime(&current_time), info->si_pid);
            if(fclose(logfile) < 0){
                perror("fclose: logfile");
            }
            counter --;
        }
    }    
}


int main(int argc, char *argv[])  
{

    pid_t pids[argc];
    int i;
/*
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

        if(fclose(logfile) < 0){
            perror("fclose: logfile");
            return 1;
            } 

        for ( i = 0; i< argc; i++){
            fprintf(logfile, "received pid :%s as argument in wd\n", argv[i]);
        }
    }
    */

   char contenuto[] = ("=> spawn watchdog with pid %d\n", getpid());

   

    /*configure the handler for sigusr2*/
    struct sigaction sa_usr2;
    sa_usr2.sa_sigaction = sigusr2Handler;
    sa_usr2.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR2, &sa_usr2, NULL) == -1){ 
        perror("sigaction");
        return -1;
    }
    /* convert the pid in argv from dtring to int*/
    for (i = 0; i < argc; i++){
        pids[i] = atoi(argv[i]);
    }

    while(1){
        /* send a signal to all process */
        for (i = 0; i< argc; i++){  
            /* send signal to all process*/
            kill(pids[i], SIGUSR1);
            /* increment the counter when send the signal*/
            counter ++; 
            sleep(2);

            printf("%i", counter);

            if(counter == 0){
                /* in this case the proccess is alive*/
                /* write into logfile*/   
                FILE *logfile = fopen("logfile.txt", "a");
                if(logfile < 0){ 
                    // error opening log file
                    perror("fopen: logfile");
                    return 1;
                }else{
                    // wtite in logfile
                    time_t current_time;
                    // obtain local time
                    time(&current_time);  
                    fprintf(logfile, "%s the process %s is alive\n", ctime(&current_time), argv[i]);
                    if (fclose(logfile) == -1){
                        perror("fclose");
                        return 1;

                    }                
                }       
            }else{
                /*The proces doesn't work*/
                /*kill all process*/
                for (i = 0; i < argc; i++){
                    kill(pids[i], SIGKILL);
                    /*write into logfile*/   
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
                        fprintf(logfile, "%s the process %s is cloded by watchdog\n", ctime(&current_time), argv[i]);
                        if (fclose(logfile) == -1){
                            perror("fclose");
                            return 1;                        
                        }
                    }        
                }
                exit(0);
            }         
            sleep(1);
        }     
    }
    return 0;
}