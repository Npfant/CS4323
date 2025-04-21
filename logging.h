//Group F
//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 04/06/2025
//Program Description: Simulation logging

#ifndef LOGGING_H
#define LOGGING_H

typedef struct {
    char type[10];            // e.g., REQ, GRNT_MUT, REL, etc.
    int trainNum1;            // Primary train ID
    int trainNum2;            // Secondary (for DEADLOCK)
    char intersecLetter[10];  // Intersections involved
    int intersecNum;          // Number of intersections released
    int semaphoreCnt;         // Used for GRNT_SEM
} Event;

pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER; // Declare sim timer mutex
int sim_time = 0;
FILE *fp;

void log_event(Event event) {
    float hour, minutes, sec;
    pthread_mutex_lock(&time_mutex); // Acquire lock
    if (strcmp(event.type, "REQ") == 0) sim_time++; // Event types of request and release increment time by 1, traversals by 2.
    else if (strcmp(event.type, "REL") == 0) sim_time++;
    else if (strcmp(event.type, "TRAV") == 0) sim_time += 2;

    hour = sim_time / 3600;
    minutes = ((int)sim_time % 3600) / 60; // Cast to int
    sec = (int)sim_time % 60; // Cast to int

    fp = fopen("sim.log", "a");
    if (fp == NULL) {
        perror("Failed to open sim.log");
        pthread_mutex_unlock(&time_mutex);
        return;
    }
        if(strcmp(event.type, "REQ") == 0){ //Request log statement
            printf("[%.2g:%.2g:%.2g] TRAIN%d: Sent AQUIRE request for Intersection%c\n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: Sent AQUIRE request for Intersection%c\n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
        }
        else if(strcmp(event.type, "GRNT_MUT") == 0){ //Mutex grant log statement
            printf("[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            printf("[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
        }
        else if(strcmp(event.type, "GRNT_SEM") == 0){ //Semaphore grant log statement
            printf("[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d. Semaphore count: %d.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1, event.semaphoreCnt);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d. Semaphore count: %d.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1, event.semaphoreCnt);
            printf("[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n", hour, minutes, sec, event.trainNum1, event.intersecLetter[0]);
        }
        else if(strcmp(event.type, "QUE") == 0){ //Queue log statement
            printf("[%.2g:%.2g:%.2g] SERVER: Intersection%c is full. Train%d added to wait queue.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Intersection%c is full. Train%d added to wait queue.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
        }
        else if(strcmp(event.type, "REL") == 0){ //Release log statement
            printf("[%.2g:%.2g:%.2g] TRAIN%d: RELEASED ", hour, minutes, sec, event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: RELEASED ", hour, minutes, sec, event.trainNum1);
            for (int i = 0; i < event.intersecNum - 1; i++) {
            printf("Intersection%c and ", event.intersecLetter[i]);
            fprintf(fp, "Intersection%c and ", event.intersecLetter[i]);
        }
            printf("Intersection%c.\n", event.intersecLetter[event.intersecNum - 1]);
            fprintf(fp, "Intersection%c.\n", event.intersecLetter[event.intersecNum - 1]);
        }    
        else if(strcmp(event.type, "DEAD") == 0){ //Deadlock log statement
            printf("[%.2g:%.2g:%.2g] SERVER: Deadlock detected! Cycle: Train%d <-> Train%d.\n", hour, minutes, sec, event.trainNum1, event.trainNum2);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Deadlock detected! Cycle: Train%d <-> Train%d.\n", hour, minutes, sec, event.trainNum1, event.trainNum2);
        }
        else if(strcmp(event.type, "PRE") == 0){ //Preemption log statement
            printf("[%.2g:%.2g:%.2g] SERVER: Preempting Intersection%c from Train%d.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Preempting Intersection%c from Train%d.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            printf("[%.2g:%.2g:%.2g] SERVER: Train%d released from Intersection%c forcibly.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Train%d released from Intersection%c forcibly.\n", hour, minutes, sec, event.intersecLetter[0], event.trainNum1);
        }
        else if(strcmp(event.type, "COM") == 0){ //Completion log statement
            printf("[%.2g:%.2g:%.2g] SIMULATION COMPLETE. All trains reached their destinations.\n", hour, minutes, sec);
            fprintf(fp, "[%.2g:%.2g:%.2g] SIMULATION COMPLETE. All trains reached their destinations.\n", hour, minutes, sec);
        }
        fclose(fp);
    pthread_mutex_unlock(&time_mutex); //Release lock
}

#endif
