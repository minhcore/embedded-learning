#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 32
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct ring_buffer
{
    uint8_t head;
    uint8_t tail;
    uint8_t *buffer;
};

bool buffer_empty(struct ring_buffer *bf);
bool buffer_full(struct ring_buffer *bf);
uint8_t read_buffer(struct ring_buffer *bf);
void write_buffer(uint8_t data, struct ring_buffer *bf);

#endif