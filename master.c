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
    /* The master spawn all the process with watchdog at the end, so it can pass in argv all the process pid. 
    After throught pipe it will pass at all process the watchdog pid.
     */
    //now are implemented 3 processes, server, drone , keyboard_manager plus watchdog

    //inizialize the variabiles needed
    int num_ps = 3;       
    pid_t child_pids [num_ps];
    //this array will need for convert the pisds number in string
    char child_pids_str [num_ps][80];

    //create all the pipe I need for send to process the watchdog pid 
    int fd_s[2];
    int fd_d[2];
    int fd_i[2];

    // pipe for cominicate with server
    if(pipe(fd_s) == -1){
        perror("pipe");
        return -1;
    }
    // pipe for comunicate with drone
    if(pipe(fd_d) == -1){
        perror("pipe");
        return -1;
    }
    // pipe for comunicate with input
    if(pipe(fd_i) == -1){
        perror("pipe");
        return -1;
    }

    //server process
    char * arg_list_server[] = {NULL};
    child_pids[0] = spawn("./server", arg_list_server);
    
    //drone process
    char * arg_list_drone[] = {NULL};
    child_pids[1] = spawn ("./drone", arg_list_drone);

    //keyboard_namager process
    char * arg_list_k_m[] = {NULL};
    child_pids[2] = spawn ("./input", arg_list_k_m);

    //now need to convert all the integer pid in a string, than pass this string as a argv to watchdog process
    //convert all the pid process fron int to string using sprintf
    for(int i = 0; i < num_ps; i++){   
        sprintf(child_pids_str[i], "%d", child_pids[i]);
    }

    // spawn watchdog, and pass as argument all the pid of processes
    char * arg_list_wd[] = {child_pids_str[0], child_pids_str[1], child_pids_str[2], NULL};
    pid_t pid_wd = spawn ("./wd", arg_list_wd);


    

    int pippo[num_ps];
    for (int i = 0; i<num_ps; i++)
    {
        waitpid(pid_wd, NULL, 0);
        pippo[i] = waitpid(child_pids[i], NULL, 0);
        printf("process %i is close\n", pippo[i]);
    }


    return 0;
}
