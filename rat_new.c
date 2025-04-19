//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 04/13/2025
//Program Description: Implementation of a resource allocation table

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdbool.h>

#define MAX_NAME_LEN 32
#define MAX_HOLDING 10
#define MAX_LINE_LEN 100

// Intersection LockType enum (MUTEX or SEMAPHORE)
typedef enum { MUTEX, SEMAPHORE } LockType;

typedef struct {
    char name[MAX_NAME_LEN];
    LockType type;
    int capacity;

    pthread_mutex_t mutex;
    sem_t semaphore;

    char holding_trains[MAX_HOLDING][MAX_NAME_LEN];
    int num_holding;
} Intersection;

bool rat(int numInter, int numTrain, Intersection* intersections, int* req, int* alloc){  //Resource allocation table method

    int avail[numInter];
    printf("Alloc Matrix: ");
    for(int i = 0; i < numInter; i++){
        avail[i] = intersections[i].capacity;  //Max that can be allocated.
        for(int j = 0; j < numTrain; j++){
            printf("%d", *(alloc + i * numInter + j));
            if(*(alloc + j * numInter + i) > 0){
                avail[i] -= *(alloc + j * numInter + i); //Decrements available from max to not currently allocated.
            }
        }
        printf("\n");
        //printf("%d ",avail[i]);
    }
    printf("\n");
    bool cycle = 1; //Start with an assumed cycle
    //printf("Req Matrix: ");
    for(int i = 0; i < numTrain; i++){
        for(int j = 0; j < numInter; j++){
            //printf("%d", req[i][j]);
            if((avail[j] - *(req + i * numInter + j)) < 0){
                break; //Stops checking request if it is larger than available.
            }
            if(j == (numInter - 1)){
                cycle = 0; //Sets cycle to false if it fully iterates through a line, i.e. there is a request that can be fulfilled.
            }
        }
        //printf("\n");
    }
    //printf("\n");
    if(cycle == 1){
        printf("DEADLOCK!\n");
    }
    else{
        printf("NO DEADLOCK\n");
    }
    return cycle;
}