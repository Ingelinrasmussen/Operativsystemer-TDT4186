#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h> // For errno and strerror function
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"

/*Veldig usikker på om vi trenger mutex. Mulig denne bare er overflødig når vi kaller P og V
(tror egentlig dette er tilfelet)*/

typedef struct BNDBUF{
    int size; /* buffer size          */
    int in; /* next empty location      */
    int out; /* next filled location     */
    //int count;  
    pthread_mutex_t mutex_lock;  //A binary semaphore used to enforce mutual exclusive updates to the buffer
    SEM *full; //count the number of data items in the buffer
    SEM *empty; //count the empty slots in the buffer.
    int *BNDBUF; //
    //int buffer[]; //buffer itmems

} BNDBUF;


BNDBUF *bb_init(unsigned int size){

    struct BNDBUF *bndbuf = malloc(sizeof(struct BNDBUF)); //Making space in memory for buffer
    bndbuf->size = size;   
    bndbuf->in = 0;
    bndbuf->out = 0;
    bndbuf->full = sem_init(0); //Buffer is not filled yet, has 0 full spaces
    bndbuf->empty = sem_init(size); //Buffer has size empty spaces
    //bndbuf->buffer[size];

    int mutex_lock_error = pthread_mutex_init(&bndbuf->mutex_lock, NULL);
    if (mutex_lock_error){
        printf("Failed initializing mutex lock.");
	};

    return bndbuf;
}

//Deletes bounded buffer
void bb_del(BNDBUF *bb){
    
    //free(bb->count);
    //free(bb->full);
    //free(bb->empty);

    //free(bb->buffer);
    bb->size = 0;
    bb->in = 0;
    bb->out = 0;
    
    free(bb);
    free(bb->BNDBUF);

    sem_del(bb->empty);
	sem_del(bb->full);
    return;
}

int bb_get(BNDBUF *bb){ //Consumer

    P(bb->full);
    pthread_mutex_lock(&bb -> mutex_lock);

    int nextin = (bb->in);
    int nextout = (bb->out);
    int element = (bb->BNDBUF[nextout]); //remove item from buffer
    bb->out = (nextout + 1) % (bb->size); //consume the item 
    
    pthread_mutex_unlock(&bb -> mutex_lock);
    V(bb->empty);
    
   
    return nextin;

}

void bb_add(BNDBUF *bb, int fd){ //Producer

    P(bb->empty); // Decrement the empty semaphore
    pthread_mutex_lock(&bb -> mutex_lock);

    int nextin = bb->in;
    int nextout = bb->out;
    bb->BNDBUF[nextin] = fd; //  produce an item 
    bb->in = (nextin + 1) % (bb->size);// place item in buffer

    pthread_mutex_unlock(&bb -> mutex_lock);
    V(bb->full);    // increment the full semaphore and signal
    
}

      

