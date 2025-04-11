// Group Project CS 4323 - Ashton and Luis (Phase 1 [work in progress])
// Description: Simulates train movement with semaphores/mutexes, forks child processes
// We are not fully done yet, but we have made great progress, and we will work later down the road to merge this code with the rest of the groups

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_TRAINS 4
#define MAX_NAME_LEN 32
#define MAX_HOLDING 10

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

Intersection intersections[] = {
    {"Intersection A", MUTEX, 1},
    {"Intersection B", SEMAPHORE, 2},
    {"Intersection C", MUTEX, 1},
    {"Intersection D", SEMAPHORE, 3},
    {"Intersection E", MUTEX, 1}
};

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

// This is the core logic (most key part) Ashton figured this out
void acquire_intersection(const char* train_name, const char* inter_name) {
    int idx = find_intersection_index(inter_name);
    if (idx == -1) {
        printf("ERROR: Unknown intersection %s\n", inter_name);
        return;
    }

    Intersection* inter = &intersections[idx];

    printf("%s is waiting at %s.\n", train_name, inter->name);

    if (inter->type == MUTEX) {
        pthread_mutex_lock(&inter->mutex);
    } else {
        sem_wait(&inter->semaphore);
    }

    add_train_to_holding(inter, train_name);
    printf("%s is passing through %s.\n", train_name, inter->name);
    sleep(2); // Simulates traversal time
}

void release_intersection(const char* train_name, const char* inter_name) {
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
}

// Train behavior
/*Luis figured out the path on where the trains need to go which he got from the Group Document.
    Ashton figured out the acquire and release for the intersections so it can run smoothly
*/
void train_behavior(const char* train_name) {
    if (strcmp(train_name, "Train1") == 0) {
        acquire_intersection(train_name, "Intersection A");
        release_intersection(train_name, "Intersection A");

        acquire_intersection(train_name, "Intersection B");
        release_intersection(train_name, "Intersection B");

        acquire_intersection(train_name, "Intersection C");
        release_intersection(train_name, "Intersection C");
    } else if (strcmp(train_name, "Train2") == 0) {
        acquire_intersection(train_name, "Intersection B");
        release_intersection(train_name, "Intersection B");

        acquire_intersection(train_name, "Intersection D");
        release_intersection(train_name, "Intersection D");

        acquire_intersection(train_name, "Intersection E");
        release_intersection(train_name, "Intersection E");
    } else if (strcmp(train_name, "Train3") == 0) {
        acquire_intersection(train_name, "Intersection C");
        release_intersection(train_name, "Intersection C");

        acquire_intersection(train_name, "Intersection D");
        release_intersection(train_name, "Intersection D");

        acquire_intersection(train_name, "Intersection A");
        release_intersection(train_name, "Intersection A");
    } else if (strcmp(train_name, "Train4") == 0) {
        acquire_intersection(train_name, "Intersection E");
        release_intersection(train_name, "Intersection E");

        acquire_intersection(train_name, "Intersection B");
        release_intersection(train_name, "Intersection B");

        acquire_intersection(train_name, "Intersection D");
        release_intersection(train_name, "Intersection D");
    }

    exit(0);
}

//from here we will include Nolan's code where the forking process will be implemented
int main() {
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
            train_behavior(trains[i]);
        }
    }

    // wait for all trains to finish
    for (int i = 0; i < NUM_TRAINS; i++) {
        wait(NULL);
    }

    printf("Simulation complete. All trains finished.\n");

    return 0;
}

