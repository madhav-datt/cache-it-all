#ifndef MEMORY_BLOCK_H
#define MEMORY_BLOCK_H

#include <stdlib.h>

typedef struct memory_block
{
    void* start_addr;
    size_t size;
    void* data;
} memory_block;

memory_block* mb_new(void* start_addr, size_t size, void* source);

void mb_free(memory_block* mb);

#endif