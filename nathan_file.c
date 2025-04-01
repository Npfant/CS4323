//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 03/31/2025
//Program Description: Individual code for group project

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER; //Declare sim timer mutex
int sim_time = 0;
FILE *fp;

void time(event){
    int hour; int minute; int sec;
    pthread_mutex_lock(&time_mutex); //Acquire lock
        if(event.type == REQ) sim_time++;
        else if(event.type == REL) sim_time++;
        else if (event.type == TRAV) sim_time += 2;
        else sim_time = sim_time;
        hour = sim_time / 3600;
        minutes = (sim_time % 3600) / 60;
        sec = sim_time % 60;
        fp = fopen("log.txt", "a");
        if(event.type == REQ){
            printf("[%.2g:%.2g:%.2g] TRAIN%d: Sent AQUIRE request for Intersection%c\n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: Sent AQUIRE request for Intersection%c\n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
        }
        else if(event.type == GRNT_MUT){
            printf("[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            printf("[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
        }
        else if(event.type == GRNT_SEM){
            printf("[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d. Semaphore count: %d.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1, event.semaphoreCnt);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: GRANTED Intersection%c to Train%d. Semaphore count: %d.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1, event.semaphoreCnt);
            printf("[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: AQUIRED Intersection%c. Proceeding... \n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
        }
        else if(event.type == QUE){
            printf("[%.2g:%.2g:%.2g] SERVER: Intersection%c is full. Train%d added to wait queue.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Intersection%c is full. Train%d added to wait queue.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
        }
        else if(event.type == REL){
            printf("[%.2g:%.2g:%.2g] TRAIN%d: RELEASED Intersection%c.\n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
            fprintf(fp, "[%.2g:%.2g:%.2g] TRAIN%d: RELEASED Intersection%c.\n" hour, minutes, sec, event.trainNum1, event.intersecLetter);
        }
        else if(event.type == DEAD){
            printf("[%.2g:%.2g:%.2g] SERVER: Deadlock detected! Cycle: Train%d <-> Train%d.\n" hour, minutes, sec, event.trainNum1, event.trainNum2);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Deadlock detected! Cycle: Train%d <-> Train%d.\n" hour, minutes, sec, event.trainNum1, event.trainNum2);
        }
        else if(event.type == PRE){
            printf("[%.2g:%.2g:%.2g] SERVER: Preempting Intersection%c from Train%d.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Preempting Intersection%c from Train%d.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            printf("[%.2g:%.2g:%.2g] SERVER: Train%d released from Intersection%c forcibly.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
            fprintf(fp, "[%.2g:%.2g:%.2g] SERVER: Train%d released from Intersection%c forcibly.\n" hour, minutes, sec, event.intersecLetter, event.trainNum1);
        }
        else if(event.type == COM){
            printf("[%.2g:%.2g:%.2g] SIMULATION COMPLETE. All trains reached their destinations.\n" hour, minutes, sec);
            fprintf(fp, "[%.2g:%.2g:%.2g] SIMULATION COMPLETE. All trains reached their destinations.\n" hour, minutes, sec);
        }
        fclose(fp);
    pthread_mutex_unlock(&time_mutex); //Release lock
}

int main() {
    pthread_t threads[];
    // Get student ID and name from user
    printf("Enter student ID: ");
    fgets(student.ID, sizeof(student.ID), stdin);
    printf("Enter student name: ");
    fgets(student.name, sizeof(student.name), stdin);
    // Remainder of the code..
    pthread_create(&threads[i], NULL, threadGen, NULL);            
    pthread_join(threads[i], NULL);
    return EXIT_SUCCESS;
}