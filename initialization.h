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


char** getTrains(){
  
  char** trains;
  trains = (char**)malloc(0 * sizeof(char*));
  FILE *trains_init = fopen("trains.txt","r");  //read trains file
  char nxtLine[100];
  int track = 0;
  while (fgets(nxtLine, 100, trains_init) != NULL){
    track++;
    trains = realloc(trains, (track * sizeof(char*)));
    trains[track] = malloc(100 * sizeof(char));
    strcpy(trains[track], nxtLine);       //read line from trains file directly into trains array
  }
  fclose(*trains_init);
  return trains;
}

Intersection* getIntersections(){

  Intersection intersections;
  intersections = (Intersection*)malloc(0 * sizeof(Intersection));
  FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
  char nxtLine[100];
  int track = 0;
  while (fgets(nxtLine, 100, intersections_init) != NULL){
    track++;
    intersections = realloc(intersections, (track * sizeof(Intersection)));
    char* interName = strtok(nxtLine, ":\n");    //copy name of intersection into interName
    char* tempCap = strtok(NULL, ":");     //copy intersection capacity into capacity
    int cap;
    sscanf(tempCap, "%d", &cap);
    if(cap > 1){                        //if capacity > 1: make locktype semaphore
      strcpy(intersections[track].name, interName);        //name
      intersections[track].type = SEMAPHORE;               //locktype
      intersections[track].capacity = cap;                 //capacity
    }else{                                         //else make locktype mutex
      strcpy(intersections[track].name, interName);        //name
      intersections[track].type = MUTEX;                   //locktype
      intersections[track].capacity = cap;                 //capacity
    }
  }
  fclose(*intersections_init);
  return intersections;
}
