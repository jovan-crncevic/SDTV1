#ifndef DEMUX_H
#define DEMUX_H

#define TS_PACKET_SIZE 188
#define SYNC_BYTE 0x47

typedef enum {
    DIRECT = 0,
    MEMORY = 1
} IO_Mode;

typedef enum {
    CONTINUOUS = 0,
    ONE_SHOT = 1
} Filter_Mode;

typedef struct {
    IO_Mode input_mode;
    IO_Mode output_mode;
    Filter_Mode filter_mode;
} Demux;

typedef struct {
    Demux demux;
    int *pids;
    int pid_count;
    uint8_t **input_buffer;
    uint8_t **output_buffer;
    int* packets_found;
} DemuxThreadArgs;

Demux DemuxInit(IO_Mode i_m, IO_Mode o_m, Filter_Mode f_m);
void* DemuxThreadFunction(void* arg);

//Ako radimo bez niti: int DemuxFilter(Demux demux, int* pids, int pid_counter, uint8_t** input_buffer, uint8_t** output_buffer);

#endif