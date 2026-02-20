#include "ring_buffer.h"

bool buffer_empty(struct ring_buffer *bf)
{
    return (bf->head == bf->tail);
}

bool buffer_full(struct ring_buffer *bf)
{
    return ((bf->head + 1) & BUFFER_MASK) == bf->tail;
}

uint8_t read_buffer(struct ring_buffer *bf)
{
    if (!buffer_empty(bf)) {
        uint8_t tmp = bf->buffer[bf->tail];
        bf->tail = (bf->tail + 1) & BUFFER_MASK;
        return tmp;
    }
    return 0;
}

void write_buffer(uint8_t data, struct ring_buffer *bf)
{
    if (!buffer_full(bf)) {
        bf->buffer[bf->head] = data;
        bf->head = (bf->head + 1) & BUFFER_MASK;
    }
}