////Author Name: Tony Lopez
//Email: gerardo.a.lopez@okstate.edu
//Date: 04/20/2025
//Description:  This header defines the `Event` structure and the `log_event()` function 
//used for logging key events during the multi-train intersection simulation.

#ifndef LOGGER_H
#define LOGGER_H

typedef struct {
    char type[10];            // e.g., REQ, GRNT_MUT, REL, etc.
    int trainNum1;            // Primary train ID
    int trainNum2;            // Secondary (for DEADLOCK)
    char intersecLetter[32];  // Intersections involved
    int intersecNum;          // Number of intersections released
    int semaphoreCnt;         // Used for GRNT_SEM
} Event;

void log_event(Event evt);

#endif
