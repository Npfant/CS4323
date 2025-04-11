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

// -------------------- HELPER FUNCTIONS --------------------
int find_intersection_index(const char* name) {
    for (int i = 0; i < num_intersections; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
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

// This is the core logic (most key part)
void acquire_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    if (idx == -1) {
        printf("ERROR: Unknown intersection %s\n", inter_name);
        return;
    }

    Intersection* inter = &intersections[idx];

    printf("%s is waiting at %s.\n", train_name, inter->name);
    send_request(req_id, train_name, inter_name);

    struct response_msg response;
    receive_response(res_id, &response);

    while (strcmp(response.response, "WAIT") == 0) {
        printf("%s is waiting for permission at %s.\n", train_name, inter->name);
        sleep(1);  // Retry after a delay (for simplicity)
        send_request(req_id, train_name, inter_name);  // Resend request
        receive_response(res_id, &response);  // Wait for new response
    }

    if (strcmp(response.response, "GRANT") == 0) {
        if (inter->type == MUTEX) {
            pthread_mutex_lock(&inter->mutex);
        } else {
            sem_wait(&inter->semaphore);
        }

        add_train_to_holding(inter, train_name);
        printf("%s is passing through %s.\n", train_name, inter->name);
        sleep(2); // Simulates traversal time

        send_response(res_id, "RELEASE");
    }
}

void release_intersection(const char* train_name, const char* inter_name, int req_id, int res_id) {
    int idx = find_intersection_index(inter_name);
    if (idx == -1) return;

    Intersection* inter = &intersections[idx];

    if (inter->type == MUTEX) {
        pthread_mutex_unlock(&inter->mutex);
    } else {
        sem_post(&inter->semaphore);
    }

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
void train_behavior(const char* train_name, int req_id, int res_id) {
    if (strcmp(train_name, "Train1") == 0) {
        acquire_intersection(train_name, "Intersection A", req_id, res_id);
        release_intersection(train_name, "Intersection A", req_id, res_id);

        acquire_intersection(train_name, "Intersection B", req_id, res_id);
        release_intersection(train_name, "Intersection B", req_id, res_id);

        acquire_intersection(train_name, "Intersection C", req_id, res_id);
        release_intersection(train_name, "Intersection C", req_id, res_id);
    } else if (strcmp(train_name, "Train2") == 0) {
        acquire_intersection(train_name, "Intersection B", req_id, res_id);
        release_intersection(train_name, "Intersection B", req_id, res_id);

        acquire_intersection(train_name, "Intersection D", req_id, res_id);
        release_intersection(train_name, "Intersection D", req_id, res_id);

        acquire_intersection(train_name, "Intersection E", req_id, res_id);
        release_intersection(train_name, "Intersection E", req_id, res_id);
    } else if (strcmp(train_name, "Train3") == 0) {
        acquire_intersection(train_name, "Intersection C", req_id, res_id);
        release_intersection(train_name, "Intersection C", req_id, res_id);

        acquire_intersection(train_name, "Intersection D", req_id, res_id);
        release_intersection(train_name, "Intersection D", req_id, res_id);

        acquire_intersection(train_name, "Intersection A", req_id, res_id);
        release_intersection(train_name, "Intersection A", req_id, res_id);
    } else if (strcmp(train_name, "Train4") == 0) {
        acquire_intersection(train_name, "Intersection E", req_id, res_id);
        release_intersection(train_name, "Intersection E", req_id, res_id);

        acquire_intersection(train_name, "Intersection B", req_id, res_id);
        release_intersection(train_name, "Intersection B", req_id, res_id);

        acquire_intersection(train_name, "Intersection D", req_id, res_id);
        release_intersection(train_name, "Intersection D", req_id, res_id);
    }

    exit(0);
}

int main() {
    // Create two message queues: one for requests, one for responses
    int req_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    int res_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    
    FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
    FILE *trains_init = fopen("trains.txt","r");               //Read trains file
  
    for(int i = 0; i < 5; i++){ //Loop to read lines into arrays
      char temp[100];
      fgets(temp,100,intersections_init);     //read line from intersections file into temp
      fgets(trains[i],100,trains_init);       //read line from trains file directly into trains array
        
      char* interName = strtok(temp, ":");    //copy name of intersection into interName
      char* capacity = strtok(NULL, ":");     //copy intersection capacity into capacity
        
      if(capacity - '0' > 1){                        //if capacity > 1: make locktype semaphore
        strcpy(intersections[i].name, interName);        //name
        intersections[i].type = SEMAPHORE;               //locktype
        strcpy(intersections[i].capacity, capacity);     //capacity
      }else{                                         //else make locktype mutex
        strcpy(intersections[i].name, interName);        //name
        intersections[i].type = MUTEX;                   //locktype
        strcpy(intersections[i].capacity, capacity);     //capacity
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

    const char* trains[NUM_TRAINS] = {"Train1", "Train2", "Train3", "Train4"};
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
