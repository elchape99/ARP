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

void sigusr2_handler(){ 

}


int main(int argc,char *argv[])         
{

    pid_t pids[argc];
    int i;

    for (i = 0; i< argc; i++){  
        pids[i] = atoi(argv[i]);
        printf("pid of the %i process is: %i\n",i, pids[i]);
    }
    while(1){
        /*send a signal to all process */
        for (i = 0; i< argc; i++){  
            kill(pids[i], SIGUSR1); // This send a signal to a parent process, In parent process I need to put a handler_signals
            sleep(1);
        }
        
    }

    return 0;
}