
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
#include <ctype.h>
#ifndef INITIALIZATION_H
#define INITIALIZATION_H

typedef enum { MUTEX, SEMAPHORE } LockType;

typedef struct {
    char name[100];
    LockType type;
    int capacity;

    pthread_mutex_t mutex;
    sem_t semaphore;

    char holding_trains[10][100];
    int num_holding;
} Intersection;

char** getTrains();
Intersection* getIntersections();

#endif
