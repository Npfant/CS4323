//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 04/13/2025
//Program Description: Implementation of a resource allocation table

#ifndef RAT_H
#define RAT_H

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