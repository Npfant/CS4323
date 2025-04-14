#ifndef LOGGER_H
#define LOGGER_H

typedef struct {
    char type[10];            // e.g., REQ, GRNT_MUT, REL, etc.
    int trainNum1;            // Primary train ID
    int trainNum2;            // Secondary (for DEADLOCK)
    char intersecLetter[10];  // Intersections involved
    int intersecNum;          // Number of intersections released
    int semaphoreCnt;         // Used for GRNT_SEM
} Event;

void log_event(Event evt);

#endif
