#include <stdio.h>
#include <stdlib.h>

int main()
{
  FILE *intersections_init = fopen("intersections.txt","r");
  FILE *trains_init = fopen("trains.txt","r");

  char intersections[5][100];
  char trains[5][100];

  for(int i = 0; i < 5; i++){
    fgets(intersections[i],100,intersections_init);
    fgets(trains[i],100,trains_init);
    printf("%s\t\t%s",intersections[i],trains[i]);
  }
}
