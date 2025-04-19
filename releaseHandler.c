// Group Project CS 4323 - Ashton and Luis
// Description: Simulates train movement with semaphores/mutexes, forks child processes
// we will work later down the road to merge this code with the rest of the groups

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TRAINS 10
#define MAX_INTERSECTIONS 10
#define MAX_NAME_LEN 32
#define MAX_ROUTE_LEN 10

typedef enum { MUTEX, SEMAPHORE } LockType;

typedef struct {
    char name[MAX_NAME_LEN];
    LockType type;
    int capacity;
    pthread_mutex_t mutex;
    sem_t semaphore;
    char holding_trains[MAX_TRAINS][MAX_NAME_LEN];
    int num_holding;
} Intersection;

typedef struct {
    char name[MAX_NAME_LEN];
    char route[MAX_ROUTE_LEN][MAX_NAME_LEN];
    int route_length;
} Train;

Intersection intersections[MAX_INTERSECTIONS];
int num_intersections = 0;

Train trains[MAX_TRAINS];
int num_trains = 0;

// File Loading 

void read_intersections(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Failed to open intersections.txt"); exit(1); }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (num_intersections >= MAX_INTERSECTIONS) break;

        char* token = strtok(line, ":");
        strncpy(intersections[num_intersections].name, token, MAX_NAME_LEN);
        intersections[num_intersections].name[strcspn(intersections[num_intersections].name, "\n")] = '\0';

        token = strtok(NULL, ":\n");
        intersections[num_intersections].capacity = atoi(token);
        intersections[num_intersections].type = (atoi(token) == 1) ? MUTEX : SEMAPHORE;
        num_intersections++;
    }

    fclose(file);
}

void read_trains(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Failed to open trains.txt"); exit(1); }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (num_trains >= MAX_TRAINS) break;

        char* name_token = strtok(line, ":");
        strncpy(trains[num_trains].name, name_token, MAX_NAME_LEN);
        trains[num_trains].name[strcspn(trains[num_trains].name, "\n")] = '\0';

        char* route_token = strtok(NULL, ":\n");
        int route_index = 0;

        char* inter_token = strtok(route_token, ",");
        while (inter_token && route_index < MAX_ROUTE_LEN) {
            strncpy(trains[num_trains].route[route_index], inter_token, MAX_NAME_LEN);
            trains[num_trains].route[route_index][strcspn(trains[num_trains].route[route_index], "\n")] = '\0';
            route_index++;
            inter_token = strtok(NULL, ",");
        }

        trains[num_trains].route_length = route_index;
        num_trains++;
    }

    fclose(file);
}

// Intersection Helpers

int find_intersection_index(const char* name) {
    for (int i = 0; i < num_intersections; i++) {
        if (strcmp(intersections[i].name, name) == 0) return i;
    }
    return -1;
}

void add_train_to_holding(Intersection* inter, const char* train_name) {
    strcpy(inter->holding_trains[inter->num_holding++], train_name);
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

// Simulation

void acquire_intersection(const char* train_name, const char* inter_name) {
    int idx = find_intersection_index(inter_name);
    if (idx == -1) return;

    Intersection* inter = &intersections[idx];
    printf("%s is waiting at %s.\n", train_name, inter->name);

    if (inter->type == MUTEX) pthread_mutex_lock(&inter->mutex);
    else sem_wait(&inter->semaphore);

    add_train_to_holding(inter, train_name);
    printf("%s is passing through %s.\n", train_name, inter->name);
    sleep(2);
}

void release_intersection(const char* train_name, const char* inter_name) {
    int idx = find_intersection_index(inter_name);
    if (idx == -1) return;

    Intersection* inter = &intersections[idx];
    if (inter->type == MUTEX) pthread_mutex_unlock(&inter->mutex);
    else sem_post(&inter->semaphore);

    remove_train_from_holding(inter, train_name);
    printf("%s has left %s.\n", train_name, inter->name);
}

void train_behavior(Train train) {
    for (int i = 0; i < train.route_length; i++) {
        acquire_intersection(train.name, train.route[i]);
        release_intersection(train.name, train.route[i]);
    }
    exit(0);
}

int main() {
    read_intersections("intersections.txt");
    read_trains("trains.txt");

    // Initialize semaphores/mutexes based on intersection type
    for (int i = 0; i < num_intersections; i++) {
        if (intersections[i].type == MUTEX)
            pthread_mutex_init(&intersections[i].mutex, NULL);
        else
            sem_init(&intersections[i].semaphore, 1, intersections[i].capacity);
    }

    // Fork a process for each train
    for (int i = 0; i < num_trains; i++) {
        pid_t pid = fork();
        if (pid == 0) train_behavior(trains[i]);  // Child
    }

    // Parent waits for all trains to complete
    for (int i = 0; i < num_trains; i++) {
        wait(NULL);
    }

    printf("Simulation complete. All trains reached their destinations.\n");
    return 0;
}



