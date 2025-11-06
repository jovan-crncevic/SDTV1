#include <stdio.h>
#include <stdlib.h>
#include "demux.h"

int main(int argc, char** argv) {
  if (argc < 6) {
    printf("Error! Usage: ./a.exe <input_mode> <output_mode> <filter_mode> <number of pids> <pids...>\n");
    return 1;
  }
  else if (atoi(argv[4]) != argc - 5) {
    printf("Error! Number of PIDs must match\n");
    return 1;
  }

  printf("SOFTWARE DEMUX SIMULATION\n");

  Demux demux;

  demux = DemuxInit(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  int pid_count = atoi(argv[4]);
  int pids[pid_count];
  for (int i = 0; i < pid_count; i++) pids[i] = atoi(argv[5 + i]);

  printf("Broj pronadjenih paketa: %d\n", DemuxFilter(demux, pids, pid_count));

  return 0;
}
