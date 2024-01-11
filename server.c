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
#include "arplib.h"

#define DRONE_ICON 'X'
#define MAX_OBST_ARR_SIZE 10
#define MAX_TARG_ARR_SIZE 10
#define k 0.2

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

int main(int argc, char *argv[])
{
    // variable usefull for the for cycle
    int i;
    // actual pid of the server
    pid_t server_pid = getpid();
    // write into logfile
    writeLog("SERVER create with pid %d ", server_pid);

    // configure the handler for sigusr1
    struct sigaction sa_usr1;
    sa_usr1.sa_sigaction = sigusr1Handler;
    sa_usr1.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("sigaction");
        writeLog("ERROR ==> server: sigaction SIGUSR1 %m ");

        return -1;
    }

    ///////////// MANAGE PIPE /////////////////////////////////////////////////////////////
    // Take the fd for comunicating with master, it's position is 1,2 in argv[]
    int fd1[2];
    for (i = 1; i < 3; i++)
    {
        fd1[i - 1] = atoi(argv[i]);
    }
    writeLog("SERVER value of fd1 are: %d %d ", fd1[0], fd1[1]);

    // close the read fiel descriptor fd1[0]
    if (close(fd1[0]) < 0)
    {
        perror("server: close fd1[1]");
        writeLog("ERROR ==> server: close fd1[0], %m ");
    }
    // write the pid in the pipe
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

    //// Take the fdi_s for comunication input-server, fdi_s are in positions 3, 4
    int fdi_s[2];
    for (i = 3; i < 5; i++)
    {
        fdi_s[i - 3] = atoi(argv[i]);
    }
    // close the write file descriptor fdi_s[1], server only read from input
    if (close(fdi_s[1]) < 0)
    {
        perror("server: close fdi_s[1]");
        writeLog("ERROR ==> server: close fdi_s[1], %m ");
    }
    writeLog("SERVER value of fdi_s are: %d %d ", fdi_s[0], fdi_s[1]);

    //// Take the fdd_s for comunication drone-server, fdd_s are in positions 5, 6
    int fdd_s[2];
    for (i = 5; i < 7; i++)
    {
        fdd_s[i - 5] = atoi(argv[i]);
    }
    // This pipe is usefull for send and receive data from server to drone (is used for dynamics)
    writeLog("SERVER value of fdd_s are: %d %d ", fdd_s[0], fdd_s[1]);

    //// Take the fds_t dor the comunication between server -> drone, are in positions 11, 12
    int fds_d[2];
    for (i = 11; i < 13; i++)
    {
        fds_d[i - 11] = atoi(argv[i]);
    }
    // close the write file descriptor fds_d[1]
    if (close(fds_d[1]) < 0)
    {
        perror("drone: close fds_d[1]");
        writeLog("ERROR ==> drone close fds_d[1] %m ");
    }
    writeLog("SERVER value of fds_d are: %d %d ", fds_d[0], fds_d[1]);

    //// Take the fdt_s for comunication between targer-server, position 7, 8
    int fdt_s[2];
    for (i = 7; i < 9; i++)
    {
        fdt_s[i - 7] = atoi(argv[i]);
    }
    writeLog("SERVER value of fdt_s are: %d %d ", fdt_s[0], fdt_s[1]);

    // close the write file descriptor fdt_s[1], server only read from target
    if (close(fdt_s[1]) < 0)
    {
        perror("server: close fdt_s[1]");
        writeLog("ERROR ==> server: close fdt_s[1], %m ");
    }

    //// Take the fdo_s for comunication between obstacle-server, position 9, 10
    int fdo_s[2];
    for (i = 9; i < 11; i++)
    {
        fdo_s[i - 9] = atoi(argv[i]);
    }
    writeLog("SERVER value of fdo_s are: %d %d ", fdo_s[0], fdo_s[1]);

    // close the write file descriptor fdo_s[1], server only read from object
    if (close(fdo_s[1]) < 0)
    {
        perror("server: close fdo_s[1]");
        writeLog("ERROR ==> server: close fdo_s[1], %m ");
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    // parte legata ad ncurses per il server
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
    // map_window = create_new_window(Srow, Scol, 0, 0);
    // move_drone_icon(rowSH - (int)pose->Ypos, colSH + (int)pos->Xpos, map_window);
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, Srow, Scol);
    rowSH = Srow / 2; // definisco gli shift per traslare (0,0) al centro dello schermo
    colSH = Scol / 2;

    // stampare roba in ordine
    int cur_x, cur_y;
    //

    // varaibles for dynamics and forces
    // contain the position of obstacle and target
    double set_of_obstacle[MAX_OBST_ARR_SIZE][2];
    double set_of_target[MAX_TARG_ARR_SIZE][2];
    // contain the information about the position of drone, come from the pipe
    double dronePosition[2];
    // input force, cames from the piefrom input
    double inputForce[2];
    // compute in while cycle
    double obstForce[2] = {0};
    double targetForce[2] = {0};
    double totalForce[2] = {0};

    // read the set of target comes from target process
    if (read(fdt_s[0], set_of_target, sizeof(double) * MAX_TARG_ARR_SIZE * 2) == -1)
    {
        perror("server: read fdt_s[0]");
        writeLog("==> ERROR ==> server:read fdt_s[0], %m ");
    }
    // moltiply the target for the windows size
    for (i = 0; i < MAX_TARG_ARR_SIZE; i++)
    {
        set_of_target[i][0] = set_of_target[i][0] * colSH;
        set_of_target[i][1] = set_of_target[i][0] * rowSH;
    }

    // definizione variaili della select
    int retVal_sel;
    fd_set read_fd;
    struct timeval time_sel;

    // define the array wit all the read file descritor
    int fd_array[3] = {fdd_s[0], fdo_s[0], fdi_s[0]};
    // find the maximum fd
    int max_fd;
    max_fd = (fdd_s[0] > fdo_s[0]) ? fdd_s[0] : fdo_s[0];
    max_fd = (max_fd > fdi_s[0]) ? max_fd : fdi_s[0];

    while (1)
    {
        // define the set of fd
        FD_ZERO(&read_fd);
        FD_SET(fdd_s[0], &read_fd);
        FD_SET(fdo_s[0], &read_fd);
        FD_SET(fdi_s[0], &read_fd);

        // time interval for select
        time_sel.tv_sec = 1;
        time_sel.tv_usec = 0;

        // ---------------fare select --------------------------------------------------------------
        if ((retVal_sel = select(max_fd + 1, &read_fd, NULL, NULL, &time_sel)) < 0)
        {
            perror("server: error select: ");
            writeLog("==> ERROR ==> server:select %m ");
        }
        else if (retVal_sel == 0)
        {
            printf("no new data\n"); // pipe vuota
            getyx(stdscr, cur_y, cur_x);
            move(cur_y, 0);
            refresh();
            fflush(stdout);
        }
        else
        {
            // check wich file descriptor have data inside
            for (i = 0; i < (sizeof(fd_array) / sizeof(int)); i++)
            {
                // check if the fd is inside the ready file descriptor set
                if (FD_ISSET(fd_array[i], &read_fd))
                {

                    if (fd_array[i] == fdd_s[0]) // <<<< drone - server >>>>
                    {
                        // read the position from the drone
                        if (read(fdd_s[0], dronePosition, sizeof(double) * 2) == -1)
                        {
                            perror("server: read fdd_s[0]");
                            writeLog("==> ERROR ==> server:read fdd_s[0], %m ");
                        }
                        printf("drone position %f, %f\n", dronePosition[0], dronePosition[1]);
                        getyx(stdscr, cur_y, cur_x);
                        move(cur_y, 0);
                        refresh();
                        fflush(stdout);
                    }
                    else if (fd_array[i] == fdo_s[0]) // <<<< obstacle - server >>>>
                    {
                        // read the set of obstacle
                        if (read(fdo_s[0], set_of_obstacle, sizeof(double) * MAX_OBST_ARR_SIZE * 2) == -1)
                        {
                            perror("server: read fdo_s[0]");
                            writeLog("==> ERROR ==> server:read fdo_s[0], %m ");
                        }
                        printf("obstacle %f, %li\n", set_of_obstacle[0][0], sizeof(set_of_obstacle) / (sizeof(double)));
                        getyx(stdscr, cur_y, cur_x);
                        move(cur_y, 0);
                        refresh();
                        fflush(stdout);
                    }
                    else if (fd_array[i] == fdi_s[0]) // <<<< input - server >>>>
                    {

                        // read the input force
                        if (read(fdi_s[0], inputForce, sizeof(double) * 2) == -1)
                        {
                            perror("server: read fdi_s[0]");
                            writeLog("==> ERROR ==> server:read fdi_s[0], %m ");
                        }
                        printf("input force %f, %f\n", inputForce[0], inputForce[1]);
                        getyx(stdscr, cur_y, cur_x);
                        move(cur_y, 0);
                        refresh();
                        fflush(stdout);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    //------------------ furoi dalla select -----------------------------------
    /*
            // obtain obstacle position
            for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
            {
                set_of_obstacle[i][0] = set_of_obstacle[i][0] * colSH;
                set_of_obstacle[i][1] = set_of_obstacle[i][0] * rowSH;

                // cassare ostacoli sopra drone
            }

            // Compute obstacle Force
            for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
            {
                // computate the xForce
                obstForce[0] = obstForce[0] + ((k * inputForce[0]) / ((set_of_obstacle[i][0] - dronePosition[0]) ^ 2));
                // computate the yForce
                obstForce[1] = obstForce[1] + ((k * inputForce[1]) / ((set_of_obstacle[i][1] - dronePosition[1]) ^ 2));
            }

            // compute the targetForce
            for (i = 0; i < (sizeof(set_of_target) / sizeof(set_of_target[0])); i++)
            {
                // computate the xForce
                trgetForce[0] = targetForce[0] + ((k * inputForce[0]) / ((set_of_target[i][0] - dronePosition[0]) ^ 2));
                // computate the yForce
                targetForce[1] = obstForce[1] + ((k * inputForce[1]) / ((set_of_target[i][1] - dronePosition[1]) ^ 2));
            }

            // Compute total Force x:
            totalForce[0] = inputForce[0] - obstForce[0] + targetForce[0];
            // Compute total force y:
            totalForce[1] = inputForce[1] - obstForce[1] + targetForce[1];

            //// CREARE PIPE NEL MASTER FDS_d/////////////////////////
            if (write(fds_d[1], totalForce, sizeof(double) * 2) == -1)
            {
                perror("server: erite fds_d[1]");
                writeLog("==> ERROR ==> server: write fds_d[1], %m ");
            }
        /*
            // Print some value for control
            for (i = 0; i < MAX_TARG_ARR_SIZE; i++)
            {
                printf("set of target");
                printf("%f, %f \n", set_of_target[i][0], set_of_target[i][1]);
                fflush(stdout);
            }
            for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
            {
                printf("set of obstacle");
                printf("%f, %f \n", set_of_obstacle[i][0], set_of_obstacle[i][1]);
                fflush(stdout);
            }
            */
    /*
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
    */
    return 0;
}

//// ---- Functions sections -----------------------------------------------------------
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

/*
        // read the position from the drone
        if (read(fdd_s[0], dronePosition, sizeof(double) * 2) == -1)
        {
            perror("server: read fdd_s[0]");
            writeLog("==> ERROR ==> server:read fdd_s[0], %m ");
        }

        // read the set of obstacle
        if (read(fdo_s[0], set_of_obstacle, sizeof(double) * MAX_OBST_ARR_SIZE * 2) == -1)
        {
            perror("server: read fdo_s[0]");
            writeLog("==> ERROR ==> server:read fdo_s[0], %m ");
        }

        // read the input force
        if (read(fdi_s[0], inputForce, sizeof(double) * 2) == -1)
        {
            perror("server: read fdi_s[0]");
            writeLog("==> ERROR ==> server:read fdi_s[0], %m ");
        }
        printf("%f, %f", inputForce[0], inputForce[1]);
        fflush(stdout);
        writeLog("server: %f, %f", inputForce[0], inputForce[1]);
*/
