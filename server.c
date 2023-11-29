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

void handle_sigusr(int sig, siginfo_t *siginfo, void *context)
{
    printf("Signal %d received from process %d\n", sig, siginfo->si_pid);
    if (sig == SIGUSR1) {
        watchdog = siginfo->si_pid;
        kill(siginfo->si_pid, SIGUSR2) //send signal to the watchdog
    }
    
}
float weight = 0;
float screw = 0; // attrito

typedef struct {
    float x;
    float y;
} position;

typedef struct {
    float fx;
    float fy;
} strength;

typedef struct {
    float vx;
    float vy;
} velocity;

int main (int argc, char* argv[])
{
    pid_t mpid;
    int i,children = 2;
    int mfd[children];
    char* pidstr[children];
    void* ptr;
    int shmmid;
    
    strength *force;
    velocity *vel;
    position *pos;
    // server con shared memory riceve tutto da drone.c e manda alla mappa 
    // per muovere il drone 

    if ((pipe(mfd)) < 0) {
        perror("pipe map ncurses");
        return 2;
    }
    
    if ((mpid = fork()) == -1) {
        perror("fork map");
        return 1;
    }
    for (i = 0; i < children; i++) {
        sprintf(pidstr[i], "%d", inpfd[i]);
    }
    if (mpid == 0) {
        char* argvm[] = {"konsole", "-e","./map",pidstr[0],pidstr[1], NULL};
        // Add maybe some arguments to argvm
        execvp("konsole",argvm);
        printf("error in exec of map\n");
        return -2;
    }
    close(mfd[0]);

    // Shared memory fai dalle slide del prof

    shmmid = shmget(KEY,sizeof(strength) + sizeof(velocity) + sizeof(position), IPC_CREAT | 0666);
    if (shmmid == -1) {
        printf("Shared memory failed\n");
        return 1;
    }
    // ftruncate(shmmid, sizeof(message));
    force = (strength*) shmat(shmmid, NULL, 0);
    if ((void*) force == (*void)-1) {
        perror("shmat");
        return 2;
    }
    vel = (velocity*)(force + 1);
    if ((void*) vel == (*void)-1) {
        perror("shmat");
        return 2;
    }
    pos = (position*)(vel + 1);
    if ((void*) pos == (*void)-1) {
        perror("shmat");
        return 2;
    }


    ptr = mmap(0, sizeof(message), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        exit(-1);
    }






    while (pos.x != -1 && pos.y != -1) {
        if ((write(mfd[1], &pos, sizeof(position))) < 0) {
            perror("write map");
            return 3;
        }
        //implementare accesso a memoria condivisa con i semafori
        // Ricorda di mettere gestione errori
    }



    shmdt(force);
    shmdt(vel);
    shmdt(pos);
    shmctl(shmmid, IPC_RMID, NULL);
    close(mfd[1]);
    wait(NULL);
    return 0;
}