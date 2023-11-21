#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h> 



double *generate_input_vector(double **vectorP);
// Funzioni per le integragrazioni approssimate
double *integrate_acceleration(double *force_vect, double *initial_velocity, double **vectorP);
double *integrate_velocity(double *velocity_vect, double *initial_position, double **vectorP);

// funzioni dei calcoli matriciali
double *matrix_moltiplication(double *matrix_1, double *matrix_2, double **vectorP);
double *matrix_sum(double *matrix_1, double *matrix_2, double **vectorP);

int main(){
    char ch;

    double *xy_force;
    xy_force = generate_input_vector(&xy_force);

    double *xy_velocity;
    double initial_velocity[2] = {0.0, 0.0};

    double *xy_position;
    double initial_position[2] = {0.0, 0.0};

    printf("valori inziali: fx:%f, fy:%f\n                vx:%f, vy:%f\n",xy_force[0], xy_force[1], initial_velocity[0], initial_velocity[1]);

    xy_velocity = integrate_acceleration(xy_force, initial_velocity, &xy_velocity);
    printf("velocità iniziale: %f, %f || velocità prima integraz: %f, %f\n", initial_velocity[0], initial_velocity[1], xy_velocity[0], xy_velocity[1]);
    while((ch = getc(stdin))!='q'){
        xy_velocity = integrate_acceleration(xy_force, xy_velocity, &xy_velocity);
        printf("velocità integrata: %f, %f\n", xy_velocity[0], xy_velocity[1]);
    }

    // printf("total force X: %f",xy_force[0]);
    // printf("total force X: %f",xy_force[1]);



    // printf("%f, %f, %f\n", 5.0, 2.0, 5.0/2.0);
    // printf("%f, %f, %f\n", 5.0, 2.0, 5.0/2);
}

double *generate_input_vector(double **vectorP){
    srand(time(NULL));
    double vector[8];
    *vectorP = malloc(sizeof(double)*2);

    for (int i = 0; i < 8; i++)
    {
        vector[i] = (double)(rand()%2);
        //printf("force value: %d, %f\n", i, vector[i]);
    }

    (*vectorP)[0] = vector[2] - vector[6] + vector[1]/2 + vector[3]/2 - vector[5]/2 - vector[7]/2;
    (*vectorP)[1] = vector[0] - vector[4] + vector[1]/2 - vector[3]/2 - vector[5]/2 - vector[7]/2;

    return *vectorP;
}

double *integrate_acceleration(double *force_vect, double *initial_velocity, double **vectorP){
    if (!*vectorP){
        *vectorP = malloc(sizeof(double)*2); // alloco la memoria solo se il puntatore deve essere inizializzato
    }else{}

    double deltaToM[2] = {0.1/10, 0.1/10};
    double viscK[2] = {-0.1, -0.1};

    double *Fluid_force = matrix_moltiplication(viscK, initial_velocity, &Fluid_force);
    double *Total_force = matrix_sum(force_vect, Fluid_force, &Total_force);

    double *delta_velocity = matrix_moltiplication(deltaToM, Total_force, &delta_velocity);
    *vectorP = matrix_sum(initial_velocity, delta_velocity, vectorP);

    return *vectorP;
}

double *integrate_velocity(double *velocity_vect, double *initial_position, double **vectorP){
    *vectorP = malloc(sizeof(double)*2); // riservo memoria per i 2 nuovi valori di posizione 

    double deltaT[2] = {0.1, 0.1};

    
}

double *matrix_moltiplication(double *matrix_1, double *matrix_2, double **vectorP){
    if (!*vectorP){
        *vectorP = malloc(sizeof(double)*2); // alloco la memoria solo se il puntatore deve essere inizializzato
    }else{}

    (*vectorP)[0] = matrix_1[0]*matrix_2[0] + matrix_1[0]*matrix_2[1];
    (*vectorP)[1] = matrix_1[1]*matrix_2[0] + matrix_1[1]*matrix_2[1];

    return *vectorP;
}

double *matrix_sum(double *matrix_1, double *matrix_2, double **vectorP){
    if (!*vectorP){
        *vectorP = malloc(sizeof(double)*2); // alloco la memoria solo se il puntatore deve essere inizializzato
    }else{}

    (*vectorP)[0] = matrix_1[0]+matrix_2[0];
    (*vectorP)[1] = matrix_1[1]+matrix_2[1];

    return *vectorP;
}
