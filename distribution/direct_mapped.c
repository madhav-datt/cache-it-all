#include <stdint.h>
#include <stdio.h>

#include "memory_block.h"
#include "direct_mapped.h"

/**
 * Allocate memory and initialize cache
 * @param mm: main memory
 * @return initialized cache
 */
direct_mapped_cache* dmc_init(main_memory* mm)
{
    direct_mapped_cache* result = malloc(sizeof(direct_mapped_cache));
    result->mm = mm;
    result->cs = cs_init();
    result->cache_set = malloc(DIRECT_MAPPED_NUM_SETS * sizeof(direct_map_set));
    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++)
    {
        result->cache_set[i].is_valid = 0;
        result->cache_set[i].is_dirty = 0;

        // Initialize data with dummy values
        result->cache_set[i].mem_block = malloc(sizeof(memory_block));
        result->cache_set[i].mem_block->data = NULL;
        result->cache_set[i].mem_block->size = MAIN_MEMORY_BLOCK_SIZE;
        result->cache_set[i].mem_block->start_addr = NULL;
    }
    return result;
}

/**
 * Compute index corresponding to block number
 * @param addr: full address
 * @return int index corresponding to block number
 */
static int addr_to_set(void* addr)
{
    unsigned int result = (unsigned int) (uintptr_t) addr;
    return (result >> MAIN_MEMORY_BLOCK_SIZE_LN) & ((1 << DIRECT_MAPPED_NUM_SETS_LN) - 1);
}

/**
 * Store val at addr (write query)
 * @param dmc: pointer to cache
 * @param addr: address where data is to be stored (always properly aligned)
 * @param val: data
 */
void dmc_store_word(direct_mapped_cache* dmc, void* addr, unsigned int val)
{
    // Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int index = addr_to_set(mb_start_addr);
    int result = (int) (uintptr_t) mb_start_addr;
    int tag = result >> (MAIN_MEMORY_BLOCK_SIZE_LN + DIRECT_MAPPED_NUM_SETS_LN);
    int mem_addr_tag = (int) (((uintptr_t) dmc->cache_set[index].mem_block->start_addr)
                >> (MAIN_MEMORY_BLOCK_SIZE_LN + DIRECT_MAPPED_NUM_SETS_LN));

    // Miss - Addr was not previously loaded into cache
    if (!(dmc->cache_set[index].is_valid == 1 && mem_addr_tag == tag))
    {
        // Write memory block to main memory if valid and dirty
        if (dmc->cache_set[index].is_valid == 1 && dmc->cache_set[index].is_dirty == 1)
            mm_write(dmc->mm, dmc->cache_set[index].mem_block->start_addr, dmc->cache_set[index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(dmc->mm, mb_start_addr);
        mb_free(dmc->cache_set[index].mem_block);

        dmc->cache_set[index].mem_block = mb;
        dmc->cache_set[index].is_valid = 1;
        dmc->cache_set[index].is_dirty = 0;

        dmc->cs.w_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = dmc->cache_set[index].mem_block->data + addr_offt;
    *mb_addr = val;
    dmc->cache_set[index].is_dirty = 1;

    // Update statistics
    dmc->cs.w_queries++;
}

/**
 * Read value at addr (read query)
 * @param dmc: pointer to cache
 * @param addr: address where data is stored
 * @return val: data stored at addr
 */
unsigned int dmc_load_word(direct_mapped_cache* dmc, void* addr)
{
    // Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int index = addr_to_set(mb_start_addr);
    int result = (int) (uintptr_t) mb_start_addr;
    int tag = result >> (MAIN_MEMORY_BLOCK_SIZE_LN + DIRECT_MAPPED_NUM_SETS_LN);
    int mem_addr_tag = (int) (((uintptr_t) dmc->cache_set[index].mem_block->start_addr)
                >> (MAIN_MEMORY_BLOCK_SIZE_LN + DIRECT_MAPPED_NUM_SETS_LN));

    // Miss - Addr was not previously loaded into cache
    if (!(dmc->cache_set[index].is_valid == 1 && mem_addr_tag == tag))
    {
        // Write memory block to main memory if valid and dirty
        if (dmc->cache_set[index].is_valid == 1 && dmc->cache_set[index].is_dirty == 1)
            mm_write(dmc->mm, dmc->cache_set[index].mem_block->start_addr, dmc->cache_set[index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(dmc->mm, mb_start_addr);
        mb_free(dmc->cache_set[index].mem_block);

        dmc->cache_set[index].mem_block = mb;
        dmc->cache_set[index].is_valid = 1;
        dmc->cache_set[index].is_dirty = 0;
        dmc->cs.r_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = dmc->cache_set[index].mem_block->data + addr_offt;

    // Update statistics
    dmc->cs.r_queries++;

    return *mb_addr;
}

/**
 * Free memory allocated to cache
 * @param dmc: pointer to cache
 */
void dmc_free(direct_mapped_cache* dmc)
{
    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++)
        mb_free(dmc->cache_set[i].mem_block);

    free(dmc->cache_set);
    free(dmc);
}
