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

void sigusr1_handler(){ // I will call this function when I send the signal with kill in watchdog
    /*send a signal to watchdog SIGUSR2*/
    printf("\nsignal handler SERVER");
}

int main(int argc, char *argv[]) {
    printf ("I'm server pid %i\n", getpid());
    
    // define the structure for signal
    struct sigaction sa_usr1 = {0}; // I put all the field to 0
    //memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = sigusr1_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1){
        perror("sigaction");
        return -1;
    }
    // Infinite loop
    while(1){
       sleep(1);      
    }
    
    return 0;



}