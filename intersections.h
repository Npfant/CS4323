// Ashton Cecil

#include "messages.h"
#include "holding.h"

int find_intersection_index(const char* name, int NUM_INTERSECTIONS, Intersection* intersections) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_train_index(const char* name, int NUM_TRAINS, char** trains) {
    for (int i = 0; i < NUM_TRAINS; i++) {
        if (strcmp(trains[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void preemption(int trainx, const char* train_name, int req_id, int res_id, int numInter, int numTrains, Intersection* intersections, char** trains, int* req, int* alloc);

// This is the core logic (most key part)
void acquire_intersection(const char* train_name, const char* inter_name, int req_id, int res_id, int* req, int* alloc, int numTrains, int numInter, Intersection* intersections, char** trains) {
    int idx = find_intersection_index(inter_name, numInter, intersections);
    int trainx = find_train_index(train_name, numTrains, trains);
    if (idx == -1) {
        printf("ERROR: Unknown intersection %s.\n", inter_name);
        return;
    }
    if (trainx == -1) {
        printf("ERROR: Unknown train %s.\n", train_name);
        return;
    }

    Intersection* inter = &intersections[idx];
    *(req + trainx * numInter + idx) = 1; //Add resource to request matrix.
    printf("Train ID: %d, Intersection ID: %d, Request: %d  \n", trainx, idx, *(req + trainx * numInter + idx));
    printf("%s is waiting at %s.\n", train_name, inter->name);
    send_request(req_id, train_name, inter_name);

    struct response_msg response;
    receive_response(res_id, &response);
    bool check_rat = 0; //Check for deadlock.
    bool cycle = 0;
    while (strcmp(response.response, "WAIT") == 0) {
        printf("%s is waiting for permission at %s.\n", train_name, inter->name);
        if(check_rat == 0){
            cycle = rat(numTrains, numInter, intersections, (int*) req, (int*) alloc); //Call resource allocation table method to check whether or not a deadlock has occurred.
            check_rat = 1;
        }
        if(cycle == 1){ //Deadlock detected.
            *(req + trainx * numInter + idx) = 0;
            preemption(trainx, train_name, req_id, res_id, numInter, numTrains, intersections, trains, (int*) req, (int*) alloc); //Preemption routine; victimizes train that would cause deadlock.
            *(req + trainx * numInter + idx) = 1;
        }
        send_request(req_id, train_name, inter_name);  // Resend request
        receive_response(res_id, &response);  // Wait for new response
    }

    if (strcmp(response.response, "GRANT") == 0) {
        if (inter->type == MUTEX) {
            pthread_mutex_lock(&inter->mutex);
        } else {
            sem_wait(&inter->semaphore);
        }
        *(req + trainx * numInter + idx) = 0; //Move resource from request matrix to allocation one.
        *(alloc + trainx * numInter + idx) = 1;
        printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, *(alloc + trainx * numInter + idx));
        add_train_to_holding(inter, train_name);
        printf("%s is passing through %s.\n", train_name, inter->name);
        sleep(2); // Simulates traversal time

        send_response(res_id, "RELEASE");
    }
}

void release_intersection(const char* train_name, const char* inter_name, int req_id, int res_id, int* req, int* alloc, int numTrains, int numInter, Intersection* intersections, char** trains) {
    int idx = find_intersection_index(inter_name, numInter, intersections);
    int trainx = find_train_index(train_name, numTrains, trains);
    if (idx == -1 || trainx == -1) return;

    Intersection* inter = &intersections[idx];

    if (inter->type == MUTEX) {
        pthread_mutex_unlock(&inter->mutex);
    } else {
        sem_post(&inter->semaphore);
    }
    *(alloc + trainx * numInter + idx) = 0; //Remove resource from allocation matrix.
    printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, *(alloc + trainx * numInter + idx));
    remove_train_from_holding(inter, train_name);
    printf("%s has left %s.\n", train_name, inter->name);
    send_response(res_id, "GRANT");
}
