#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <errno.h>


/*This function do an exec in child process*/
int spawn(const char * program, char ** arg_list) {
 
  pid_t child_pid = fork();
  if (child_pid != 0){
    //child process
    char messaggio[100] = "exec failed";
    strcat(messaggio, program);
    if (execvp (program, arg_list) == -1){
        perror(messaggio);
        return -1;
    }
   }else{
    //main process
    return child_pid;
    
 }
}

int main() {
    /* The master spawn all the process with watchdog at the end, so it can pass in argv all the process pid. 
    After throught pipe it will pass at all process the watchdog pid.
     */

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

    int pipe_fd[2]; // creazione dell'array per le pipe
    if((pipe(pipe_fd)) < 0){
        perror("errore creazione pipe");
    }

    char str_pipe_fd[2][20]; // conversione fd pipe in stringhe
    for (int i = 0; i < 2; i++)
    {
        sprintf(str_pipe_fd[i], "%d", pipe_fd[i]);
    }
    
    //now are implemented 3 processes, server, drone, input plus watchdog

    //inizialize the variabiles needed
    int num_ps = 3;       
    pid_t child_pids [num_ps];

    //this array will need for convert the pisds number in string
    char child_pids_str [num_ps][80];

    //server process
    char * arg_list_server[] = {"konsole", "-e","./server", NULL};
    child_pids[0] = spawn("konsole", arg_list_server);
    
    //drone process -----------------------------------------------------------------------
    char * arg_list_drone[] = {"konsole", "-e","./drone", str_pipe_fd[0], str_pipe_fd[1], NULL};
    child_pids[1] = spawn ("konsole", arg_list_drone);

    //keyboard_namager process ------------------------------------------------------------
    char * arg_list_i[] = {"konsole", "-e","./input", str_pipe_fd[0], str_pipe_fd[1], NULL};
    child_pids[2] = spawn ("konsole", arg_list_i);

    // ---------------------------------------------------------------------------------//

    sleep(0.5);
    //now need to convert all the integer pid in a string, than pass this string as a argv to watchdog process
    //convert all the pid process fron int to string using sprintf
    // for(int i = 0; i < num_ps; i++){   
    //     sprintf(child_pids_str[i], "%d", child_pids[i]);
    // }

    // // spawn watchdog, and pass as argument all the pid of processes
    // char * arg_list_wd[] = {"konsole", "-e","./wd", child_pids_str[0], child_pids_str[1], child_pids_str[2], NULL};
    // pid_t pid_wd = spawn ("konsole", arg_list_wd);

    
    // avoid master terminating
    getchar();
    
    

    return 0;
}
