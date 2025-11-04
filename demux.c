#include <stdio.h>
#include <stdint.h>
#include "demux.h"

#define TS_PACKET_SIZE 188
#define SYNC_BYTE 0x47

Demux DemuxInit(IO_Mode i_m, IO_Mode o_m, Filter_Mode f_m) {
    Demux demux;
    demux.input_mode = i_m;
    demux.output_mode = o_m;
    demux.filter_mode = f_m;

    return demux;
}

int MatchCheck(const uint8_t* packet, int target_pid)
{
    if (!packet || packet[0] != SYNC_BYTE)
        return 0;

    int pid = ((packet[1] & 0x1F) << 8) | packet[2];
    return pid == target_pid;
}

static void DirectToDirect(int* pids, int pid_counter, int* ret) {
    FILE *input_file = fopen("tuner.ts", "rb");
    FILE *output_file = fopen("ram.txt", "w");

    if (!input_file || !output_file) {
        perror("Error: Can't open/create file");
        *ret = -1;
        return;
    }
    uint8_t ts_packet[TS_PACKET_SIZE];
    int packet_count = 0, matched = 0;

    while (fread(ts_packet, 1, TS_PACKET_SIZE, input_file) == TS_PACKET_SIZE)
    {
        packet_count++;

        for (int i = 0; i < pid_counter; i++)
        {
            if (MatchCheck(ts_packet, pids[i]))
            {
                matched++;
                fprintf(output_file, "\nMatched packet #%d (PID 0x%04X)\n", packet_count, pids[i]);

                for (int j = 0; j < TS_PACKET_SIZE; j++) {
                    fprintf(output_file, "%02X ", ts_packet[j]);
                    if ((j + 1) % 16 == 0) fprintf(output_file, "\n");
                }
            }
        }
    }
    if (matched == 0)
    {
        fprintf(output_file, "\nGiven pid was not found\n");
    }
    else
    {
        fprintf(output_file, "\nTotal packets: %d, matched : %d\n", packet_count, matched);
    }

    fclose(input_file);
    fclose(output_file);

    *ret = 1;
}

static void DirectToMemory(int* ret) {
    *ret = 2;
}

static void MemoryToDirect(int* ret) {
    *ret = 3;
}

static void MemoryToMemory(int* ret) {
    *ret = 4;
}

int DemuxFilter(Demux demux, int* pids, int pid_counter) {
    int ret = -1;

    if (demux.input_mode == 0 && demux.output_mode == 0) DirectToDirect(pids, pid_counter, &ret); 
    else if (demux.input_mode == 0 && demux.output_mode == 1) DirectToMemory(&ret); 
    else if (demux.input_mode == 1 && demux.output_mode == 0) MemoryToDirect(&ret); 
    else if (demux.input_mode == 1 && demux.output_mode == 1) MemoryToMemory(&ret);

    return ret;
}