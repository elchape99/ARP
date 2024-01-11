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
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "arplib.h"

#define INP_NUM 8

/////--- Functions heder --------------------------------------------------------------------------------------

/* function for write in logfile*/
void writeLog(const char *format, ...);

/* signal handler functions, when receive a ignal from watchdog sena bach a signal*/
void sigusr1Handler(int signum, siginfo_t *info, void *context);

/* function for the dynamics of the drone */
double *generate_input_vect(double **vect_pointer, char ch); // trasformare input in forza
double *put_vector_to_zero(double **vect_pointer);           // inizializzazione vettore input

double *generate_x_force(double *vect_pointer, double *force); // somma input nella forza X
double *generate_y_force(double *vect_pointer, double *force); // somma input nella forza Y

double *velocity(double Force, double initial_velocity, double *new_vel);     // data una forza calcola velocità sull'asse
double *position(double Velocity, double initial_position, double *new_pose); // data una velocità calcola posizione sull'asse

int main(int argc, char *argv[])
{

    pid_t drone_pid = getpid();
    // write into logfile
    writeLog("DRONE create with pid %d ", drone_pid);
    int i;

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("sigaction");
        return -1;
    }

    // manage pipe --------------------------------------------------------------------------
    // pipe for writing the pid to watchdog
    int fd3[2];
    // file descritor are in position 3 and 4 of argv[]
    for (i = 1; i < 3; i++)
    {
        fd3[i - 1] = atoi(argv[i]);
    }
    writeLog("DRONE riceived fd3: %d, %d", fd3[0], fd3[1]);
    // close the read file descriptor fd2[0]
    if (close(fd3[0]) < 0)
    {
        perror("drone: close fd3[0]");
        writeLog("ERROR ==>d rone: close fd3[0] %m ");
    }
    // write the pid inside the pipe
    if (write(fd3[1], &drone_pid, sizeof(drone_pid)) < 0)
    {
        perror("drone: write fd3[1] ");
        writeLog("ERROR ==> drone: write fd3[1] %m ");
    }
    // close the write file descriptor fd2[1]
    if (close(fd3[1]) < 0)
    {
        perror("drone: close fd3[1] drone");
        writeLog("ERROR ==> drone:  close fd3[1] drone %m ");
    }
    //// pipe for comunication between drone -> server, are in position 3, 4
    int fdd_s[2];
    // file descritor are in position 3 and 4 of argv[]
    for (i = 3; i < 5; i++)
    {
        fdd_s[i - 3] = atoi(argv[i]);
    }
    // close the read file descriptor fdd_s[0]
    if (close(fdd_s[0]) < 0)
    {
        perror("drone: close fdd_s[0] ");
        writeLog("ERROR ==> drone: close fd[1] %m ");
    }
    writeLog("SERVER value of fdd_s are: %d %d ", fdd_s[0], fdd_s[1]);

    //// pipe for comunication between server -> drone, are in position 5, 6
    int fds_d[2];
    for (i = 5; i < 7; i++)
    {
        fds_d[i - 5] = atoi(argv[i]);
    }
    // close the write file descriptor fds_d[1]
    if (close(fds_d[1]) < 0)
    {
        perror("drone: close fds_d[1]");
        writeLog("ERROR ==> drone close fds_d[1] %m ");
    }
    writeLog("SERVER value of fds_d are: %d %d ", fds_d[0], fds_d[1]);

    // inizializzazione delle variabili per la dinamica --------------------------------------------------------------
    double *input_vect = malloc(sizeof(double) * INP_NUM); // riservo la memoria per il vettore di input
    for (int i = 0; i < INP_NUM; i++)
    {
        input_vect[i] = 0.0;
        printf("%.2f  ", input_vect[i]);
        fflush(stdout);
    }

    double XForce = 0.0, YForce = 0.0;
    double *XForce_p = &XForce, *YForce_p = &YForce;

    double Xvel = 0.0, Yvel = 0.0, Xpos = 0.0, Ypos = 0.0;
    double *Xvel_p = &Xvel, *Xpos_p = &Xpos, *Yvel_p = &Yvel, *Ypos_p = &Ypos;

    int retVal_read;
    double total_force[2] = {0.0};
    double drone_position[2] = {0.0};
    double drone_position_old[2] = {0.0};

    // definizione variabili per la select
    int retVal_sel;
    fd_set read_fd;
    struct timeval time_sel;

    // ciclo infinito per ricever input dal server

    // DRONE MANDA POSIZIONE DALLA PIPE fdd_s
    while (1)
    {
        // ridefinisco ad ogni ciclo --> azione select retVal_sel == 0
        FD_ZERO(&read_fd);
        FD_SET(fds_d[0], &read_fd); // definisco il set dei fd da controllare

        time_sel.tv_sec = 0; // timeout settatto a 0.5 secondi
        time_sel.tv_usec = 30000;

        if ((retVal_sel = select(fds_d[0] + 1, &read_fd, NULL, NULL, &time_sel)) < 0)
        {
            perror("drone: error select: "); // controllo errori
            writeLog("ERROR ==> drone: select fds_d[0] %m ");
        }
        else if (retVal_sel == 0)
        {
            printf("no new data\n"); // pipe vuota
            fflush(stdout);
        }
        else
        { // nuovi dati disponibili
            if ((retVal_read = read(fds_d[0], total_force, sizeof(double) * 2)) < 0)
            {
                perror("errore read"); // controllo errore read
                writeLog("ERROR ==> drone: read fds_d[0] %m ");
            }
            else
            {
                printf("controllo lettura: %d, ( %f, %f )\n", retVal_read, total_force[0], total_force[1]); // controllo valori letti
                fflush(stdout);
            }
        }

        // genero valori di forza sui due assi
        XForce = total_force[0];
        YForce = total_force[1];

        // genera velocità
        Xvel_p = velocity(*XForce_p, *Xvel_p, Xvel_p);
        Yvel_p = velocity(*YForce_p, *Yvel_p, Yvel_p);
        // printf("controllo valori: yf:%.2f, yVel:%.2f\n", *YForce_p, *Yvel_p);
        fflush(stdout);

        // genera posizione
        Xpos_p = position(*Xvel_p, *Xpos_p, Xpos_p);
        Ypos_p = position(*Yvel_p, *Ypos_p, Ypos_p);

        drone_position[0] = Xpos;
        drone_position[1] = Ypos;

        if (drone_position[0] == drone_position_old[0] && drone_position[1] == drone_position_old[1])
        {
            printf("don't write \n");           
        }
        else
        {
            // sending force data to the server process, trogh the pipe fdd_s[1]
            if (write(fdd_s[1], drone_position, sizeof(double) * 2) < 0)
            {
                perror("drone: write fdd_s[1] ");
                writeLog("==> ERROR ==> drone: write dd_s[1] %m ");
            }

            printf("%f, %f\n", drone_position[0], drone_position[1]);
            fflush(stdout);
            writeLog("drone: %f, %f", drone_position[0], drone_position[1]);
             drone_position_old[0] = drone_position[0];
            drone_position_old[1] = drone_position[1];
        }
    }

    // close the read file descriptor for fdd_s
    if (close(fdd_s[0]) == -1)
    {
        perror("drone: close fdd_s[0]");
        writeLog("==> ERROR ==> drone: clse fdd_s[0] %m ");
    }
    // close the write file descriptor fd2[1]
    if (close(fds_d[1]) == -1)
    {
        perror("drone: close fds_d[1]");
        writeLog("==> ERROR ==> drone: close fds_d[1] %m ");
    }
    return 0;
}

////--- Function section --------------------------------------------------------------

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

void sigusr1Handler(int signum, siginfo_t *info, void *context)
{
    if (signum == SIGUSR1)
    {
        /*send a signal SIGUSR2 to watchdog */
        kill(info->si_pid, SIGUSR2);
        writeLog("DRONE, pid: %d, received signal from wd pid: %d ", getpid(), info->si_pid);
    }
}

double *generate_input_vect(double **vect_pointer, char ch)
{
    switch (ch)
    {
    case 'w':
        (*vect_pointer)[7] = (*vect_pointer)[7] + 1;
        break; // per ogni possibile input aumento il valore di un vettore
    case 'e':
        (*vect_pointer)[0] = (*vect_pointer)[0] + 1;
        break; // caso 'q' --> programma termina o si blocca
    case 'r':
        (*vect_pointer)[1] = (*vect_pointer)[1] + 1;
        break; // caso default da definire
    case 'f':
        (*vect_pointer)[2] = (*vect_pointer)[2] + 1;
        break;
    case 'v':
        (*vect_pointer)[3] = (*vect_pointer)[3] + 1;
        break;
    case 'c':
        (*vect_pointer)[4] = (*vect_pointer)[4] + 1;
        break;
    case 'x':
        (*vect_pointer)[5] = (*vect_pointer)[5] + 1;
        break;
    case 's':
        (*vect_pointer)[6] = (*vect_pointer)[6] + 1;
        break;
    case 'd':
        *vect_pointer = put_vector_to_zero(&(*vect_pointer));
        break;
    case 'q':
        sleep(5);
    default:
        break;
    }

    return *vect_pointer;
}

double *put_vector_to_zero(double **vect_pointer)
{
    for (int i = 0; i < INP_NUM; i++)
    {
        (*vect_pointer)[i] = 0.0; // inizializzazione
    }
    return *vect_pointer;
}

double *generate_x_force(double *vect_pointer, double *force)
{
    // ogni elemento rapprensenta una direzione per il drone
    *force = vect_pointer[2] - vect_pointer[6] + vect_pointer[1] / 2.0 + vect_pointer[3] / 2.0 - vect_pointer[7] / 2.0 - vect_pointer[5] / 2.0;
    return force;
}

double *generate_y_force(double *vect_pointer, double *force)
{
    *force = vect_pointer[0] - vect_pointer[4] + vect_pointer[1] / 2.0 - vect_pointer[3] / 2.0 + vect_pointer[7] / 2.0 - vect_pointer[5] / 2.0;
    return force;
}

// le due seguenti funzioni eseguono un integrazione numerica approssimata forza(accelerazione)->velocità->posizione
double *velocity(double Force, double initial_velocity, double *new_vel)
{
    double dtime_m = 0.1 / 0.5, frict_k = -0.5;

    // printf("in -- %.2f + %f * (%.2f + %f * %.2f) --- %.2f\n", initial_velocity, dtime_m, Force, frict_k, initial_velocity, *new_vel);
    *new_vel = initial_velocity + dtime_m * (Force + frict_k * initial_velocity);
    // printf("out-- %.2f --\n", *new_vel);
    return new_vel;
}

double *position(double Velocity, double initial_position, double *new_pose)
{
    double dtime = 0.1;

    *new_pose = initial_position + dtime * Velocity;
    return new_pose;
}
