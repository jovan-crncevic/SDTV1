#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include "demux.h"

int main(int argc, char** argv) {
  /* PROVERA ISPRAVNOSTI POZIVA PROGRAMA */
  if (argc < 6) {
    printf("Error! Usage: ./a.exe <input_mode> <output_mode> <filter_mode> <number of pids> <pids...>\n");
    return 1;
  }
  else if (atoi(argv[4]) != argc - 5) {
    printf("Error! Number of PIDs must match\n");
    return 1;
  }

  /* DEKLARACIJA I INICIJALIZACIJA DEMUXA */
  Demux demux;
  demux = DemuxInit(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  /* PRIPREMA ARGUMENATA FUNKCIJE ZA FILTRIRANJE */
  int pid_count = atoi(argv[4]);
  int pids[pid_count];
  for (int i = 0; i < pid_count; i++) pids[i] = atoi(argv[5 + i]);
  uint8_t *input_buffer = NULL, *output_buffer = NULL;
  int packets_found;

  /* POZIV THREAD FUNKCIJE ZA FILTRIRANJE I TESTIRANJE ASINHRONOSTI */
  DemuxThreadArgs args = {
      .demux = demux,
      .pids = pids,
      .pid_count = pid_count,
      .input_buffer = &input_buffer,
      .output_buffer = &output_buffer,
      .packets_found = &packets_found
  };
  
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, DemuxThreadFunction, &args);
  pthread_detach(thread_id);

  printf("DemuxFilter is working in background, I'm sleeping now\n");
  sleep(5);
  printf("Sleeping is done\n");

  /* PROVERA POSTOJANJA IZLAZNOG BAFERA I POTENCIJALNI ISPIS */
  if (output_buffer) {
    for (long i = 0; i < packets_found; i++) {
      printf("Packet #%ld:\n", i + 1);

      for (long j = 0; j < TS_PACKET_SIZE; j++) {
          printf("%02X ", output_buffer[i * TS_PACKET_SIZE + j]);
          if ((j + 1) % 16 == 0) printf("\n");
      }

      if (TS_PACKET_SIZE % 16 != 0) printf("\n");
      printf("\n");
    }
  }

  /* OSLOBADJANJE ZAUZETE MEMORIJE */
  free(input_buffer);
  free(output_buffer);

  return 0;
}
