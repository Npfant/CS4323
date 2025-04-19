#include "rat_new.h"

void add_train_to_holding(Intersection* inter, const char* train_name) {    //changed by me
    if (inter->num_holding < MAX_HOLDING) {        
       strncpy(inter->holding_trains[inter->num_holding], train_name, MAX_NAME_LEN - 1);
       inter->holding_trains[inter->num_holding][MAX_NAME_LEN - 1] = '\0';  // Ensure null termination
       inter->num_holding++;
   } else {
       printf("ERROR: Holding capacity reached at %s\n", inter->name);
   }
}

void remove_train_from_holding(Intersection* inter, const char* train_name) {
   for (int i = 0; i < inter->num_holding; i++) {
       if (strcmp(inter->holding_trains[i], train_name) == 0) {
           for (int j = i; j < inter->num_holding - 1; j++) {
               strcpy(inter->holding_trains[j], inter->holding_trains[j + 1]);
           }
           inter->num_holding--;
           break;
       }
   }
}