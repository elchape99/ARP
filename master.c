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


/*This function do an exec in child process*/
int spawn(const char * program, char ** arg_list) {
 
  pid_t child_pid = fork();
  if (child_pid != 0)
    //main process
    return child_pid;

   else {
    //child process
    if (execvp (program, arg_list) == -1){
        perror("exec failed");
        return -1;
    }
 }
}

int main() {
    /* 
    The master spawn all the process with watchdog at the end, so it can pass in argv all the process pid. 
    After throught pipe it will pass at all process the watchdog pid.
    */
   pid_t pid_des;
   

    //Inizialize the log file, inizialize with mode w, all the data inside will be delete
    
    FILE *logfile = fopen("logfile.txt", "w");
    if(logfile < 0){ //if problem opening file, send error
        perror("fopen: logfile");
        return 1;
    }else{
        //wtite in logfile
        time_t current_time;
        //obtain local time
        time(&current_time);
        fprintf(logfile, "%s => create master with pid %d\n", ctime(&current_time), getpid());
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
        char * arg_list_des[] = {NULL};
        if (execvp("./description", arg_list_des) == -1){
            perror("exec failed");
            return -1;
        }
    }else{
        //parent process
        wait(NULL);
    }
    
    //now are implemented 3 processes, server, drone, input plus watchdog

    //inizialize the variabiles needed
    int num_ps = 3;       
    pid_t child_pids [num_ps];

    //this array will need for convert the pisds number in string
    char child_pids_str [num_ps][80];

    //server process
    char * arg_list_server[] = {NULL};
    child_pids[0] = spawn("./server", arg_list_server);
    
    //drone process
    char * arg_list_drone[] = {NULL};
    child_pids[1] = spawn ("./drone", arg_list_drone);

    //keyboard_namager process
    char * arg_list_i[] = {NULL};
    child_pids[2] = spawn ("./input", arg_list_i);

    sleep(0.5);
    //now need to convert all the integer pid in a string, than pass this string as a argv to watchdog process
    //convert all the pid process fron int to string using sprintf
    for(int i = 0; i < num_ps; i++){   
        sprintf(child_pids_str[i], "%d", child_pids[i]);
    }

    // spawn watchdog, and pass as argument all the pid of processes
    char * arg_list_wd[] = {child_pids_str[0], child_pids_str[1], child_pids_str[2], NULL};
    pid_t pid_wd = spawn ("./wd", arg_list_wd);


    
    int wait_ps[num_ps];
    for (int i = 0; i<num_ps; i++)
    {
        wait_ps[i] = waitpid(child_pids[i], NULL, 0);
        waitpid(pid_wd, NULL, 0);
        printf("process %i is close\n", wait_ps[i]);
    }
    
    /*
    while (1)
    {   
        if(wait(NULL) == -1){
            break;
        }
    }
    */

    return 0;
}
