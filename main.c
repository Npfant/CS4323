#include "preemption.h"
#include "initialization.h"

#define MAX_NAME_LEN 32
#define MAX_HOLDING 10
#define MAX_LINE_LEN 100

//Global variables
char** trains = NULL;    
Intersection* intersections = NULL;
int NUM_TRAINS = 0;
int NUM_INTERSECTIONS = 0;
int shmReq;
int shmAlloc;


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
void train_behavior(char* train_info, int req_id, int res_id, int* req, int* alloc) {
    char* train_name = strtok(train_info, ":");       //get dat train name
    char* interName = strtok(NULL, ":");              //get da intersections list
    interName = strtok(interName, ",");               //now get only the first one
    while (interName != NULL) {                       //if there are still more intersections, GET ANOTHER ONE
        
        acquire_intersection(train_name, interName, req_id, res_id, (int*) req, (int*) alloc, NUM_TRAINS, NUM_INTERSECTIONS, intersections, trains);    //train enter :D
        release_intersection(train_name, interName, req_id, res_id, (int*) req, (int*) alloc, NUM_TRAINS, NUM_INTERSECTIONS, intersections, trains);    //train leave :(
        interName = strtok(NULL, ",\t\r\n\v\f\b");              //GET THE NEXT ONE
    }
    exit(0);
}

//Counts the lines (basically the amount of trains/intersections) in the respective files.
int countLines(FILE *filename){
    int currentLine = 1;
    char c;
    
    do{
      c = fgetc(filename);
      
      if (c == '\n'){
        currentLine++;
      }
      
    } while (c != EOF);
    
    return currentLine;
}

void createBuf1(int NUM_TRAINS, int NUM_INTERSECTIONS, int* req, key_t key) //Create request matrix shared memory space.
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

void createBuf2(int NUM_TRAINS, int NUM_INTERSECTIONS, int* alloc, key_t key) //Create allocation matrix shared memory.
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
    
    //Get counts
    
    //Test
    printf("Found %d trains and %d intersections\n", NUM_TRAINS, NUM_INTERSECTIONS);
    
    //Allocate memory for trains and intersections
    trains = (char**)getTrains();
    intersections = (Intersection*)getIntersections();
    NUM_TRAINS = sizeof(trains);
    NUM_INTERSECTIONS = sizeof(intersections);
    
    //Allocate memory for each train string
    for (int i = 0; i < NUM_TRAINS; i++) {
        trains[i] = (char*)malloc(MAX_LINE_LEN * sizeof(char));
    }
    
    //Create request and allocation matricies in shared memory
    int req[NUM_TRAINS][NUM_INTERSECTIONS]; //Initialize allocation and resource matricies to number of trains and intersections.
    int alloc[NUM_TRAINS][NUM_INTERSECTIONS];
    key_t key1 = ftok(".", 'b');
    key_t key2 = ftok(".", 'c');
    createBuf1(NUM_TRAINS, NUM_INTERSECTIONS, (int*) req, key1);
    createBuf2(NUM_TRAINS, NUM_INTERSECTIONS, (int*) alloc, key2);

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

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

    //Initialize mutex and semaphore locks
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        if (intersections[i].type == MUTEX) {
            pthread_mutex_init(&intersections[i].mutex, &mattr);
        } else {
            sem_init(&intersections[i].semaphore, 1, intersections[i].capacity);
        }
    }

    //Fork trains
    for (int i = 0; i < NUM_TRAINS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            train_behavior(trains[i], req_id, res_id, (int*) req, (int*) alloc);
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
    
    //Free the allocated memory
    for (int i = 0; i < NUM_TRAINS; i++) {
        free(trains[i]);
    }
    free(trains);
    free(intersections);
    
    //Close files
    fclose(intersections_init);
    fclose(trains_init);

    return 0;
}
