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
#include <math.h>
#include "arplib.h"

#define DRONE_ICON 'X'
#define MAX_OBST_ARR_SIZE 10
#define MAX_TARG_ARR_SIZE 10

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

bool spawn_autorization(int obst_x, int obst_y, int drone_x, int drone_y);

int signum(int x);

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

    // print window for the map
    WINDOW *spawn_window;
    // parte legata ad ncurses per il server
    int Srow = 50, Scol = 80;     // righe e colonne massime dello schermo
    int rowSH, colSH;
    // initialization row
    /*
    initscr();
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    box(stdscr, 0, 0);*/

    //getmaxyx(stdscr, Srow, Scol);
    // window for printing the drone, obst, targh
    int spawn_Col = Scol - 2;
    int spawn_Row = Srow - 2;

    rowSH = spawn_Row / 2; // definisco gli shift per traslare (0,0) al centro dello schermo
    colSH = spawn_Col / 2;

    //spawn_window = newwin(spawn_Row, spawn_Col, 2, 2);
    //wrefresh(spawn_window);

    // stampare roba in ordine
    int cur_x, cur_y;
    //

    // varaibles for dynamics and forces
    // contain the position of obstacle and target
    double set_of_obstacle[MAX_OBST_ARR_SIZE][2] = {{0.0}};
    double set_of_target[MAX_TARG_ARR_SIZE][2] = {{0.0}};

    int int_set_of_obstacle[MAX_OBST_ARR_SIZE][2] = {{0}};
    int int_set_of_target[MAX_TARG_ARR_SIZE][2] = {{0}};

    // contain the information about the position of drone, come from the pipe
    double dronePosition[2] = {0.0};

    int int_dronePosition[2] = {0};

    // input force, cames from the piefrom input
    double inputForce[2] = {0.0};

    // total force of obst, target and force to send to drone
    double obstForce[2] = {0.0};
    double targetForce[2] = {0.0};
    double totalForce[2] = {0.0};

    // read the set of target comes from target process
    if (read(fdt_s[0], set_of_target, sizeof(double) * MAX_TARG_ARR_SIZE * 2) == -1)
    {
        perror("server: read fdt_s[0]");
        writeLog("==> ERROR ==> server:read fdt_s[0], %m ");
    }
    // moltiply the target for the windows size
    for (i = 0; i < MAX_TARG_ARR_SIZE; i++)
    {
        int_set_of_target[i][0] = (int)(set_of_target[i][0] * spawn_Col);
        int_set_of_target[i][1] = (int)(set_of_target[i][0] * spawn_Row);
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

    // variabili di controllo per evitare di ristampare la mappa al ritmo del while
    int new_position, new_obstacles;

    // variabili per il calcolo delle forze
    int distance[2] = {0};
    double k = 0.2;

    while (1)
    {
        // azzero controlli di stampa
        new_position = 0;
        new_obstacles = 0;

        // define the set of fd
        FD_ZERO(&read_fd);
        FD_SET(fdd_s[0], &read_fd);
        FD_SET(fdo_s[0], &read_fd);
        FD_SET(fdi_s[0], &read_fd);

        // time interval for select
        time_sel.tv_sec = 1;
        time_sel.tv_usec = 0;

        // select for check the value
        if ((retVal_sel = select(max_fd + 1, &read_fd, NULL, NULL, &time_sel)) < 0)
        {
            perror("server: error select: ");
            writeLog("==> ERROR ==> server:select %m ");
        }
        else if (retVal_sel == 0)
        {
            // timeout expired
            // printf("timeout expired\n");
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

                        // moltiply the position for the windows size
                        int_dronePosition[0] = (int)(dronePosition[0] * spawn_Col);
                        int_dronePosition[1] = (int)(dronePosition[1] * spawn_Row);

                        new_position = 1;
                    }
                    else if (fd_array[i] == fdo_s[0]) // <<<< obstacle - server >>>>
                    {
                        // read the set of obstacle
                        if (read(fdo_s[0], set_of_obstacle, sizeof(double) * MAX_OBST_ARR_SIZE * 2) == -1)
                        {
                            perror("server: read fdo_s[0]");
                            writeLog("==> ERROR ==> server:read fdo_s[0], %m ");
                        }
                        
                        // moltiply the obstacle for the windows size --> get int position of obstacle
                        for(i = 0; i < MAX_OBST_ARR_SIZE; i++){
                            int_set_of_obstacle[i][0] = (int)(set_of_obstacle[i][0] * spawn_Col);
                            int_set_of_obstacle[i][1] = (int)(set_of_obstacle[i][1] * spawn_Row);
                        }

                        new_obstacles = 1; // set the flag for print the map
                    }
                    else if (fd_array[i] == fdi_s[0]) // <<<< input - server >>>>
                    {

                        // read the input force
                        if (read(fdi_s[0], inputForce, sizeof(double) * 2) == -1)
                        {
                            perror("server: read fdi_s[0]");
                            writeLog("==> ERROR ==> server:read fdi_s[0], %m ");
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    

        //------------------ furoi dalla select -----------------------------------
        // compute the obstacle and target force
        // useful variables
        distance[0] = 0;
        distance[1] = 0;
        k = 0.2;

        // each cycle i have to reset the obstacle force
        obstForce[0] = 0.0;
        obstForce[1] = 0.0;

        // each cycle i have to reset the target force
        targetForce[0] = 0.0;
        targetForce[1] = 0.0;

        //printf("calcolo forze ostacoli\n");

        for(i = 0; i<MAX_OBST_ARR_SIZE; i++){
            // calculating the distance between drone and obstacle
            distance[0] = int_dronePosition[0] - int_set_of_obstacle[i][0];
            distance[1] = int_dronePosition[1] - int_set_of_obstacle[i][1];

            // check if drone is too close to obstacle
            if(abs(distance[0]) < 2 && abs(distance[1]) < 2){
                // if true, the obstacle is too close to drone, so the repulsive force is 0
                k = 0.0;
            }
            // null distance exeption
            if(distance[0] == 0){
                distance[0] = 1;
            }else if(distance[1] == 0){
                distance[1] = 1;
            }else{
                // do nothing
            }


            // compute the x and y force
            obstForce[0] = obstForce[0] + ((k * inputForce[0]) / (distance[0]));
            obstForce[1] = obstForce[1] + ((k * inputForce[1]) / (distance[1]));

            //printf("obst force: %f, %f ---- k: %.1f , %d, %d\n", ((k * inputForce[0]) / (distance[0])), ((k * inputForce[1]) / (distance[1])), k, distance[0], distance[1]);
        }

        //printf("calcolo forze target\n");

        for(i = 0; i<MAX_TARG_ARR_SIZE; i++){
            // calculating the distance between drone and target
            distance[0] = int_dronePosition[0] - int_set_of_target[i][0];
            distance[1] = int_dronePosition[1] - int_set_of_target[i][1];

            // compute the x and y force
            targetForce[0] = targetForce[0] + ((inputForce[0]) / (distance[0]));
            targetForce[1] = targetForce[1] + ((inputForce[1]) / (distance[1]));

            //printf("obst force: %f, %f ---- %d, %d\n", ((inputForce[0]) / (distance[0])), ((inputForce[1]) / (distance[1])), distance[0], distance[1]);
        }
        
        totalForce[0] = inputForce[0] + obstForce[0] + targetForce[0];
        totalForce[1] = inputForce[1] + obstForce[1] + targetForce[1];


        // creare la stringa da inviare al drone nella forma |1|totalForce[0]|1|totalForce[1]|
        // the X char terminate the string -> the drone can read the string

        // write force to drone
        if(!(isnan(totalForce[0]) || isnan(totalForce[1]))){
            if (write(fds_d[1], totalForce, sizeof(double) * 2) == -1)
            {
                perror("server: erite fds_d[1]");
                writeLog("==> ERROR ==> server: write fds_d[1], %m ");
            }
        }
        
        
        if (new_obstacles == 1){
            for (i = 0; i < 3; i++)
            {
                printf("set of obst");
                printf("%d, %d \n", int_set_of_obstacle[i][0], int_set_of_obstacle[i][1]);
                fflush(stdout);
            }
            for (i = 0; i < 3; i++)
            {
                printf("set of target");
                printf("%d, %d \n", int_set_of_target[i][0], int_set_of_target[i][1]);
                fflush(stdout);
            }
        }
        printf("drone position: %d, %d\n", int_dronePosition[0], int_dronePosition[1]);
        //printf("total force: %f, %f\n", totalForce[0], totalForce[1]);
        //printf("input force: %f, %f\n", inputForce[0], inputForce[1]);
        //printf("obst force: %f, %f\n", obstForce[0], obstForce[1]);
        //printf("target force: %f, %f\n", targetForce[0], targetForce[1]);
        
        /*
        if(new_obstacles == 1 || new_position == 1){
            
            // clear the map window
            wclear(spawn_window);

            // print the drone icon
            mvwaddch(spawn_window, rowSH - (int)dronePosition[1], colSH + (int)dronePosition[0], DRONE_ICON);


            // print the obstacle
            for (i = 0; i < MAX_OBST_ARR_SIZE; i++) {
                if ((int)set_of_obstacle[i][0] != -1 && (int)set_of_obstacle[i][1] != -1) {
                    mvwaddch(spawn_window, (int)set_of_obstacle[i][1], (int)set_of_obstacle[i][0], 'O');
                }   
            }

            
            // print the target
            for(i=0; i < MAX_TARG_ARR_SIZE; i++){
                if((int)set_of_target[i][0] != -1 && (int)set_of_target[i][1] != -1){
                    mvwaddch(spawn_window, (int)set_of_target[i][1], (int)set_of_target[i][0], 'T');
                }
            }

            refresh();
            
            
            
        } */
        
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

        // gestione del resize della finestra
        //getmaxyx(stdscr, Srow, Scol);

        spawn_Col = Scol - 2;
        spawn_Row = Srow - 2;

        rowSH = spawn_Row / 2; // definisco gli shift per traslare (0,0) al centro dello schermo
        colSH = spawn_Col / 2;
        
    } // while(1) end --> if all the target are reached, we exit from this cycle


    return 0;
}

//// ---- Functions sections -----------------------------------------------------------
WINDOW *create_new_window(int row, int col, int ystart, int xstart){
    WINDOW *local_window = newwin(row, col, ystart, xstart);
    box(local_window, 0, 0);

    wrefresh(local_window);
    return local_window;
}

bool spawn_autorization(int obst_x, int obst_y, int drone_x, int drone_y)
{
    // return true whe I can spawn the obstacle
    double trsh = 2.0;
    // compute distance between obstacle and drone
    double mod = sqrt((obst_x - drone_x) ^ 2 + (obst_y - drone_y) ^ 2);
    // if distance is less than a threshold, I return false, so the obstacle is too near to drone
    if (mod < trsh)
    {
        return false;
    }
    else
    {
        return true;
    }
    // the obstacle is enough far, so spawn it
    return true;
}

int signum(int x){
    if(x > 0){
        return 1;
    }else if(x < 0){
        return -1;
    }else{
        return 1;
    }
}



/*
// obtain obstacle position

        
        for (i = 0; i < MAX_OBST_ARR_SIZE; i++)
        {
            // if true spawn the obstacle
            if (spawn_autorization(set_of_obstacle[i][0], set_of_obstacle[1][1], dronePosition[0], dronePosition[1]))
            {
                set_of_obstacle[i][0] = (set_of_obstacle[i][0] * Scol);
                set_of_obstacle[i][1] = (set_of_obstacle[i][0] * Srow);
            }
            else
            // set the vaue of obstacle out the rappresentation of Ncurses
            {
                set_of_obstacle[i][0] = -1; // negative position are not printed by ncurses
                set_of_obstacle[i][1] = -1;
            }
        }

        
        for(i = 0; i < MAX_OBST_ARR_SIZE; i++){
            printf("obstacle %f, %f\n", set_of_obstacle[i][0], set_of_obstacle[i][1]);
            fflush(stdout); 
        }
        sleep(1); ////////////////// DELETE THIS LINE
        

        double k;
        // Compute obstacle Force
        for (i = 0; i < (sizeof(set_of_obstacle) / sizeof(set_of_obstacle[0])); i++)
        {
            if((int)set_of_obstacle[i][0] == -1 && (int)set_of_obstacle[i][1] == -1){
                // if the obst position is position (-1, -1) is out of the map -> the repulsive force is 0
                k = 0.0;
            }else
            {
                k = 0.2;
            }
            // computate the xForce
            obstForce[0] = obstForce[0] + ((k * inputForce[0]) / pow((set_of_obstacle[i][0] - dronePosition[0]), 2.0));
            // computate the yForce
            obstForce[1] = obstForce[1] + ((k * inputForce[1]) / pow((set_of_obstacle[i][1] - dronePosition[1]), 2.0));
        }

        // compute target Force
        for (i = 0; i < (sizeof(set_of_target) / sizeof(set_of_target[0])); i++)
        {
            if((int)set_of_obstacle[i][0] == -1 && (int)set_of_obstacle[i][1] == -1){
                // if the targ position is position (-1, -1) is out of the map -> the attractive force is 0
                k = 0.0;
            }else
            {
                k = 0.2;
            }

            // computate the xForce
            targetForce[0] = targetForce[0] + ((k * inputForce[0]) / pow((set_of_target[i][0] - dronePosition[0]), 2.0));
            // computate the yForce
            targetForce[1] = targetForce[1] + ((k * inputForce[1]) / pow((set_of_target[i][1] - dronePosition[1]), 2.0));
        }

        // Compute total Force x:
        totalForce[0] = inputForce[0] + obstForce[0] + targetForce[0];
        // Compute total force y:
        totalForce[1] = inputForce[1] + obstForce[1] + targetForce[1];

*/

    
