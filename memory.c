#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "demux.h"

int ReadFromMemory(uint8_t** input_buffer, long *file_size) {
    FILE *file = fopen("tuner.ts", "rb");
    if (!file) return -2;

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *input_buffer = malloc(*file_size);
    if (!(*input_buffer)) {
        fclose(file);
        return -3;
    }

    if (fread(*input_buffer, 1, *file_size, file) != *file_size) {
        free(*input_buffer);
        fclose(file);
        return -4;
    }

    fclose(file);
    return 1;
}

int WriteToMemory(uint8_t* output_buffer, long* position, uint8_t* ts_packet) {
    memcpy(&output_buffer[*position], ts_packet, TS_PACKET_SIZE);
    *position += TS_PACKET_SIZE;
    return 1;
}