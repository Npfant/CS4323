//Author: Dylan Palmese


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

int main()
{
  FILE *intersections_init = fopen("intersections.txt","r"); //Read intersections file
  FILE *trains_init = fopen("trains.txt","r");               //Read trains file

  char intersections[5][100];  //Store intersection data in array of 5 lines
  char trains[5][100];         //Store train data in array of 5 lines

  for(int i = 0; i < 5; i++){ //Loop to read lines into arrays
    fgets(intersections[i],100,intersections_init);
    fgets(trains[0],100,trains_init);
    //printf("%s\t\t%s",intersections[i],trains[i]);
  }
  //Create locks
  pthread_mutex_t isA;  //Intersection A: can hold 1 train
  sem_t isB;         //Intersection B: can hold 3 trains
  pthread_mutex_t isC;  //Intersection C: can hold 1 train
  sem_t isD;         //Intersection D: can hold 2 trains
  pthread_mutex_t isE;  //Intersection E: can hold 1 train

  //Code to lock and unlock each intersection - to be used on server side later
  
  //To lock all:
  /*
  pthread_mutex_lock(&isA);
  for(int i = 0; i < 3; i++){
    sem_wait(&isB[i]);
  }
  pthread_mutex_lock(&isC);
  for(int i = 0; i < 2; i++){
    sem_wait(&isD[i]);
  }
  pthread_mutex_lock(&isE);
  */

  //To unlock all:
  /*
  pthread_mutex_unlock(&isA);
  for(int i = 0; i < 3; i++){
    sem_post(&isB[i]);
  }
  pthread_mutex_unlock(&isC);
  for(int i = 0; i < 2; i++){
    sem_post(&isD[i]);
  }
  pthread_mutex_unlock(&isE);
  */
}
