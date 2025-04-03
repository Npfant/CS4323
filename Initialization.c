#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  FILE *intersections_init = fopen("intersections.txt","r");
  FILE *trains_init = fopen("trains.txt","r");

  char intersections[5][100];
  char trains[5][100];

  for(int i = 0; i < 5; i++){
    char str1[100];
    char str2[100];
    fgets(str1,100,intersections_init);
    fgets(str2,100,trains_init);
    for(int j = 0; j < 100; j++){
      if(str1[j] == ':'){
        strncpy(intersections[i],str1+j,100-j);
        break;
      }
    }
    for(int j = 0; j < 100; j++){
      if(str2[j] == ':'){
        strncpy(trains[i],str1+j,100-j);
        break;
      }
    }
    printf("%s\t\t%s",intersections[i],trains[i]);
  }
}
