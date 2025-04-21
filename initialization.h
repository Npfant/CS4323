//Group:    Group F
//Name:     Dylan Palmese
//Email:    dylan.palmese@okstate.edu
//Date:     4/17/2025

#ifndef INITIALIZATION_H
#define INITIALIZATION_H
#define MAX_NAME_LEN 32
#define MAX_HOLDING 10
#define MAX_LINE_LEN 100

// Intersection LockType enum (MUTEX or SEMAPHORE)
typedef enum { MUTEX, SEMAPHORE } LockType;

typedef struct {
    char name[MAX_NAME_LEN];
    LockType type;
    int capacity;

    pthread_mutex_t *mutex;
    sem_t semaphore;

    char holding_trains[MAX_HOLDING][MAX_NAME_LEN];
    int num_holding;
} Intersection;

int trainNum;    //tracker for number of trains
int interNum;    //tracker for number of intersections

int howManyTrains(){
  return trainNum;    //function to return number of trains
}
int howManyInters(){
  return interNum;    //function to return number of intersections
}

char** getTrains(){
  
  char** trains;
  FILE *trains_init = fopen("trains.txt","r");  //read trains file
  trainNum = countLines(trains_init);           //set number of trains variable
  rewind(trains_init);                          //reset to top of file
  trains = (char**)malloc(trainNum * sizeof(char*));    //allocate memory
  char nxtLine[100];
  for(int i = 0; i < trainNum; i++){
    fgets(nxtLine, 100, trains_init);        
    trains[i] = malloc(100 * sizeof(char));             //allocate memory for each element
    strcpy(trains[i], nxtLine);       //read line from trains file directly into trains array
  }
  fclose(trains_init);                //close file
  //printf("-%d-\n", trainNum);
  return trains;
}

Intersection* getIntersections(){

  Intersection* intersections;
  FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
  interNum = countLines(intersections_init);                 //set number of intersections variable
  rewind(intersections_init);                                //reset to top of file
  intersections = (Intersection*)malloc(interNum * sizeof(Intersection));    //allocate memory
  char nxtLine[100];
  for(int i = 0; i < interNum; i++){
    fgets(nxtLine, 100, intersections_init);
    char* interName = strtok(nxtLine, ":\n");    //copy name of intersection into interName
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
  fclose(intersections_init);                //close file
  return intersections;
}

void mutexes(int NUM_INTERSECTIONS, Intersection* intersections){
  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

  //Initialize mutex and semaphore locks
  for (int i = 0; i < NUM_INTERSECTIONS; i++) {
      if (intersections[i].type == MUTEX) {
        int shmMut = shmget(i,sizeof(pthread_mutex_t),IPC_CREAT|0666);
      
        if(shmMut == -1 )
        {  
          perror("shmget");
          exit(1);
        }
        else
        {  
          printf("Creating new shared memory segment\n");
          intersections[i].mutex = (pthread_mutex_t*) shmat(shmMut,0,0);
        }  
          pthread_mutex_init(intersections[i].mutex, &mattr);
      } else {
          sem_init(&intersections[i].semaphore, 1, intersections[i].capacity);
      }
  }
}

void lock(Intersection* inter){
  if (inter->type == MUTEX) {
    pthread_mutex_lock(inter->mutex);
  } else {
      sem_wait(&inter->semaphore);
  }
}
void unlock(Intersection* inter){
  if (inter->type == MUTEX) {
    pthread_mutex_unlock(inter->mutex);
  } else {
      sem_post(&inter->semaphore);
  }
}

int shmAlloc;
int shmReq;
int (*alloc)[100]; //Initialize allocation and resource matricies to number of trains and intersections.
int (*req)[100];

void createBuf1(int NUM_TRAINS, int NUM_INTERSECTIONS, key_t key)
{
  key = ftok(".",'b');
  shmReq = shmget(key,sizeof(int[NUM_TRAINS][NUM_INTERSECTIONS]),IPC_CREAT|0666);

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

void createBuf2(int NUM_TRAINS, int NUM_INTERSECTIONS, key_t key)
{
  key = ftok(".",'c');
  shmAlloc = shmget(key,sizeof(int[NUM_TRAINS][NUM_INTERSECTIONS]),IPC_CREAT|0666);

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
#endif