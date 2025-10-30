#include <stdio.h>
#include "demux.h"

int main() {
  printf("Software demux simulation for digital television receiver\n");

  Demux demux;
  demux = DemuxInit(DIRECT, DIRECT, ONE_SHOT);

  DemuxFilterTest();

  return 0;
}
