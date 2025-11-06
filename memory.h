#ifndef MEMORY_H
#define MEMORY_H

int ReadFromMemory(uint8_t** input_buffer, long *file_size);
int WriteToMemory(uint8_t* output_buffer, long* position, uint8_t* ts_packet);

#endif