//Author Name: Nolan Voss
//Email: nolan.voss@okstate.edu
//Date: 04/10/2025
//Program Description: Forking for the trains. The trains will read from trains.txt for their names, and fork in the parent process to create however many we need. 

#include <stdio.h>  // for printf
#include <stdlib.h> // for exit(1)
#include <unistd.h> // for fork
#include <string.h> //for string

int main() {
  //Total number of trains
  #define TRAINCOUNT 5
  //Max line length to check lines in train.txt
  #define MAXLINELEN 100

  char* train_name = "";
  
  char childNames[TRAINCOUNT][50];

  //Test variable
  //int totalProcesses = 0;
  
  FILE *file = fopen("train.txt", "r");
  
  //reads train.txt for the name of the trains
  for (int i = 0; i < TRAINCOUNT; i++){
    char line[MAXLINELEN];
    
    if (fgets(line, sizeof(line), file) != NULL){
      char *colon = strchr(line, ':'); //reads for first colon, separating the train name and its instructions
      
      if (colon != NULL) {
        size_t nameLen = colon - line;
        strncpy(childNames[i], line, nameLen);
        childNames[i][nameLen] = '\0'; //adding null terminator
      }
    } else {
        fprintf(stderr, "Not enough lines in trains.txt\n"); //Error in case we change train.txt to add more trains than TRAINCOUNT
        exit(1);
    }
  }
  
  fclose(file);
  
  int trainPID = fork(); 

  //Creates the forking process for the amount of trains planned
  for (int i = 0; i < TRAINCOUNT; i++){
    //totalProcesses ++;

    if (trainPID < 0) { // fork failed; exit
      fprintf(stderr, "fork failed\n");
      exit(1);
    } else if (trainPID == 0) { //child (new process)
      train_name = childNames[i];
      printf("TrainName: %s\n", train_name);
      //Once we merge this with the main code, this will run train_behavior function.

    } else { //parent goes down this path (main)
      int trainPID = fork(); 
    }
  }
  
  //printf("totalProcesses: %d", totalProcesses);

  return 0;
}
