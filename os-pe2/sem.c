#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h> // For errno and strerror function
#include <pthread.h>

#include "sem.h"

//TO DO Fill in functions. done
//TO FO Feilmeldinger

typedef struct SEM{

    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    unsigned int count, signals;

} SEM;

 //Creates a new semaphore
SEM *sem_init(int initVal) {

    SEM *sem = malloc(sizeof(struct SEM)); //Assigning memory to struct variable sem

    sem->count = initVal; //Assigning initVal to count variable of sem using arrow operator
    sem->signals = 0; //Initialise signals to zero
    pthread_mutex_init(sem->mutex, NULL); //initialise or a mutex
    pthread_cond_init(sem->cond, NULL); //initialise condition variables
    return sem;
}

//Delete semaphore
int sem_del(SEM *sem){
    int errors = 0;
    if (pthread_mutex_destroy((sem->mutex))!= 0){
        errors -= 1;
        printf("Not able to destroy mutex");
    }

    if (pthread_cond_destroy((sem->cond))!= 0){
        errors -= 1;
        printf("Not able to destroy condition");
    }    
    free(&sem); //Frees the semaphore, regardless of number of errors
    return errors; //If no errors occur, this will be 0. If an error has occured, returned value will be negative
}

//Wait-operation
void P(SEM *sem){ 

    pthread_mutex_lock(sem->mutex);

    while(sem->count == 0) {
        pthread_cond_wait(sem->cond,sem->mutex);
    }

    --(sem->count);
    pthread_mutex_unlock(sem->mutex);
    return;
}

//Signal operation
void V(SEM *sem){ 
    pthread_mutex_lock(sem->mutex);

    ++(sem->count);

    pthread_mutex_unlock(sem->mutex);
    pthread_cond_signal(sem->cond);
    return;
}

