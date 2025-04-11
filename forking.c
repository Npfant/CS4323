//Author Name: Nolan Voss
//Email: nolan.voss@okstate.edu
//Date: 04/10/2025
//Program Description: Forking for the trains

#include <stdio.h>  // for printf
#include <stdlib.h> // for exit(1)
#include <unistd.h> // for fork
#include <pthread.h> // for mutex

int main() {
  //Total number of trains
  #define CHILDCOUNT 5

  char* train_name = "";

  //Test variable
  //int totalProcesses = 0;
  
  const char* childNames[CHILDCOUNT] = {"Train1", "Train2", "Train3", "Train4", "Train5"};
  
  int trainPID = fork(); 

  //Creates the forking process for the amount of trains planned
  for (int i = 0; i < CHILDCOUNT; i++){
    totalProcesses ++;

    if (trainPID < 0) { // fork failed; exit
      fprintf(stderr, "fork failed\n");
      exit(1);
    } else if (trainPID == 0) { //child (new process)
      train_name = childNames[i];
      printf("TrainName: %s\n", train_name);

    } else { //parent goes down this path (main)
      int trainPID = fork(); 
    }
  }
  
  //printf("totalProcesses: %d", totalProcesses);

  return 0;
}
