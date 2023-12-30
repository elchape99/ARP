#include <ncurses.h>
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
#include <stdarg.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define DRONE_ICON 'X'

/* function for write in logfile*/
void writeLog(const char *format, ...)
{

    FILE *logfile = fopen("logfile.txt", "a");
    if (logfile == NULL)
    {
        perror("server: error opening logfile");
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, format);

    time_t current_time;
    time(&current_time);

    fprintf(logfile, "%s => ", ctime(&current_time));
    vfprintf(logfile, format, args);

    va_end(args);
    fflush(logfile);
    if (fclose(logfile) == -1)
    {
        perror("fclose logfile");
        writeLog("ERROR ==> server: fclose logfile");
    }
}
 
// Inserire perror nella kill 
void sigusr1Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR1)
    {
        /*send a signal SIGUSR2 to watchdog */
        // printf("SERVER sig handler");
        kill(info->si_pid, SIGUSR2);
        writeLog("SERVER, pid %d, received signal from wd pid: %d ", getpid(), info->si_pid);
    }
}

WINDOW *create_new_window(int row, int col, int ystart, int xstart); // creazione delle finestre
void move_drone_icon(int row, int col, WINDOW *map_wind_pointer);

// Struct for the drone position
typedef struct
{
    double Xpos;
    double Ypos;
} posizione;

#define SHM_SIZE sizeof(posizione) // Dimensione della struttura

int main(int argc, char *argv[])
{
    int i;
    // actual pid of the server
    pid_t server_pid = getpid();
    // write into logfile
    writeLog("SERVER create with pid %d ", server_pid);

    /*Take the fd for comunicating with watchdog from argv[] the position are in 1 and 2 position*/
    int fd1[2];
    for (i = 1; i < 3; i++)
    {
        fd1[i - 1] = atoi(argv[i]);
    }
    writeLog("the file descriptor received from master to server are: %d %d", fd1[0], fd1[1]);

    //close the read fiel descriptor fd1[0]
    if (close(fd1[0]) < 0)
    {
        perror("server: close fd1[1]");
        writeLog("ERROR ==> server: close fd1[0], %m ");
    }
    // write the pis in the pipe
    if (write(fd1[1], &server_pid, sizeof(server_pid)) < 0)
    {
        perror("server: write fd1[1],");
        writeLog("ERROR ==> server, write fd1[1] %m ");
    }
    if (close(fd1[1]) < 0)
    {
        perror("server: close fd1[1]");
        writeLog("ERROR ==> server: close fd1[1], %m ");
    }


    //////////////////////////////////// CONTORLLARE TUTTI  MESSAGGI DI ERRORE DA QUI IN AVANTI ///////////////////////77

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("sigaction");
        return -1;
    }

    // Initialization of shared memory
    key_t key = ftok("/tmp", 's');                       // Genera la chiave IPC
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666); // Ottieni l'ID della memoria condivisa
    if (shmid == -1)
    {
        perror("Errore nell'ottenere l'ID della memoria condivisa");
        writeLog("ERROR ==> shmget server: %m ");

        exit(EXIT_FAILURE);
    }
    posizione *pos = (posizione *)shmat(shmid, NULL, 0); // Attacca la memoria condivisa
    if (pos == (void *)-1)
    {
        perror("Errore nell'attaccamento della memoria condivisa");
        writeLog("ERROR ==> shmat server: %m ");

        exit(EXIT_FAILURE);
    }

    // manage semahore ---------------------------------------------
    const char *sem_name = "/sem1";

    sem_t *sem1 = sem_open(sem_name, O_CREAT, 0666, 0);

    if (sem1 == SEM_FAILED)
    {
        perror("sem_open server: ");
        writeLog("ERROR ==> sem_open server %m ");
        exit(EXIT_FAILURE);
    }

    // parte legata ad ncurses per il server
    posizione drone_pose;
    posizione drone_pose_old;
    drone_pose_old.Ypos = 0.0;
    drone_pose_old.Xpos = 0.0;

    WINDOW *map_window; // definizione puntatore alla mappa
    int Srow, Scol;     // righe e colonne massime dello schermo
    int rowSH, colSH;
    // initialization row
    initscr();
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, Srow, Scol);
    rowSH = Srow / 2; // definisco gli shift per traslare (0,0) al centro dello schermo
    colSH = Scol / 2;

    // creo la finestra della mappa
    map_window = create_new_window(Srow, Scol, 0, 0);
    move_drone_icon(rowSH - (int)pos->Ypos, colSH + (int)pos->Xpos, map_window);

    while (1)
    {
        // creo la variabile posizione in modo randomico
        if (sem_wait(sem1) < 0)
        {
            perror("sem_wait server");
            writeLog("ERROR ==> sem_wait server %m ");
        }
        drone_pose.Ypos = pos->Ypos;
        drone_pose.Xpos = pos->Xpos;
        if (sem_post(sem1) < 0)
        {
            perror("sem_post server ");
            writeLog("ERROR ==> sem_post server %m ");
        }

        if ((int)drone_pose.Ypos == (int)drone_pose_old.Ypos && (int)drone_pose.Xpos == (int)drone_pose_old.Xpos)
        {
            continue;
        }
        else
        {
            move_drone_icon(rowSH - (int)drone_pose.Ypos, colSH + (int)drone_pose.Xpos, map_window);
        }

        drone_pose_old.Ypos = drone_pose.Ypos;
        drone_pose_old.Xpos = drone_pose.Xpos;

        // usleep(50000);
        //  printf("Posizione: (%lf, %lf)\n", pos->Xpos, pos->Ypos);
        //  fflush(stdout);
    }

    shmdt(pos);                    // Stacca la memoria condivisa
    shmctl(shmid, IPC_RMID, NULL); // Rimuove la memoria condivisa quando il server termina

    return 0;
}

WINDOW *create_new_window(int row, int col, int ystart, int xstart)
{
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    wrefresh(local_window);
    return local_window;
}

void move_drone_icon(int row, int col, WINDOW *map_wind_pointer)
{
    wclear(map_wind_pointer); // pulisco la finestra da qualsiasi cosa
    box(map_wind_pointer, 0, 0);
    mvwaddch(map_wind_pointer, row, col, DRONE_ICON);
    wrefresh(map_wind_pointer);
}
