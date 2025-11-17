#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "demux.h"
#include "memory.h"

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
static int DirectToDirect(int* pids, int pid_counter, int is_one_shot) {
    FILE *input_file = fopen("tuner.ts", "rb");
    FILE *output_file = fopen("ram.ts", "wb");
    if (!input_file) return -2;
    if (!output_file) {
        fclose(input_file);
        return -2;
    }

    uint8_t ts_packet[TS_PACKET_SIZE];
    int matched = 0, stop = 0, collecting = 0, bytes_collected = 0, expected_length = -1;

    while (fread(ts_packet, 1, TS_PACKET_SIZE, input_file) == TS_PACKET_SIZE && !stop) {
        for (int i = 0; i < pid_counter; i++) {
            if (MatchCheck(ts_packet, pids[i])) {
                matched++;

                fwrite(ts_packet, 1, TS_PACKET_SIZE, output_file);

                if (is_one_shot) {
                    int adaptation_length = 0;
                    if (ts_packet[3] & 0x20) {
                        adaptation_length = 1 + ts_packet[4];
                    }

                    int payload_offset = 4 + adaptation_length;
                    if (payload_offset >= TS_PACKET_SIZE)
                        break;

                    if (ts_packet[1] & 0x40) {
                        uint8_t pointer = ts_packet[payload_offset];
                        payload_offset += 1 + pointer;

                        collecting = 1;
                        bytes_collected = 0;
                        expected_length = -1;
                    }

                    if (!collecting)
                        break;

                    int payload_size = TS_PACKET_SIZE - payload_offset;
                    if (payload_size <= 0)
                        break;

                    uint8_t *payload = ts_packet + payload_offset;

                    if (expected_length < 0 && payload_size >= 3) {
                        int section_length = ((payload[1] & 0x0F) << 8) | payload[2];
                        expected_length = 3 + section_length;
                        printf("DEBUG: Expected lenght is %d\n", expected_length);
                    }

                    bytes_collected += payload_size;

                    if (expected_length > 0 && bytes_collected >= expected_length) {
                        stop = 1;
                        break;
                    }
                }
            }
        }
    }

    fclose(input_file);
    fclose(output_file);

    return matched;
}

/* DIREKTNI REZIM NA ULAZU I MEMORIJSKI REZIM NA IZLAZU */
static int DirectToMemory(int* pids, int pid_counter, int is_one_shot, uint8_t** output_buffer) {
    FILE *input_file = fopen("tuner.ts", "rb");
    if (!input_file) return -2;

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file), position = 0;
    fseek(input_file, 0, SEEK_SET);

    *output_buffer = malloc(file_size);
    if (!(*output_buffer)) {
        fclose(input_file);
        return -2;
    }

    uint8_t ts_packet[TS_PACKET_SIZE];
    int matched = 0, stop = 0, collecting = 0, bytes_collected = 0, expected_length = -1;

    while (fread(ts_packet, 1, TS_PACKET_SIZE, input_file) == TS_PACKET_SIZE && !stop) {
        for (int i = 0; i < pid_counter; i++) {
            if (MatchCheck(ts_packet, pids[i])) {
                matched++;

                WriteToMemory(*output_buffer, &position, ts_packet);

                if (is_one_shot) {
                    int adaptation_length = 0;
                    if (ts_packet[3] & 0x20) {
                        adaptation_length = 1 + ts_packet[4];
                    }

                    int payload_offset = 4 + adaptation_length;
                    if (payload_offset >= TS_PACKET_SIZE)
                        break;

                    if (ts_packet[1] & 0x40) {
                        uint8_t pointer = ts_packet[payload_offset];
                        payload_offset += 1 + pointer;

                        collecting = 1;
                        bytes_collected = 0;
                        expected_length = -1;
                    }

                    if (!collecting)
                        break;

                    int payload_size = TS_PACKET_SIZE - payload_offset;
                    if (payload_size <= 0)
                        break;

                    uint8_t *payload = ts_packet + payload_offset;

                    if (expected_length < 0 && payload_size >= 3) {
                        int section_length = ((payload[1] & 0x0F) << 8) | payload[2];
                        expected_length = 3 + section_length;
                        printf("DEBUG: Expected lenght is %d\n", expected_length);
                    }

                    bytes_collected += payload_size;

                    if (expected_length > 0 && bytes_collected >= expected_length) {
                        stop = 1;
                        break;
                    }
                }
            }
        }
    }

    fclose(input_file);

    return matched;
}

/* MEMORIJSKI REZIM NA ULAZU I DIREKTNI REZIM NA IZLAZU */
static int MemoryToDirect(int* pids, int pid_counter, int is_one_shot, uint8_t** input_buffer) {
    long file_size;
    uint8_t ts_packet[TS_PACKET_SIZE];
    int matched = 0, stop = 0, collecting = 0, bytes_collected = 0, expected_length = -1;

    int ret = ReadFromMemory(input_buffer, &file_size);
    if (ret != 1) return ret;

    FILE *output_file = fopen("ram.ts", "wb");
    if (!output_file) {
        return -2;
    }

    for (long i = 0; i < file_size; i += TS_PACKET_SIZE) {
        if (stop) break;

        memcpy(ts_packet, &(*input_buffer)[i], TS_PACKET_SIZE);

        for (int k = 0; k < pid_counter; k++) {
            if (MatchCheck(ts_packet, pids[k])) {
                matched++;

                fwrite(ts_packet, 1, TS_PACKET_SIZE, output_file);

                if (is_one_shot) {
                    int adaptation_length = 0;
                    if (ts_packet[3] & 0x20) {
                        adaptation_length = 1 + ts_packet[4];
                    }

                    int payload_offset = 4 + adaptation_length;
                    if (payload_offset >= TS_PACKET_SIZE)
                        break;

                    if (ts_packet[1] & 0x40) {
                        uint8_t pointer = ts_packet[payload_offset];
                        payload_offset += 1 + pointer;

                        collecting = 1;
                        bytes_collected = 0;
                        expected_length = -1;
                    }

                    if (!collecting)
                        break;

                    int payload_size = TS_PACKET_SIZE - payload_offset;
                    if (payload_size <= 0)
                        break;

                    uint8_t *payload = ts_packet + payload_offset;

                    if (expected_length < 0 && payload_size >= 3) {
                        int section_length = ((payload[1] & 0x0F) << 8) | payload[2];
                        expected_length = 3 + section_length;
                        printf("DEBUG: Expected lenght is %d\n", expected_length);
                    }

                    bytes_collected += payload_size;

                    if (expected_length > 0 && bytes_collected >= expected_length) {
                        stop = 1;
                        break;
                    }
                }
            }
        }
    }

    fclose(output_file);

    return matched;
}

/* MEMORIJSKI REZIM NA ULAZU I MEMORIJSKI REZIM NA IZLAZU */
static int MemoryToMemory(int* pids, int pid_counter, int is_one_shot, uint8_t** input_buffer, uint8_t** output_buffer) {
    long file_size, position = 0;
    uint8_t ts_packet[TS_PACKET_SIZE];
    int matched = 0, stop = 0, collecting = 0, bytes_collected = 0, expected_length = -1;

    int ret = ReadFromMemory(input_buffer, &file_size);
    if (ret != 1) return ret;

    *output_buffer = malloc(file_size);
    if (!(*output_buffer)) {
        return -2;
    }

    for (long i = 0; i < file_size; i += TS_PACKET_SIZE) {
        if (stop) break;

        memcpy(ts_packet, &(*input_buffer)[i], TS_PACKET_SIZE);

        for (int k = 0; k < pid_counter; k++) {
            if (MatchCheck(ts_packet, pids[k])) {
                matched++;

                WriteToMemory(*output_buffer, &position, ts_packet);

                if (is_one_shot) {
                    int adaptation_length = 0;
                    if (ts_packet[3] & 0x20) {
                        adaptation_length = 1 + ts_packet[4];
                    }

                    int payload_offset = 4 + adaptation_length;
                    if (payload_offset >= TS_PACKET_SIZE)
                        break;

                    if (ts_packet[1] & 0x40) {
                        uint8_t pointer = ts_packet[payload_offset];
                        payload_offset += 1 + pointer;
                        printf("DEBUG: Payload offset is %d\n", payload_offset);

                        collecting = 1;
                        bytes_collected = 0;
                        expected_length = -1;
                    }

                    if (!collecting)
                        break;

                    int payload_size = TS_PACKET_SIZE - payload_offset;
                    if (payload_size <= 0)
                        break;

                    uint8_t *payload = ts_packet + payload_offset;

                    if (expected_length < 0 && payload_size >= 3) {
                        int section_length = ((payload[1] & 0x0F) << 8) | payload[2];
                        expected_length = 3 + section_length;
                        printf("DEBUG: Expected lenght is %d\n", expected_length);
                    }

                    bytes_collected += payload_size;

                    if (expected_length > 0 && bytes_collected >= expected_length) {
                        stop = 1;
                        break;
                    }
                }
            }
        }
    }

    return matched;
}

/* GLOBALNA FUNKCIJA ZA FILTRIRANJE */
static int DemuxFilter(Demux demux, int* pids, int pid_counter, uint8_t** input_buffer, uint8_t** output_buffer) {
    int ret = -1;

    if (demux.input_mode == 0 && demux.output_mode == 0) ret = DirectToDirect(pids, pid_counter, demux.filter_mode);
    else if (demux.input_mode == 0 && demux.output_mode == 1) ret = DirectToMemory(pids, pid_counter, demux.filter_mode, output_buffer); 
    else if (demux.input_mode == 1 && demux.output_mode == 0) ret = MemoryToDirect(pids, pid_counter, demux.filter_mode, input_buffer); 
    else if (demux.input_mode == 1 && demux.output_mode == 1) ret = MemoryToMemory(pids, pid_counter, demux.filter_mode, input_buffer, output_buffer);

    return ret;
}

void* DemuxThreadFunction(void* arg) {
    DemuxThreadArgs *args = (DemuxThreadArgs*)arg;

    *(args->packets_found) = DemuxFilter(
        args->demux,
        args->pids,
        args->pid_count,
        args->input_buffer,
        args->output_buffer
    );

    switch(*(args->packets_found)) {
        case -4:
            printf("Error: Can't load file to buffer\n");
            break;

        case -3:
            printf("Error: Can't allocate memory\n");
            break;

        case -2:
            printf("Error: Can't open file\n");
            break;

        case -1:
            printf("Error: Demux initialization is wrong\n");
            break;

        case 0:
            printf("Error: No packets with entered PID(s) found\n");
            break;

        default:
            printf("Demux finished. Total packets found: %d\n", *(args->packets_found));
    }

    return NULL;
}