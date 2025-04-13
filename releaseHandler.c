// Group Project CS 4323 - Ashton and Luis
// Description: Simulates train movement with semaphores/mutexes, forks child processes
// we will work later down the road to merge this code with the rest of the groups

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRAINS 10                  // Max number of trains allowed
#define MAX_INTERSECTIONS 10          // Max number of intersections
#define MAX_NAME_LEN 32               // Max length for names
#define MAX_ROUTE_LEN 10              // Max intersections per train route

// Enum to represent the type of lock used by an intersection
typedef enum { MUTEX, SEMAPHORE } LockType;

// Structure to hold intersection info loaded from intersections.txt
typedef struct {
    char name[MAX_NAME_LEN];          
    LockType type;                    
    int capacity;                     
} Intersection;

// Structure to hold a train’s name and its route (list of intersections)
typedef struct {
    char name[MAX_NAME_LEN];                  
    char route[MAX_ROUTE_LEN][MAX_NAME_LEN];  
    int route_length;                         
} Train;

Intersection intersections[MAX_INTERSECTIONS];
int num_intersections = 0;

Train trains[MAX_TRAINS];
int num_trains = 0;


/*
  Reads intersection data from intersections.txt.
  Populates intersections[] array dynamically.
 */
void read_intersections(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open intersections.txt");
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        // First part: intersection name
        char* token = strtok(line, ":");
        if (token == NULL) continue;
        strncpy(intersections[num_intersections].name, token, MAX_NAME_LEN);

        // Second part: capacity
        token = strtok(NULL, ":\n");
        if (token == NULL) continue;
        intersections[num_intersections].capacity = atoi(token);

        // Determine lock type
        intersections[num_intersections].type = (atoi(token) == 1) ? MUTEX : SEMAPHORE;

        num_intersections++;  // Move to next slot in array
    }

    fclose(file);
}

/*
 Reads train route data from trains.txt.
 Populates trains[] array dynamically
 */
void read_trains(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open trains.txt");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Get train name (before colon)
        char* name_token = strtok(line, ":");
        if (name_token == NULL) continue;
        strncpy(trains[num_trains].name, name_token, MAX_NAME_LEN);

        // Get route string 
        char* route_token = strtok(NULL, ":\n");
        int route_index = 0;

        // Split route by commas and store each intersection name
        char* inter_token = strtok(route_token, ",");
        while (inter_token != NULL && route_index < MAX_ROUTE_LEN) {
            strncpy(trains[num_trains].route[route_index++], inter_token, MAX_NAME_LEN);
            inter_token = strtok(NULL, ",");
        }

        trains[num_trains].route_length = route_index;  // Total intersections in this train's route
        num_trains++;  // Move to the next train slot
    }

    fclose(file);
}


