#include <stdlib.h>
#include <string.h>
#include "fifo.h"

fifo_t fifo_create(unsigned int size, unsigned int byte_size)
{
    fifo_t fifo;

    fifo.size = size;
    fifo.byte_size = byte_size;
    fifo.count = 0;
    fifo.data = malloc(size * byte_size);
    fifo.next = fifo.data;

    return fifo;
}

void fifo_push(fifo_t* fifo, void* item)
{
    if (fifo->count < fifo->size)
        fifo->count++;

    memcpy(fifo->next, item, fifo->byte_size);

    if ((char*)fifo->next + fifo->byte_size >= (char*)fifo->data + (fifo->size * fifo->byte_size))
        fifo->next = fifo->data;
    else
        fifo->next = (char*)fifo->next + fifo->byte_size;
}

void* fifo_get(fifo_t* fifo, unsigned int index)
{
    void* cur;

    cur = (char*)fifo->next - fifo->byte_size - (fifo->byte_size * index);
    if ((char*)cur < (char*)fifo->data)
        cur = (char*)cur + (fifo->size * fifo->byte_size);

    return cur;
}

void fifo_destroy(fifo_t* fifo)
{
    free(fifo->data);
    fifo->data = NULL;
}
