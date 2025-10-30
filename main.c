#include <stdio.h>
#include "demux.h"

int main() {
  printf("Software demux simulation for digital television receiver\n");

  Demux demux;
  demux = DemuxInit(DIRECT, DIRECT, ONE_SHOT);

  printf("Mod u kojem radi ovaj demux je %d\n", demux.input_mode);

  return 0;
}
