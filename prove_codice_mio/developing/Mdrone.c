#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <string.h> 
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>


double *generate_input_vect(double **vect_pointer, char ch);
double *put_vector_to_zero(double **vect_pointer);

double *generate_x_force(double *vect_pointer, double *force);
double *generate_y_force(double *vect_pointer, double *force);

double *velocity(double Force, double initial_velocity, double *new_vel);
double *position(double Velocity, double initial_position, double *new_pose);

int main(int argc, char *argv[]){

    /*
        cosa deve fare il programma 
        1) ricevere informazioni dalla pipe
        2) convertire l'input dal char al vettore
        3) convertire il vettore in due variabili Fx e Fy

        4) eseguire l'integrazione della forza -> ottengo posizione

        5) scrivere posizione e input nella shared memory (stampare a schermo)
    */

   int pipe_fd[2];
   for(int i=1; i<argc; i++){
        pipe_fd[i-1] = atoi(argv[i]); // converto gli argv da stringhe in integer
   }
   close(pipe_fd[1]);


    double *input_vect = malloc(sizeof(double)*8); // riservo la memoria per il vettore di input

    double XForce = 0.0, YForce = 0.0;
    double *XForce_p = &XForce, *YForce_p =  &YForce;

    double Xvel = 0.0, Yvel =0.0, Xpos =0.0, Ypos = 0.0;
    double *Xvel_p = &Xvel, *Xpos_p = &Xpos, *Yvel_p = &Yvel, *Ypos_p = &Ypos;

    int retVal_read;
    char ch;

    int retVal_sel;
    fd_set read_fd;
    struct timeval time_sel;

    while(TRUE){
        FD_ZERO(&read_fd);
        FD_SET(pipe_fd[0], &read_fd);

        time_sel.tv_sec = 0;
        time_sel.tv_usec = 500000;

        if((retVal_sel = select(pipe_fd[0]+1, &read_fd, NULL, NULL, &time_sel))<0){
            perror("errore select: ");
        }else if (retVal_sel == 0){
            printf("no new data\n");
            fflush(stdout);
        }else{
            if((retVal_read = read(pipe_fd[0], &ch, 1)) < 0){
                perror("errore read");
            }else{
                printf("controllo lettura: %d, ( %c )\n", retVal_read, ch);
                fflush(stdout);
            }
            input_vect = generate_input_vect(&input_vect, ch); // gli passo il valore di input ricevuto e scrivo nel vettore
            for (int i = 0; i < 8; i++){
                printf("%.2f ", input_vect[i]);
            }
            printf("\n");
            fflush(stdout);
        }

        XForce_p = generate_x_force(input_vect, XForce_p);
        YForce_p = generate_y_force(input_vect, YForce_p);
         
        // genera velocità
        Xvel_p = velocity(*XForce_p, *Xvel_p, Xvel_p);
        Yvel_p = velocity(*YForce_p, *Yvel_p, Yvel_p);
        //printf("controllo valori: yf:%.2f, yVel:%.2f\n", *YForce_p, *Yvel_p);
        fflush(stdout);
        

        // genera posizione 
        Xpos_p = position(*Xvel_p, *Xpos_p, Xpos_p);
        Ypos_p = position(*Yvel_p, *Ypos_p, Ypos_p);

        // invia posizione a server (mappa)
        printf("forVal(x,y):%.2f, %.2f---velVal(x,y):%.2f, %.2f---posVal(x,y):%.2f, %.2f\n", *XForce_p, *YForce_p, *Xvel_p ,*Yvel_p, *Xpos_p, *Ypos_p);
        fflush(stdout);
    }

    return 0;
}

double *generate_input_vect(double **vect_pointer, char ch){
    switch (ch)
    {
        case 'w':  (*vect_pointer)[7]=(*vect_pointer)[7]+1;break;
        case 'e':  (*vect_pointer)[0]=(*vect_pointer)[0]+1;break;
        case 'r':  (*vect_pointer)[1]=(*vect_pointer)[1]+1;break;
        case 'f':  (*vect_pointer)[2]=(*vect_pointer)[2]+1;break;
        case 'v':  (*vect_pointer)[3]=(*vect_pointer)[3]+1;break;
        case 'c':  (*vect_pointer)[4]=(*vect_pointer)[4]+1;break;
        case 'x':  (*vect_pointer)[5]=(*vect_pointer)[5]+1;break;
        case 's':  (*vect_pointer)[6]=(*vect_pointer)[6]+1;break;
        case 'd':  *vect_pointer = put_vector_to_zero(&(*vect_pointer));break;
        case 'q': exit(1);
        default:   break;
    }

    return *vect_pointer;
}

double *put_vector_to_zero(double **vect_pointer){
    for(int i = 0; i<8; i++){
        (*vect_pointer)[i] = 0.0; // inizializzazione
    }
}

double *generate_x_force(double *vect_pointer, double *force){
    *force = vect_pointer[2] -vect_pointer[6] + vect_pointer[1]/2.0 + vect_pointer[3]/2.0 - vect_pointer[7]/2.0 - vect_pointer[5]/2.0;
    return force;
}

double *generate_y_force(double *vect_pointer, double *force){
    *force = vect_pointer[0] -vect_pointer[4] + vect_pointer[1]/2.0 - vect_pointer[3]/2.0 + vect_pointer[7]/2.0 - vect_pointer[5]/2.0;
    return force;
}

double *velocity(double Force, double initial_velocity, double *new_vel){
    double dtime_m = 1, frict_k = -0.1;

    //printf("in -- %.2f + %f * (%.2f + %f * %.2f) --- %.2f\n", initial_velocity, dtime_m, Force, frict_k, initial_velocity, *new_vel);
    *new_vel = initial_velocity + dtime_m * (Force + frict_k * initial_velocity);
    //printf("out-- %.2f --\n", *new_vel);
    return new_vel;
}


double *position(double Velocity, double initial_position, double *new_pose){
    double dtime = 0.1;

    *new_pose = initial_position + dtime * Velocity;
    return new_pose;
}



        