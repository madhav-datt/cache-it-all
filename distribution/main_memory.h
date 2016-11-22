#ifndef MAIN_MEMORY_H
#define MAIN_MEMORY_H

#include "memory_block.h"

#define MAIN_MEMORY_SIZE 65536
#define MAIN_MEMORY_SIZE_LN 16
#define MAIN_MEMORY_START_ADDR 0x0000
#define MAIN_MEMORY_BLOCK_SIZE 32
#define MAIN_MEMORY_BLOCK_SIZE_LN 5
#define MAIN_MEMORY_INIT_FILE "mm_init.data"

typedef struct main_memory
{
    void* data;
    unsigned int w_queries;
    unsigned int r_queries;
} main_memory;

main_memory* mm_init();

void mm_write(main_memory* mm, void* start_addr, memory_block* mb);

memory_block* mm_read(main_memory* mm, void* start_addr);

void mm_free(main_memory* mm);

#endif