#include <stdio.h>
#include <stdlib.h>
#include "demux.h"

#define NUMBER_OF_TESTS 8

int main(int argc, char** argv) {
  if (argc != 4) {
    printf("Error\n");
    return 1;
  }

  printf("Software demux simulation for digital television receiver\n");

  Demux demux;

  demux = DemuxInit(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  int pids[1] = { 18 };

  int status = DemuxFilter(demux, pids, 1);
  printf("%d\n", status);

  return 0;
}
