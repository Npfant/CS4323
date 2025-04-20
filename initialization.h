//Group:    Group F
//Name:     Dylan Palmese
//Email:    dylan.palmese@okstate.edu
//Date:     4/17/2025

#include "preemption.h"
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
#include <ctype.h>

int trainNum;
int interNum;

int countLines(FILE *filename){
    int currentLine = 1;
    char c;
    
    do{
      c = fgetc(filename);
      
      if (c == '\n'){
        currentLine++;
      }
      
    } while (c != EOF);
    //printf("%d\n",currentLine);
    return currentLine;
}

int howManyTrains(){
  return trainNum;
}
int howManyTrains(){
  return interNum;
}

char** getTrains(){
  
  char** trains;
  FILE *trains_init = fopen("trains.txt","r");  //read trains file
  trainNum = countLines(trains_init);
  rewind(trains_init);
  trains = (char**)malloc(NUM_TRAINS * sizeof(char*));
  char nxtLine[100];
  for(int i = 0; i < NUM_TRAINS; i++){
    fgets(nxtLine, 100, trains_init);
    trains[i] = malloc(100 * sizeof(char));
    strcpy(trains[i], nxtLine);       //read line from trains file directly into trains array
  }
  fclose(trains_init);
  //printf("-%d-\n", NUM_TRAINS);
  return trains;
}

Intersection* getIntersections(){

  Intersection* intersections;
  FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
  interNum = countLines(intersections_init);
  rewind(intersections_init);
  intersections = (Intersection*)malloc(NUM_INTERS * sizeof(Intersection));
  char nxtLine[100];
  for(int i = 0; i < NUM_INTERS; i++){
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
  fclose(intersections_init);
  return intersections;
}

