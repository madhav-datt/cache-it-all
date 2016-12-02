#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory_block.h"
#include "fully_associative.h"

#define NEARING_OVERFLOW 2147483640

/**
 * Allocate memory and initialize cache
 * @param mm: main memory
 * @return initialized cache
 */
fully_associative_cache* fac_init(main_memory* mm)
{
    fully_associative_cache* result = malloc(sizeof(fully_associative_cache));
    result->mm = mm;
    result->cs = cs_init();
    result->num_sets = 0;
    result->cache_set = malloc(FULLY_ASSOCIATIVE_NUM_WAYS * sizeof(fully_assoc_set));
    result->usage = malloc(FULLY_ASSOCIATIVE_NUM_WAYS * sizeof(int));
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        result->cache_set[i].is_valid = 0;
        result->cache_set[i].is_dirty = 0;

        // Initialize data with dummy values
        result->cache_set[i].mem_block = malloc(sizeof(memory_block));
        result->cache_set[i].mem_block->data = NULL;
        result->cache_set[i].mem_block->size = MAIN_MEMORY_BLOCK_SIZE;
        result->cache_set[i].mem_block->start_addr = NULL;

        result->usage[i] = 0;
    }
    return result;
}


/**
 * Find least recently used memory block for eviction
 * @param fac: pointer to cache
 * @return integer way - corresponding to index of memory block to be evicted
 */
static int lru(fully_associative_cache* fac)
{
    // Check if cache still has space, ie. no eviction needed
    if (fac->num_sets < FULLY_ASSOCIATIVE_NUM_WAYS)
        return fac->num_sets++;

    int max_index = 0;
    float max_value = fac->usage[0];

    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (fac->usage[i] > max_value)
        {
            max_value = fac->usage[i];
            max_index = i;
        }
    }
    return max_index;
}

/**
 * Handle and eliminate potential LRU overflows
 * Change fac->usage values to smaller, scaled down quantities while maintaining relative usage order
 * (ie. least recently used, 2nd least recently used etc. are maintained)
 * @param fac: pointer to cache
 */
static void normalize_usage_count(fully_associative_cache* fac)
{
    for (int i = 0; i < fac->num_sets; i++)
        fac->usage[i] /= 2;
}

/**
 * Find way number in case of hit
 * @param fac: pointer to cache
 * @param mb_start_addr: start address of required memory block
 * @return index (way number) if hit, -1 if miss
 */
static int find_hit(fully_associative_cache* fac, void* mb_start_addr)
{
    int mem_block_tag = (int) (uintptr_t) mb_start_addr;
    mem_block_tag = mem_block_tag >> MAIN_MEMORY_BLOCK_SIZE_LN;

    // Compare tag against all memory blocks currently filled into ways
    for (int i = 0; i < fac->num_sets; i++)
    {
        int current_addr = (int) (uintptr_t) fac->cache_set[i].mem_block->start_addr;
        int current_tag = current_addr >> MAIN_MEMORY_BLOCK_SIZE_LN;
        if (fac->cache_set[i].is_valid == 1 && current_tag == mem_block_tag)
            return i;
    }
    return -1;
}

/**
 * Store val at addr (write query)
 * @param fac: pointer to cache
 * @param addr: address where data is to be stored (always properly aligned)
 * @param val: data
 */
void fac_store_word(fully_associative_cache* fac, void* addr, unsigned int val)
{
    // Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int index = find_hit(fac, mb_start_addr);

    // Miss - Addr was not previously loaded into cache
    if (index == -1)
    {
        // Get least recently used way
        index = lru(fac);

        // Write memory block to main memory if valid and dirty
        if (fac->cache_set[index].is_valid == 1 && fac->cache_set[index].is_dirty == 1)
            mm_write(fac->mm, fac->cache_set[index].mem_block->start_addr, fac->cache_set[index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(fac->mm, mb_start_addr);
        mb_free(fac->cache_set[index].mem_block);

        fac->cache_set[index].mem_block = mb;
        fac->cache_set[index].is_valid = 1;
        fac->cache_set[index].is_dirty = 0;

        fac->cs.w_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = fac->cache_set[index].mem_block->data + addr_offt;
    *mb_addr = val;
    fac->cache_set[index].is_dirty = 1;

    fac->usage[index] = 0;
    for (int i = 0; i < fac->num_sets; i++)
        if (i != index)
            fac->usage[i]++;

    // Update statistics
    fac->cs.w_queries++;

    // Normalize and adjust to eliminate potential overflows every NEARING_OVERFLOW queries
    if ((fac->cs.w_queries + fac->cs.r_queries) % NEARING_OVERFLOW == 0)
        normalize_usage_count(fac);
}

/**
 * Read value at addr (read query)
 * @param fac: pointer to cache
 * @param addr: address where data is stored
 * @return val: data stored at addr
 */
unsigned int fac_load_word(fully_associative_cache* fac, void* addr)
{
    // Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int index = find_hit(fac, mb_start_addr);

    // Miss - Addr was not previously loaded into cache
    if (index == -1)
    {
        // Get least recently used way
        index = lru(fac);

        // Write memory block to main memory if valid and dirty
        if (fac->cache_set[index].is_valid == 1 && fac->cache_set[index].is_dirty == 1)
            mm_write(fac->mm, fac->cache_set[index].mem_block->start_addr, fac->cache_set[index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(fac->mm, mb_start_addr);
        mb_free(fac->cache_set[index].mem_block);

        fac->cache_set[index].mem_block = mb;
        fac->cache_set[index].is_valid = 1;
        fac->cache_set[index].is_dirty = 0;

        fac->cs.r_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = fac->cache_set[index].mem_block->data + addr_offt;

    fac->usage[index] = 0;
    for (int i = 0; i < fac->num_sets; i++)
        if (i != index)
            fac->usage[i]++;

    // Update statistics
    fac->cs.r_queries++;

    // Normalize and adjust to eliminate potential overflows every NEARING_OVERFLOW queries
    if ((fac->cs.w_queries + fac->cs.r_queries) % NEARING_OVERFLOW == 0)
        normalize_usage_count(fac);

    return *mb_addr;
}

/**
 * Free memory allocated to cache
 * @param fac: pointer to cache
 */
void fac_free(fully_associative_cache* fac)
{
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
        mb_free(fac->cache_set[i].mem_block);

    free(fac->usage);
    free(fac->cache_set);
    free(fac);
}
