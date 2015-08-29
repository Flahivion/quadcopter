/* 
 * File:   fifo.h
 * Author: Daniel
 *
 * Created on 31 December 2014, 3:58 PM
 */

#ifndef FIFO_H
#define	FIFO_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    void* data;
    void* next;
    unsigned int size;
    unsigned int count;
    unsigned int byte_size;
} fifo_t;

fifo_t fifo_create(unsigned int size, unsigned int byte_size);
void fifo_push(fifo_t* fifo, void* item);
void* fifo_get(fifo_t* fifo, unsigned int index);
void fifo_destroy(fifo_t* fifo);


#ifdef	__cplusplus
}
#endif

#endif	/* FIFO_H */

