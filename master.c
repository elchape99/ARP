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

#define NUMPROCESS 4
#define SERVER 0
#define DRONE 1
#define INPUT 2
#define WATCHDOG 3

int newprocess (int num, char* pname, char** arglist[])
{
    pid_t pid;
    if ((pid = fork()) == -1) {
        perror("fork");
        printf("error in fork of %d\n",num);
        return -1;
    }
    if (pid == 0) {
        // child process
        if (execvp(pname, arglist) == -1){
            perror("exec failed");
            printf("\nerror in exec of %d\n",num);
            return -2;
        }
        else{
        //parent process
            return pid;
        }
    }
}


int main ()
{
    int pipes[NUMPROCESS][2];
    int i;
    int waitpid[NUMPROCESS];
    pid_t pid[NUMPROCESS]; // child pids
    char pidstr[NUMPROCESS][70]; //from int to string
    pid_t pid_des;
   
    //Inizialize the log file, inizialize with mode w, all the data inside will be delete
    
    FILE *logfile = fopen("logfile.txt", "w");
    if(logfile < 0){ //if problem opening file, send error
        perror("fopen: logfile");
        return 1;
    }else{
        //wtite in logfile
        time_t ctime;
        //obtain local time
        time(&current_time);
        fprintf(logfile, "%s => create master with pid %d\n", ctime(&ctime), getpid());
        fclose(logfile);
    }
    // now we start showing the description of th game
    
    printf("Welcome to the Drone Game!\n");
    if ((pid_des = fork()) == -1) {
        perror("fork description");
        return 2;
    }
    if (pid_des == 0) {
        // child description process
        char * argdes[] = {"konsole", "-e","./description", NULL};
        if (execvp("konsole", argdes) == -1){
            perror("exec failed");
            return -1;
        }
    }else{
        //parent process
        wait(NULL);
    }
    //Now we start the game, so it is needed to create all

    char * argserver[] = {NULL};
    pid[SERVER] = newprocess(SERVER, "./server", &argserver);

    char * argdrone[] = {NULL};
    pid[DRONE] = newprocess(DRONE, "./drone", &argdrone);

    char * arginput[] = {NULL};
    pid[INPUT] = newprocess(INPUT, "./input", &arginput);

    sleep(1);

    for (i = 0; i< NUMPROCESS; i++) {
        sprintf(pidstr[i], "%d", pid[i]);
    }

    char * argwatchdog[] = {pidstr[SERVER], pidstr[DRONE], pidstr[INPUT], NULL};
    pid[WATCHDOG] = newprocess(WATCHDOG, "./watchdog", &argwatchdog);

     



    for (i = 0; i< NUMPROCESS; i++) {
        waitpid[i] = waitpid(pid[i],NULL,0);
        printf("process %d terminated\n",i);
    }
    return 0;
}