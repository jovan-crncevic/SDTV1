#ifndef DEMUX_H
#define DEMUX_H

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

Demux DemuxInit(IO_Mode i_m, IO_Mode o_m, Filter_Mode f_m);

#endif