//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 04/11/2025
//Program Description: Implementation of a preemption deadlock mitigation strategy

#include "intersections.h"
#ifndef PREEMPTION_H
#define PREEMPTION_H

void preemption(int trainx, const char* train_name, int req_id, int res_id, int numInter, int numTrains, Intersection* intersections, char** trains, int* req, int* alloc){
    for(int i = 0; i < numInter; i++){
        if(*(alloc + trainx * numInter + i) == 1){
            Intersection* inter = &intersections[i];
            if (inter->type == MUTEX) {
                pthread_mutex_unlock(&inter->mutex);
            } else {
                sem_post(&inter->semaphore);
            }
            *(alloc + trainx * numInter + i) = 0;
            *(req + trainx * numInter + i) = 1;
        }
    }
    for(int i = 0; i < numInter; i++){
        if(*(req + trainx * numInter + i) == 1){
            Intersection* inter = &intersections[i];
            acquire_intersection(train_name, inter->name, req_id, res_id, (int*) req, (int*) alloc, numTrains, numInter, intersections, trains);    //train enter :D
            release_intersection(train_name, inter->name, req_id, res_id, (int*) req, (int*) alloc, numTrains, numInter, intersections, trains);    //train leave :(
        }
    }
}

#endif