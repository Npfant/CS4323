//Author Name: Nolan Voss
//Email: nolan.voss@okstate.edu
//Date: 04/20/2025
//Program Description: Forking for the trains. The trains will read from an array for the train information, and fork in the parent process to create however many we need.
//Gives that information to main via a method that is defined in main.
//Also, wrote the code to count the amount of trains/intersections present in the .txt files.

#include <stdio.h>  //for printf
#include <stdlib.h> //for exit(1)
#include <unistd.h> //for fork
#include <string.h> //for string

#ifndef FORKING_H
#define FORKING_H

//Helper function taken from main. **FUNCTION IN MAIN NOT CREATED BY ME, BUT THIS NEEDS TO BE HERE TO RUN THIS HEADER**
extern void train_behavior(char* train_info, int req_id, int res_id, int* req, int* alloc);


int countLines(FILE *filename){
    int currentLine = 1;        //Counts the # of lines. The return value.
    char c;                     //Variable that stores every character read from the file.        
    
    do{
      c = fgetc(filename);      //Reads a character from the file
      
      if (c == '\n'){
        currentLine++;          //Once a newline character is read, increments the counter.
      }
      
    } while (c != EOF);         //Continues this process until it reads the end of the file. 
    
    return currentLine;         //Returns the count.
}

//Implemented in main.c
int forking(char** trains, int NUM_TRAINS, int req_id, int res_id, int* req, int* alloc) {
   
  //Creates the forking process for the amount of trains planned
  for (int i = 0; i < NUM_TRAINS; i++){
    pid_t pid = fork();                             //Forks a process every iteration.

    if (pid == 0) {                                         //Child (new process).
        train_behavior(trains[i], req_id, res_id, req, alloc);      //Runs train_behavior, incorporating these processes into main.
    }
    
    if (pid == -1) {                                        //Forking failed
        fprintf(stderr, "Forking failed\n");                        //Prints an error message
        exit(1);                                                    //Exits program
    }
  }
}

#endif
