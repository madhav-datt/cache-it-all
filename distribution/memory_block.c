#include <stdlib.h>
#include <string.h>

#include "memory_block.h"

memory_block* mb_new(void* start_addr, size_t size, void* source)
{
    memory_block* result = malloc(sizeof(memory_block));
    result->start_addr = start_addr;
    result->size = size;
    result->data = malloc(size);
    memcpy(result->data, source, size);
    return result;
}

void mb_free(memory_block* mb)
{
    free(mb->data);
    free(mb);
}