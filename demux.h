#ifndef DEMUX_H
#define DEMUX_H

typedef struct {
    int a;
    int b;
} Demux;

Demux DemuxInit(int a, int b);

#endif