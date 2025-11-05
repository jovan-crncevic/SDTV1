#include <stdio.h>
#include <stdint.h>
#include "demux.h"

#define TS_PACKET_SIZE 188
#define SYNC_BYTE 0x47

/* INICIJALIZACIJA DEMULTIPLEKSERA */
Demux DemuxInit(IO_Mode i_m, IO_Mode o_m, Filter_Mode f_m) {
    Demux demux;
    demux.input_mode = i_m;
    demux.output_mode = o_m;
    demux.filter_mode = f_m;

    return demux;
}

/* PROVERAVANJE PODUDARANJA PAKETA I PID- A */
static int MatchCheck(const uint8_t* packet, int target_pid) {
    if (!packet || packet[0] != SYNC_BYTE) return 0;

    int pid = ((packet[1] & 0x1F) << 8) | packet[2];
    return pid == target_pid;
}

/* DIREKTNI REZIM NA ULAZU I DIREKTNI REZIM NA IZLAZU */
static int DirectToDirect(int* pids, int pid_counter) {
    FILE *input_file = fopen("tuner.ts", "rb");
    FILE *output_file = fopen("ram.txt", "w");

    if (!input_file || !output_file) return -2;

    uint8_t ts_packet[TS_PACKET_SIZE];
    int packet_count = 0, matched = 0;

    while (fread(ts_packet, 1, TS_PACKET_SIZE, input_file) == TS_PACKET_SIZE) {
        packet_count++;

        for (int i = 0; i < pid_counter; i++) {
            if (MatchCheck(ts_packet, pids[i])) {
                matched++;
                fprintf(output_file, "\nMatched packet #%d (PID 0x%04X)\n", packet_count, pids[i]);

                for (int j = 0; j < TS_PACKET_SIZE; j++) {
                    fprintf(output_file, "%02X ", ts_packet[j]);
                    if ((j + 1) % 16 == 0) fprintf(output_file, "\n");
                }
            }
        }
    }

    fclose(input_file);
    fclose(output_file);

    if (!matched) return 0;
    return 1;
}

/* DIREKTNI REZIM NA ULAZU I MEMORIJSKI REZIM NA IZLAZU */
static int DirectToMemory(int* pids, int pid_counter) {
    return 1;
}

/* MEMORIJSKI REZIM NA ULAZU I DIREKTNI REZIM NA IZLAZU */
static int MemoryToDirect(int* pids, int pid_counter) {
    return 1;
}

/* MEMORIJSKI REZIM NA ULAZU I MEMORIJSKI REZIM NA IZLAZU */
static int MemoryToMemory(int* pids, int pid_counter) {
    return 1;
}

/* GLOBALNA FUNKCIJA ZA FILTRIRANJE */
int DemuxFilter(Demux demux, int* pids, int pid_counter) {
    int ret = -1;

    if (demux.input_mode == 0 && demux.output_mode == 0) ret = DirectToDirect(pids, pid_counter);
    else if (demux.input_mode == 0 && demux.output_mode == 1) ret = DirectToMemory(pids, pid_counter); 
    else if (demux.input_mode == 1 && demux.output_mode == 0) ret = MemoryToDirect(pids, pid_counter); 
    else if (demux.input_mode == 1 && demux.output_mode == 1) ret = MemoryToMemory(pids, pid_counter);

    return ret;
}