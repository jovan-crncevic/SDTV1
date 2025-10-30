#include <stdio.h>
#include <stdint.h>
#include "demux.h"

#define TS_PACKET_SIZE 188

Demux DemuxInit(IO_Mode i_m, IO_Mode o_m, Filter_Mode f_m) {
    Demux demux;
    demux.input_mode = i_m;
    demux.output_mode = o_m;
    demux.filter_mode = f_m;

    return demux;
}

int DemuxFilterTest() {
    FILE *file = fopen("ch0.ts", "rb");

    if (!file) {
        perror("Error: Can't open file");
        return 1;
    }

    uint8_t ts_packet[TS_PACKET_SIZE];
    int packet_count = 0;

    while (fread(ts_packet, 1, TS_PACKET_SIZE, file) == TS_PACKET_SIZE) {
        packet_count++;
        printf("TS packet %d:\n", packet_count);

        for (int i = 0; i < TS_PACKET_SIZE; i++) {
            printf("%02X ", ts_packet[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n- - - - - - - - - -\n");
    }

    fclose(file);
    return 0;
}
