/*
 * Group Project - CS 4323 Operating Systems
 
    

 * Description:
 * This module handles how trains (child processes) release intersections.
 * It is triggered when the server receives a RELEASE message from a train.
 * Based on the intersection type (mutex or semaphore), it unlocks the resource
 * and updates the resource allocation table stored in shared memory.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

#define MAX_TRAINS 10
#define MAX_NAME_LEN 32

// Define the intersection type: either a mutex (1 train) or semaphore (>1 trains)
typedef enum {
    MUTEX,
    SEMAPHORE
} LockType;

// Define the message format for communication
struct message {
    long msg_type;  // Must be long for System V message queues
    char train_name[MAX_NAME_LEN];
    char intersection_name[MAX_NAME_LEN];
};

// Structure representing an intersection in shared memory
typedef struct {
    char name[MAX_NAME_LEN];               // Intersection name
    LockType type;                         // MUTEX or SEMAPHORE
    int capacity;                          // Capacity of the intersection
    pthread_mutex_t mutex;                 // Mutex (used if capacity == 1)
    sem_t semaphore;                       // Semaphore (used if capacity > 1)
    char holding_trains[MAX_TRAINS][MAX_NAME_LEN];  // List of trains holding this intersection
    int num_holding;                       // Number of trains currently holding
} Intersection;

// Helper: Find intersection by name
int find_intersection_index(const char* name, Intersection intersections[], int total) {
    for (int i = 0; i < total; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;  // Not found
}

// Helper: Remove train from the holding_trains list
void remove_train_from_holding(Intersection* inter, const char* train_name) {
    for (int i = 0; i < inter->num_holding; i++) {
        if (strcmp(inter->holding_trains[i], train_name) == 0) {
            // Shift down remaining trains
            for (int j = i; j < inter->num_holding - 1; j++) {
                strcpy(inter->holding_trains[j], inter->holding_trains[j + 1]);
            }
            inter->num_holding--;
            break;
        }
    }
}

// Main handler function to process RELEASE message
void handle_release(struct message msg, Intersection intersections[], int total_intersections) {
    int idx = find_intersection_index(msg.intersection_name, intersections, total_intersections);
    if (idx == -1) {
        printf("SERVER: ERROR - Intersection '%s' not found.\n", msg.intersection_name);
        return;
    }

    Intersection* inter = &intersections[idx];

    // Check if the train actually holds this intersection
    int holds_it = 0;
    for (int i = 0; i < inter->num_holding; i++) {
        if (strcmp(inter->holding_trains[i], msg.train_name) == 0) {
            holds_it = 1;
            break;
        }
    }

    if (!holds_it) {
        printf("SERVER: WARNING - Train '%s' tried to release intersection '%s' it does not hold.\n",
               msg.train_name, msg.intersection_name);
        return;
    }

    // Release the lock
    if (inter->type == MUTEX) {
        pthread_mutex_unlock(&inter->mutex);
        printf("SERVER: Mutex released on %s by %s\n", inter->name, msg.train_name);
    } else if (inter->type == SEMAPHORE) {
        sem_post(&inter->semaphore);
        printf("SERVER: Semaphore released on %s by %s\n", inter->name, msg.train_name);
    }

    // Update holding list
    remove_train_from_holding(inter, msg.train_name);
    printf("SERVER: Updated resource table. '%s' no longer holds '%s'.\n",
           msg.train_name, msg.intersection_name);
}
