//Author Name: Nathan Fant
//Email: nathan.fant@okstate.edu
//Date: 04/13/2025
//Program Description: Implementation of a resource allocation table

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

int height = 4;
int width = 4;

int alloc[height][width] = {{1,0,0,0},{0,0,1,0},{0,1,0,0},{0,0,0,1}};
int req[height][width] = {{0,0,1,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}};
int init[width] = {1,1,1,1};

void rat(){
    int avail[width];
    for(int i = 0; i < width; i++){
        avail[i] = init[i];
        for(int j = 0; j < height; j++){
            if(alloc[j][i] > 0){
                avail[i] -= alloc[i][j]; //Decrements available from max to not currently allocated.
            }
        }
        printf("%d ",avail[i]);
    }
    printf("\n");
    bool cycle = 1; //Start with an assumed cycle

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if((avail[j] - req[i][j]) < 0){
                break; //Stops checking request if it is larger than available.
            }
            if(j == (width - 1)){
                cycle = 0; //Sets cycle to false if it fully iterates through a line.
            }
        }
    }
    if(cycle == 1){
        printf("DEADLOCK!\n");
    }
    else{
        printf("NO DEADLOCK\n");
    }
}