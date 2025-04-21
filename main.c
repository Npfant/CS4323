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
#include "forking.h"
#include "initialization.h"
#include "messages.h"
#include "intersections.h"
#include "rat.h"
#include "logger.h"
#include "logging.h"

//Global variables
char** trains = NULL;    
Intersection* intersections = NULL;
int NUM_TRAINS = 0;
int NUM_INTERSECTIONS = 0;

void handle_request(int req_id, int res_id) {
    struct request_msg request;
    struct response_msg response;
    
    while (1) {
        receive_request(req_id, &request);
        printf("Server received request from %s for %s\n", request.train_name, request.intersection);
        
        int idx = find_intersection_index(request.intersection, NUM_INTERSECTIONS, intersections);
        if (idx == -1) {
            printf("ERROR: Unknown intersection %s\n", request.intersection);
            continue;
        }
        
        Intersection* inter = &intersections[idx];
        
        // Check if the intersection is available
        if (inter->num_holding < inter->capacity) {
            // Intersection is available, grant access
            send_response(res_id, "GRANT");
            add_train_to_holding(inter, request.train_name);
        } else {
            // Intersection is not available, send WAIT response
            send_response(res_id, "WAIT");
        }
    }
}

// Train behavior
void train_behavior(char* train_info, int req_id, int res_id) {
    char* train_name = strtok(train_info, ":");       //get dat train name
    char* interName = strtok(NULL, ":");              //get da intersections list
    interName = strtok(interName, ",");               //now get only the first one
    while (interName != NULL) {                       //if there are still more intersections, GET ANOTHER ONE
        acquire_intersection(train_name, interName, req_id, res_id, NUM_TRAINS, NUM_INTERSECTIONS, intersections, trains);    //train enter :D
        release_intersection(train_name, interName, req_id, res_id, NUM_TRAINS, NUM_INTERSECTIONS, intersections, trains);    //train leave :(
        interName = strtok(NULL, ",\t\r\n\v\f\b");              //GET THE NEXT ONE
    }
    exit(0);
}

int main() {
    // Create two message queues: one for requests, one for responses
    int req_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    int res_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    
    //Allocate memory for trains and intersections
    trains = (char**)getTrains();
    intersections = (Intersection*)getIntersections();
    NUM_TRAINS = howManyTrains();
    NUM_INTERSECTIONS = howManyInters();

    printf("Found %d trains and %d intersections\n", NUM_TRAINS, NUM_INTERSECTIONS);
    
    
    //Create request and allocation matricies in shared memory
    key_t key1 = ftok(".", 'b');
    key_t key2 = ftok(".", 'c');
    createBuf1(NUM_TRAINS, NUM_INTERSECTIONS, key1);
    createBuf2(NUM_TRAINS, NUM_INTERSECTIONS, key2);

    for(int i = 0; i < NUM_INTERSECTIONS; i++){
      for(int j = 0; j < NUM_TRAINS; j++){
        req[i][j] = 0;
        alloc[i][j] = 0;
      }
    }

    for (int i = 0; i < NUM_TRAINS; i++){
        
        // Remove trailing newline/whitespace
        int len = strlen(trains[i]);
        while (len > 0 && (trains[i][len-1] == '\n' || trains[i][len-1] == '\r')) {
            trains[i][--len] = '\0';
        }
    }
    
    if (req_id == -1 || res_id == -1) {
        perror("msgget failed");
        exit(1);
    }
    mutexes(NUM_INTERSECTIONS, intersections);
    //Fork trains
    int createForks = forking(trains, NUM_TRAINS, req_id, res_id);

    // Server processing requests
    for (int i = 0; i < NUM_TRAINS; i++) {
        struct request_msg request;
        receive_request(req_id, &request);
        printf("Server received request from %s for %s\n", request.train_name, request.intersection);

        // Handle request (grant or deny)
        send_response(res_id, getpid(), "GRANT"); ;  // Simple grant for now
    }

    // Wait for all trains to finish
    for (int i = 0; i < NUM_TRAINS; i++) {
        wait(NULL);
    }

    printf("Simulation complete. All trains finished.\n");

    Event e;
    strcpy(e.type, "COM");
    log_event(e);

    // Cleanup message queues
    msgctl(req_id, IPC_RMID, NULL);
    msgctl(res_id, IPC_RMID, NULL);
    
    //Free the allocated memory
    for (int i = 0; i < NUM_TRAINS; i++) {
        free(trains[i]);
    }
    free(trains);
    free(intersections);

    return 0;
}
