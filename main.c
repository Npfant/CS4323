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

#define NUM_TRAINS 5
#define MAX_NAME_LEN 32
#define MAX_HOLDING 10

char trains[5][100];         //Store train data in array of 5 lines

// Message structure for request queue (train to server)
struct request_msg {
    long msg_type;        // Message type (should be > 0)
    char train_name[MAX_NAME_LEN];
    char intersection[MAX_NAME_LEN];
};

// Message structure for response queue (server to train)
struct response_msg {
    long msg_type;        // Message type (should be > 0)
    char response[MAX_NAME_LEN];
};

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

Intersection intersections[5];

int num_intersections = sizeof(intersections) / sizeof(Intersection);

key_t key;
int shmAlloc;
int shmReq;
int (*alloc)[5]; //Initialize allocation and resource matricies to number of trains and intersections.
int (*req)[5];

// -------------------- HELPER FUNCTIONS --------------------
int find_intersection_index(const char* name) {
    for (int i = 0; i < num_intersections; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_train_index(const char* name) {
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

void send_request(int req_id, const char* train_name, const char* inter_name) {
    struct request_msg msg;
    msg.msg_type = 1;
    strncpy(msg.train_name, train_name, MAX_NAME_LEN - 1);
    strncpy(msg.intersection, inter_name, MAX_NAME_LEN - 1);
    if (msgsnd(req_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}

void send_response(int res_id, const char* response) {
    struct response_msg msg;
    msg.msg_type = 1;
    strncpy(msg.response, response, MAX_NAME_LEN - 1);
    if (msgsnd(res_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}

// Receive request from the request queue
void receive_request(int req_id, struct request_msg* msg) {
    if (msgrcv(req_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv failed");
    }
}

// Receive response from the response queue
void receive_response(int res_id, struct response_msg* msg) {
    if (msgrcv(res_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv failed");
    }
}

bool rat(){  //Resource allocation table method
    int avail[num_intersections];
    printf("Alloc Matrix: ");
    for(int i = 0; i < num_intersections; i++){
        avail[i] = intersections[i].capacity;  //Max that can be allocated.
        for(int j = 0; j < NUM_TRAINS; j++){
            printf("%d", alloc[i][j]);
            if(alloc[j][i] > 0){
                avail[i] -= alloc[j][i]; //Decrements available from max to not currently allocated.
            }
        }
        printf("\n");
        //printf("%d ",avail[i]);
    }
    printf("\n");
    bool cycle = 1; //Start with an assumed cycle
    //printf("Req Matrix: ");
    for(int i = 0; i < NUM_TRAINS; i++){
        for(int j = 0; j < num_intersections; j++){
            //printf("%d", req[i][j]);
            if((avail[j] - req[i][j]) < 0){
                break; //Stops checking request if it is larger than available.
            }
            if(j == (num_intersections - 1)){
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

void preemption(int trainx, const char* train_name, int req_id, int res_id){
    for(int i = 0; i < num_intersections; i++){
        if(alloc[trainx][i] == 1){
            Intersection* inter = &intersections[i];
            if (inter->type == MUTEX) {
                pthread_mutex_unlock(&inter->mutex);
            } else {
                sem_post(&inter->semaphore);
            }
            alloc[trainx][i] = 0;
            req[trainx][i] = 1;
        }
    }
    for(int i = 0; i < num_intersections; i++){
        if(req[trainx][i] == 1){
            Intersection* inter = &intersections[i];
            acquire_intersection(train_name, inter->name, req_id, res_id);    //train enter :D
            release_intersection(train_name, inter->name, req_id, res_id);    //train leave :(
        }
    }
}

// This is the core logic (most key part)
void acquire_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    int trainx = find_train_index(train_name);
    if (idx == -1) {
        printf("ERROR: Unknown intersection %s.\n", inter_name);
        return;
    }
    if (trainx == -1) {
        printf("ERROR: Unknown train %s.\n", train_name);
        return;
    }

    Intersection* inter = &intersections[idx];
    req[trainx][idx] = 1; //Add resource to request matrix.
    printf("Train ID: %d, Intersection ID: %d, Request: %d  \n", trainx, idx, req[trainx][idx]);
    printf("%s is waiting at %s.\n", train_name, inter->name);
    send_request(req_id, train_name, inter_name);

    struct response_msg response;
    receive_response(res_id, &response);
    bool check_rat = 0; //Check for deadlock.
    bool cycle = 0;
    while (strcmp(response.response, "WAIT") == 0) {
        printf("%s is waiting for permission at %s.\n", train_name, inter->name);
        if(check_rat == 0){
            cycle = rat(); //Call resource allocation table method to check whether or not a deadlock has occurred.
            check_rat = 1;
        }
        if(cycle == 1){ //Deadlock detected.
            req[trainx][idx] = 0;
            preemption(trainx, train_name, req_id, res_id); //Preemption routine; victimizes train that would cause deadlock.
            req[trainx][idx] = 1;
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
        req[trainx][idx] = 0; //Move resource from request matrix to allocation one.
        alloc[trainx][idx] = 1;
        printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, alloc[trainx][idx]);
        add_train_to_holding(inter, train_name);
        printf("%s is passing through %s.\n", train_name, inter->name);
        sleep(2); // Simulates traversal time

        send_response(res_id, "RELEASE");
    }
}

void release_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    int trainx = find_train_index(train_name);
    if (idx == -1 || trainx == -1) return;

    Intersection* inter = &intersections[idx];

    if (inter->type == MUTEX) {
        pthread_mutex_unlock(&inter->mutex);
    } else {
        sem_post(&inter->semaphore);
    }
    alloc[trainx][idx] = 0; //Remove resource from allocation matrix.
    printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, alloc[trainx][idx]);
    remove_train_from_holding(inter, train_name);
    printf("%s has left %s.\n", train_name, inter->name);
    send_response(res_id, "GRANT");
}

void handle_request(int req_id, int res_id) {
    struct request_msg request;
    struct response_msg response;
    
    while (1) {
        receive_request(req_id, &request);
        printf("Server received request from %s for %s\n", request.train_name, request.intersection);
        
        int idx = find_intersection_index(request.intersection);
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
        acquire_intersection(train_name, interName, req_id, res_id);    //train enter :D
        release_intersection(train_name, interName, req_id, res_id);    //train leave :(
        interName = strtok(NULL, ",\t\r\n\v\f\b");              //GET THE NEXT ONE
    }
    exit(0);
}

void createBuf1() //Create request matrix shared memory space.
{
  key = ftok(".",'b');
  shmReq = shmget(key,sizeof(int[5][5]),IPC_CREAT|0666);

  if(shmReq == -1 )
  {  
    perror("shmget");
    exit(1);
  }
  else
  {  
    printf("Creating new shared memory segment\n");
    req = shmat(shmReq,0,0);
    if(req == (void*) -1 )
    {  
      perror("shmat");
      exit(1);
    }
  }  
}

void createBuf2() //Create allocation matrix shared memory.
{
  key = ftok(".",'c');
  shmAlloc = shmget(key,sizeof(int[5][5]),IPC_CREAT|0666);

  if(shmAlloc == -1 )
  {  
    perror("shmget");
    exit(1);
  }
  else
  {  
    printf("Creating new shared memory segment\n");
    alloc = shmat(shmAlloc,0,0);
    if(alloc == (void*) -1 )
    {  
      perror("shmat");
      exit(1);
    }
  }  
}

int main() {
    // Create two message queues: one for requests, one for responses
    int req_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    int res_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    //Create request and allocation matricies in shared memory.
    createBuf1();
    createBuf2();
    
    FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
    FILE *trains_init = fopen("trains.txt","r");               //Read trains file
  
    int height = 5;
    for(int i = 0; i < height; i++){ //Loop to read lines into arrays
      char temp[100];
      fgets(temp,100,intersections_init);     //read line from intersections file into temp
      fgets(trains[i],100,trains_init);       //read line from trains file directly into trains array
      char delim[] = ":\t\r\n\v\f\b";  
      char* interName = strtok(temp, delim);    //copy name of intersection into interName
      char* tempCap = strtok(NULL, ":");     //copy intersection capacity into capacity
      int cap;
      sscanf(tempCap, "%d", &cap);
        
      if(cap > 1){                        //if capacity > 1: make locktype semaphore
        strcpy(intersections[i].name, interName);        //name
        intersections[i].type = SEMAPHORE;               //locktype
        intersections[i].capacity = cap;                 //capacity
      }else{                                         //else make locktype mutex
        strcpy(intersections[i].name, interName);        //name
        intersections[i].type = MUTEX;                   //locktype
        intersections[i].capacity = cap;                 //capacity
      }
    }
    

    if (req_id == -1 || res_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    for (int i = 0; i < num_intersections; i++) {
        if (intersections[i].type == MUTEX) {
            pthread_mutex_init(&intersections[i].mutex, NULL);
        } else {
            sem_init(&intersections[i].semaphore, 1, intersections[i].capacity);
        }
    }

    for (int i = 0; i < NUM_TRAINS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            train_behavior(trains[i], req_id, res_id);
        }
    }

    // Server processing requests
    for (int i = 0; i < NUM_TRAINS; i++) {
        struct request_msg request;
        receive_request(req_id, &request);
        printf("Server received request from %s for %s\n", request.train_name, request.intersection);

        // Handle request (grant or deny)
        send_response(res_id, "GRANT");  // Simple grant for now
    }

    // Wait for all trains to finish
    for (int i = 0; i < NUM_TRAINS; i++) {
        wait(NULL);
    }

    printf("Simulation complete. All trains finished.\n");

    // Cleanup message queues
    msgctl(req_id, IPC_RMID, NULL);
    msgctl(res_id, IPC_RMID, NULL);

    return 0;
}