//Group:    Group F
//Name:     Dylan Palmese
//Email:    dylan.palmese@okstate.edu
//Date:     4/17/2025

#include "rat_new.h"
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
#ifndef INITIALIZATION_H
#define INITIALIZATION_H



char** getTrains();
Intersection* getIntersections();

#endif
