#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "main_memory.h"

main_memory* mm_init()
{
    FILE* input_file = fopen(MAIN_MEMORY_INIT_FILE, "r");
    if (input_file == 0)
    {
        fprintf(stderr, "Error: Could not open %s."
                        " Ensure that file is in the proper directory.\n\n",
                        MAIN_MEMORY_INIT_FILE);
        exit(1);
    }

    main_memory* result = malloc(sizeof(main_memory));

    result->data = malloc(MAIN_MEMORY_SIZE);
    if (fread(result->data, MAIN_MEMORY_SIZE, 1, input_file) != 1)
    {
        fprintf(stderr, "Error: Not enough data in %s.\n\n",
                MAIN_MEMORY_INIT_FILE);
        exit(2);
    }
    
    fclose(input_file);
    
    result->w_queries = 0;
    result->r_queries = 0;

    return result;
}

void mm_write(main_memory* mm, void* start_addr, memory_block* mb)
{
    // start_addr argument must match mb argument's start_addr field
    assert(start_addr == mb->start_addr);
    
    // the block we ask to write must be aligned to a MAIN_MEMORY block
    assert((size_t) (start_addr - MAIN_MEMORY_START_ADDR)
           % MAIN_MEMORY_BLOCK_SIZE == 0);
    
    // the block we ask to write must have size MAIN_MEMORY_BLOCK_SIZE
    assert(mb->size == MAIN_MEMORY_BLOCK_SIZE);
    
    // make sure we are not out of bounds
    assert(start_addr >= MAIN_MEMORY_START_ADDR);
    assert(start_addr + mb->size
           <= (void*) MAIN_MEMORY_START_ADDR + MAIN_MEMORY_SIZE);
    
    memcpy(mm->data + (size_t) start_addr - MAIN_MEMORY_START_ADDR, mb->data, mb->size);
    
    printf("MM: Wrote %zu bytes at %p.\n", mb->size, start_addr);
    ++mm->w_queries;
}

memory_block* mm_read(main_memory* mm, void* start_addr)
{
    // the block we ask to read must be aligned to a MAIN_MEMORY block
    assert((size_t) (start_addr - MAIN_MEMORY_START_ADDR)
           % MAIN_MEMORY_BLOCK_SIZE == 0);
    
    // make sure we are not out of bounds
    assert(start_addr + MAIN_MEMORY_BLOCK_SIZE <=
           (void*) MAIN_MEMORY_START_ADDR + MAIN_MEMORY_SIZE);
    
    memory_block* result
        = mb_new(start_addr, MAIN_MEMORY_BLOCK_SIZE,
                 mm->data + (size_t) start_addr - MAIN_MEMORY_START_ADDR);
        
    printf("MM: Read %zu bytes at %p.\n", result->size, start_addr);
    ++mm->r_queries;
    
    return result;
}

void mm_free(main_memory* mm)
{
    free(mm->data);
    free(mm);
}