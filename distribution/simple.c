#include <stdlib.h>

#include "memory_block.h"
#include "simple.h"

simple_cache* sc_init(main_memory* mm)
{
    simple_cache* result = malloc(sizeof(simple_cache));
    result->mm = mm;
    result->cs = cs_init();
    return result;
};

void sc_store_word(simple_cache* sc, void* addr, unsigned int val)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;
    
    // Load memory block from main memory
    memory_block* mb = mm_read(sc->mm, mb_start_addr);
    
    // Update relevant word in memory block
    unsigned int* mb_addr = mb->data + addr_offt;
    *mb_addr = val;
    
    // Story memory block back into main memory
    mm_write(sc->mm, mb_start_addr, mb);
    
    // Update statistics
    ++sc->cs.w_queries;
    ++sc->cs.w_misses;
    
    // Free memory block
    mb_free(mb);
}

unsigned int sc_load_word(simple_cache* sc, void* addr)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;
    
    // Load memory block from main memory
    memory_block* mb = mm_read(sc->mm, mb_start_addr);
    
    // Extract the word we care about
    unsigned int* mb_addr = mb->data + addr_offt;
    unsigned int result = *mb_addr;
    
    // Update statistics
    ++sc->cs.r_queries;
    ++sc->cs.r_misses;
    
    // Free memory block
    mb_free(mb);
    
    // Return result
    return result;
}

void sc_free(simple_cache* sc)
{
    // Note: your cache free functions should NOT free main memory
    // Main memory is free'd by the main function after sc_free is called
    free(sc);
}