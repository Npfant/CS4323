// Ashton Cecil
// Date: 4/11/2025
// Email: acecil@okstate.edu
// Program Description: Handles train access to intersections

#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H
#define MAX_NAME_LEN 32
#define MAX_HOLDING 10
#define MAX_LINE_LEN 100

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

void add_train_to_holding(Intersection* inter, const char* train_name) {    //changed by me
    if (inter->num_holding < MAX_HOLDING) {        
       strncpy(inter->holding_trains[inter->num_holding], train_name, MAX_NAME_LEN - 1);
       inter->holding_trains[inter->num_holding][MAX_NAME_LEN - 1] = '\0';  // Ensure null termination
       inter->num_holding++;
   } else {
       printf("ERROR: Holding capacity reached at %s\n", inter->name);
   }
}

void remove_train_from_holding(Intersection* inter, const char* train_name) {
   for (int i = 0; i < inter->num_holding; i++) {
       if (strcmp(inter->holding_trains[i], train_name) == 0) {
           for (int j = i; j < inter->num_holding - 1; j++) {
               strcpy(inter->holding_trains[j], inter->holding_trains[j + 1]);
           }
           inter->num_holding--;
           break;
       }
   }
}

void preemption(int trainx, const char* train_name, int req_id, int res_id, int numInter, int numTrains, Intersection* intersections, char** trains);

bool rat(int numInter, int numTrain, Intersection* intersections);

// This is the core logic (most key part)
void acquire_intersection(const char* train_name, const char* inter_name, int req_id, int res_id, int numTrains, int numInter, Intersection* intersections, char** trains) {

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
    req[trainx][idx] = 1;
    printf("Train ID: %d, Intersection ID: %d, Request: %d  \n", trainx, idx, req[trainx][idx]);
    printf("%s is waiting at %s.\n", train_name, inter->name);
    send_request(req_id, train_name, inter_name);
    
    bool check_rat = 0; //Check for deadlock.
    bool cycle = 0;
    if(check_rat == 0){
        cycle = rat(numTrains, numInter, intersections); //Call resource allocation table method to check whether or not a deadlock has occurred.
        check_rat = 1;
    }
    if(cycle == 1){ //Deadlock detected.
        req[trainx][idx] = 0;
        preemption(trainx, train_name, req_id, res_id, numInter, numTrains, intersections, trains); //Preemption routine; victimizes train that would cause deadlock.
        req[trainx][idx] = 1;
    }

    struct response_msg response;
    receive_response(res_id, &response);
    while (strcmp(response.response, "WAIT") == 0) {
        printf("%s is waiting for permission at %s.\n", train_name, inter->name);
        /*if(check_rat == 0){
            cycle = rat(numTrains, numInter, intersections, (int*) req, (int*) alloc); //Call resource allocation table method to check whether or not a deadlock has occurred.
            check_rat = 1;
        }
        if(cycle == 1){ //Deadlock detected.
            *(req + trainx * numInter + idx) = 0;
            preemption(trainx, train_name, req_id, res_id, numInter, numTrains, intersections, trains, (int*) req, (int*) alloc); //Preemption routine; victimizes train that would cause deadlock.
            *(req + trainx * numInter + idx) = 1;
        }*/
        send_request(req_id, train_name, inter_name);  // Resend request
        receive_response(res_id, &response);  // Wait for new response
    }

    if (strcmp(response.response, "GRANT") == 0) {
        lock(inter);
        req[trainx][idx] = 0; //Move resource from request matrix to allocation one.
        alloc[trainx][idx] = 1;
        //printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, *(alloc + trainx * numInter + idx));
        add_train_to_holding(inter, train_name);
        printf("%s is passing through %s.\n", train_name, inter->name);
        sleep(2);
        send_response(res_id, "RELEASE");
    }
}

void release_intersection(const char* train_name, const char* inter_name, int req_id, int res_id, int numTrains, int numInter, Intersection* intersections, char** trains) {
    
    int idx = find_intersection_index(inter_name, numInter, intersections);
    int trainx = find_train_index(train_name, numTrains, trains);
    if (idx == -1 || trainx == -1) return;

    Intersection* inter = &intersections[idx];

    unlock(inter);
    alloc[trainx][idx] = 0; //Remove resource from allocation matrix.
    //printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, *(alloc + trainx * numInter + idx));
    remove_train_from_holding(inter, train_name);
    printf("%s has left %s.\n", train_name, inter->name);
    send_response(res_id, "GRANT");
}

#endif
