//Luis Guzman
//Main file that works with Logger.h, nathan_file.c, intersections.txt, trains.txt
//I added a shared memory process with the createSharedIntersections function
//added onto main function.
//I also added the deadlock detection

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

// Message structure for request queue
struct request_msg {
    long msg_type;
    char train_name[MAX_NAME_LEN];
    char intersection[MAX_NAME_LEN];
};

// Message structure for response queue
struct response_msg {
    long msg_type;
    char response[MAX_NAME_LEN];
};

// Lock types
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

// Global vars
char** trains = NULL;
Intersection* intersections = NULL;
Intersection* shared_intersections = NULL;// This was added for shared memory -Luis
int NUM_TRAINS = 0;
int NUM_INTERSECTIONS = 0; //This could either be 5 or 0 -Luis

key_t key;
int shmAlloc;
int shmReq;
int shmInter;

int (*alloc)[5]; // [NUM_TRAINS][NUM_INTERSECTIONS]
int (*req)[5];

// ------------------------------------
// Utility + shared memory helpers
// ------------------------------------

int countLines(FILE *filename){
    int count = 1;
    char c;
    while ((c = fgetc(filename)) != EOF) {
        if (c == '\n') count++;
    }
    rewind(filename); //reads the file from start
    return count;
}

int find_intersection_index(const char* name) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        if (strcmp(intersections[i].name, name) == 0) 
            return i;
    }
    return -1;
}

int find_train_index(const char* name) {
    for (int i = 0; i < NUM_TRAINS; i++) {
        if (strncmp(trains[i], name, strlen(name)) == 0) return i; //strlen(name) was added by me -Luis
    }
    return -1;
}

void add_train_to_holding(Intersection* inter, const char* train_name) {
    if (inter->num_holding < MAX_HOLDING) {
        strncpy(inter->holding_trains[inter->num_holding], train_name, MAX_NAME_LEN - 1);
        inter->holding_trains[inter->num_holding][MAX_NAME_LEN - 1] = '\0';
        inter->num_holding++;
    }
    else {
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
    if(msgsnd(req_id, &msg, sizeof(msg) - sizeof(long), 0) == -1){
        perror("msgsnd failed");
    }
}

void send_response(int res_id, const char* response) {
    struct response_msg msg;
    msg.msg_type = 1;
    strncpy(msg.response, response, MAX_NAME_LEN - 1);
    if(msgsnd(res_id, &msg, sizeof(msg) - sizeof(long), 0) == -1){
        perror("msgnd failed");
    }
}

void receive_request(int req_id, struct request_msg* msg) {
    if(msgrcv(req_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1){
        perror("msgrcv failed");
    }
}

void receive_response(int res_id, struct response_msg* msg) {
    if(msgrcv(res_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1){
        perror("msgrcv failed");
    }
}

// ------------------------------------
// Deadlock detection (Resource Allocation Table)
void rat() {
    int avail[NUM_INTERSECTIONS];
    printf("Alloc Matrix: ");
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        avail[i] = intersections[i].capacity;
        for (int j = 0; j < NUM_TRAINS; j++) {
            printf("%d", alloc[j][i]);
            if (alloc[j][i] > 0)
                avail[i] -= alloc[j][i];
        }
    }
    printf("\n");
    
    bool cycle = true; //Changed it to true-Luis
    for (int i = 0; i < NUM_TRAINS; i++) {
        bool can_satisfy = true; //Added this -Luis
        for (int j = 0; j < NUM_INTERSECTIONS; j++) {
            if (req[i][j] > avail[j]) { //
                can_satisfy = false;    //
                break;                  //
            }                           //
        }                               //
        if (can_satisfy) {              //added this-Luis
            cycle = false;              //
            break;                      //
        }                               //
    }                                   //

    if (cycle == 1){
        printf("DEADLOCK!\n");
    } else{
        printf("NO DEADLOCK\n");
    }
}

void* deadlock_detector(void* arg) { //Deadlock detection - Luis
    while (1) { //Created by Luis
        sleep(5);
        printf("\n[Deadlock Detector] Checking for deadlocks...\n");
        rat();
        printf("[Deadlock Detector] Check complete.\n");
    }
    return NULL;
}

// ------------------------------------
// Intersection handling logic
void acquire_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    int trainx = find_train_index(train_name);

    if (idx == -1){
        printf("ERROR: Unknown interaction %s.\n", inter_name);
        return;
    }
    if(trainx == -1){
        printf("ERROR: Unknown train %s.\n", train_name);
        return;
    }

    Intersection* inter = &intersections[idx];
    req[trainx][idx] = 1;
    printf("Train ID: %d, Intersection ID: %d, Requesting: %d\n", trainx, idx, req[trainx][idx]);
    rat();
    printf("%s is waiting at %s.\n", train_name, inter->name);
    send_request(req_id, train_name, inter_name);
    
    struct response_msg response;
    receive_response(res_id, &response);

    while (strcmp(response.response, "WAIT") == 0) {
        printf("%s waiting for permission at %s\n", train_name, inter->name);
        sleep(1);
        send_request(req_id, train_name, inter_name);
        receive_response(res_id, &response);
    }

    if (strcmp(response.response, "GRANT") == 0) {
        if (inter->type == MUTEX){ 
            pthread_mutex_lock(&inter->mutex);
        }
        else {
            sem_wait(&inter->semaphore);
        }

        req[trainx][idx] = 0;
        alloc[trainx][idx] = 1;
        
        printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, alloc[trainx][idx]);
        add_train_to_holding(inter, train_name);
        printf("%s is passing through %s.\n", train_name, inter->name);
        
        sleep(2); // simulate crossing
        send_response(res_id, "RELEASE");
    }
}

void release_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    int trainx = find_train_index(train_name);
    if (idx == -1 || trainx == -1) return;

    Intersection* inter = &intersections[idx];

    if (inter->type == MUTEX) pthread_mutex_unlock(&inter->mutex);
    else sem_post(&inter->semaphore);

    alloc[trainx][idx] = 0;
    printf("Train ID: %d, Intersection ID: %d, Allocation: %d  \n", trainx, idx, alloc[trainx][idx]);
    remove_train_from_holding(inter, train_name);
    printf("%s has left %s.\n", train_name, inter->name);
    send_response(res_id, "GRANT");

   // printf("%s released %s\n", train_name, inter->name);
}

// ------------------------------------
// Server to handle requests centrally
void handle_request(int req_id, int res_id) {
    //struct request_msg request; //***CHECK
    struct request_msg request;
    while (1) {
        receive_request(req_id, &request);
        printf("Server received request from %s for %s\n", request.train_name, request.intersection);
        int idx = find_intersection_index(request.intersection);
        if (idx == -1){
            printf("ERROR: Unknown intersection %s\n", request.intersection);
            continue;
        } 
        
        Intersection* inter = &intersections[idx];
        if (inter->num_holding < inter->capacity) {
            send_response(res_id, "GRANT");
            add_train_to_holding(inter, request.train_name);
        } else {
            send_response(res_id, "WAIT");
        }
    }
}

// ------------------------------------
// Train logic
void train_behavior(char* train_info, int req_id, int res_id) {
    char* train_name = strtok(train_info, ":");
    char* interName = strtok(NULL, ":");
    interName = strtok(interName, ",");
    while (interName != NULL) {
        acquire_intersection(train_name, interName, req_id, res_id);
        release_intersection(train_name, interName, req_id, res_id);
        interName = strtok(NULL, ",\t\r\n\v\f\b");
    }
    exit(0);
}

// ------------------------------------
// Shared Memory Setup
void createBuf1() {
    key = ftok(".", 'b');
    shmReq = shmget(key, sizeof(int[5][5]), IPC_CREAT | 0666);
    //req = (int (*)[5])shmat(shmReq, NULL, 0);
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

void createBuf2() {
    key = ftok(".", 'c');
    shmAlloc = shmget(key, sizeof(int[5][5]), IPC_CREAT | 0666);
   // alloc = (int (*)[5])shmat(shmAlloc, NULL, 0);
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

void createSharedIntersections() {
    key = ftok(".", 'i');
    shmInter = shmget(key, sizeof(Intersection) * 5, IPC_CREAT | 0666);
    shared_intersections = (Intersection*)shmat(shmInter, NULL, 0);
    intersections = shared_intersections;
}

// ------------------------------------
// Main function
int main() {
    int req_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    int res_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    createBuf1();
    createBuf2();
    createSharedIntersections();

    FILE *intersections_init = fopen("intersections.txt", "r");
    FILE *trains_init = fopen("trains.txt", "r");

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

    NUM_TRAINS = countLines(trains_init);
    NUM_INTERSECTIONS = countLines(intersections_init);
    printf("Found %d trains and %d intersections\n", NUM_TRAINS, NUM_INTERSECTIONS);

    trains = (char**)malloc(NUM_TRAINS * sizeof(char*));
    for (int i = 0; i < NUM_TRAINS; i++) {
        trains[i] = (char*)malloc(MAX_LINE_LEN * sizeof(char));
        fgets(trains[i], MAX_LINE_LEN, trains_init);
    }

    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        char temp[MAX_LINE_LEN];
        fgets(temp, MAX_LINE_LEN, intersections_init);
        char* interName = strtok(temp, ":");
        char* tempCap = strtok(NULL, ":");
        int cap = atoi(tempCap);
        strcpy(intersections[i].name, interName);
        intersections[i].capacity = cap;
        intersections[i].type = (cap > 1) ? SEMAPHORE : MUTEX;
    }

    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        if (intersections[i].type == MUTEX) {
            pthread_mutex_init(&intersections[i].mutex, &mattr);
        } else {
            sem_init(&intersections[i].semaphore, 1, intersections[i].capacity);
        }
    }
    pthread_t detector_thread; //Deadlock detection -Luis
    pthread_create(&detector_thread, NULL, deadlock_detector, NULL); //Luis

    for (int i = 0; i < NUM_TRAINS; i++) {
        if (fork() == 0) {
            train_behavior(trains[i], req_id, res_id);
        }
    }

    handle_request(req_id, res_id); // Main server loop

    for (int i = 0; i < NUM_TRAINS; i++) {
        wait(NULL);
    }

    msgctl(req_id, IPC_RMID, NULL);
    msgctl(res_id, IPC_RMID, NULL);

    for (int i = 0; i < NUM_TRAINS; i++) free(trains[i]);
    free(trains);
    fclose(intersections_init);
    fclose(trains_init);

    return 0;
}
