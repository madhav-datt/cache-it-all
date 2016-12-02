#include <stdint.h>
#include <stdio.h>

#include "memory_block.h"
#include "set_associative.h"

#define NEARING_OVERFLOW 2147483640

/**
 * Allocate memory and initialize cache
 * @param mm: main memory
 * @return initialized cache
 */
set_associative_cache* sac_init(main_memory* mm)
{
    set_associative_cache* result = malloc(sizeof(set_associative_cache));
    result->mm = mm;
    result->cs = cs_init();
    result->cache_set = malloc(SET_ASSOCIATIVE_NUM_SETS * sizeof(sac_map_set));
    for (int i = 0; i < SET_ASSOCIATIVE_NUM_SETS; i++)
    {
        result->cache_set[i].num_ways = 0;
        result->cache_set[i].usage = malloc(SET_ASSOCIATIVE_NUM_WAYS * sizeof(int));
        result->cache_set[i].ways = malloc(SET_ASSOCIATIVE_NUM_WAYS * sizeof(sac_map_way));

        for(int j = 0; j < SET_ASSOCIATIVE_NUM_WAYS; j++)
        {
            result->cache_set[i].ways[j].is_valid = 0;
            result->cache_set[i].ways[j].is_dirty = 0;

            // Initialize data with dummy values
            result->cache_set[i].ways[j].mem_block = malloc(sizeof(memory_block));
            result->cache_set[i].ways[j].mem_block->data = NULL;
            result->cache_set[i].ways[j].mem_block->size = MAIN_MEMORY_BLOCK_SIZE;
            result->cache_set[i].ways[j].mem_block->start_addr = NULL;
        }
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
    return (result >> MAIN_MEMORY_BLOCK_SIZE_LN) & ((1 << SET_ASSOCIATIVE_NUM_SETS_LN) - 1);
}

/**
 * Find least recently used memory block for eviction
 * @param sac: pointer to cache
 * @param set_index: index of corresponding set for which lru is to be found
 * @return integer way - corresponding to index of memory block to be evicted
 */
static int lru(set_associative_cache* sac, int set_index)
{
    // Check if cache still has space, ie. no eviction needed
    if (sac->cache_set[set_index].num_ways < SET_ASSOCIATIVE_NUM_WAYS)
        return sac->cache_set[set_index].num_ways++;

    int max_index = 0;
    int max_value = sac->cache_set[set_index].usage[0];

    for (int i = 0; i < SET_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (sac->cache_set[set_index].usage[i] > max_value)
        {
            max_value = sac->cache_set[set_index].usage[i];
            max_index = i;
        }
    }
    return max_index;
}

/**
 * Custom comparator to sort indices of usage array fac->usage, such that index (i) corresponding to maximum
 * fac->usage[i] value is first in the sorted order
 * @param a: parameter 1
 * @param b: parameter 2
 * @return Comparison between parameters a and b
 */
// array contains a copy of fac->usage[i]
static int* array;
static int cmp(const void* a, const void* b)
{
    int ia = *(int*) a;
    int ib = *(int*) b;
    return array[ia] < array[ib] ? -1 : array[ia] > array[ib];
}

/**
 * Handle and eliminate potential LRU overflows
 * Change sac->usage values to small quantities while maintaining relative usage order
 * (ie. least recently used, 2nd least recently used etc. are maintained)
 * @param sac: pointer to cache
 * @param set_index: index of corresponding set for which lru is to be found
 */
static void normalize_usage_count(set_associative_cache* sac, int set_index)
{
    // Create array of indices for sorting according to cmp
    int* index = malloc(sac->cache_set[set_index].num_ways * sizeof(int));
    for(int i = 0; i < sac->cache_set[set_index].num_ways; i++)
        index[i] = i;

    // Sort indices of usage array fac->usage, such that index (i) corresponding to maximum fac->usage[i] value
    // is first in the sorted order
    array = sac->cache_set[set_index].usage;
    qsort(index, (size_t) sac->cache_set[set_index].num_ways, sizeof(*index), cmp);

    // Change fac->usage values to small quantities while maintaining relative usage order
    int num_usages_least = sac->cache_set[set_index].num_ways + 1;
    for (int i = 0; i < sac->cache_set[set_index].num_ways; i++)
        sac->cache_set[set_index].usage[index[i]] = num_usages_least--;

    free(index);
}

/**
 * Find way number in case of hit
 * @param sac: pointer to cache
 * @param mb_start_addr: start address of required memory block
 * @param set_index: index of corresponding set for which lru is to be found
 * @return index (way number) if hit, -1 if miss
 */
static int find_hit(set_associative_cache* sac, void* mb_start_addr, int set_index)
{
    int mem_block_tag = (int) (uintptr_t) mb_start_addr;
    mem_block_tag = mem_block_tag >> (MAIN_MEMORY_BLOCK_SIZE_LN + SET_ASSOCIATIVE_NUM_SETS_LN);

    // Compare tag against all memory blocks currently filled into ways
    for (int i = 0; i < sac->cache_set[set_index].num_ways; i++)
    {
        int current_addr = (int) (uintptr_t) sac->cache_set[set_index].ways[i].mem_block->start_addr;
        int current_tag = current_addr >> (MAIN_MEMORY_BLOCK_SIZE_LN + SET_ASSOCIATIVE_NUM_SETS_LN);
        if (sac->cache_set[set_index].ways[i].is_valid == 1 && current_tag == mem_block_tag)
            return i;
    }
    return -1;
}

/**
 * Store val at addr (write query)
 * @param sac: pointer to cache
 * @param addr: address where data is to be stored (always properly aligned)
 * @param val: data
 */
void sac_store_word(set_associative_cache* sac, void* addr, unsigned int val)
{
    // Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int set_index = addr_to_set(mb_start_addr);
    int way_index = find_hit(sac, mb_start_addr, set_index);

    // Miss - Addr was not previously loaded into cache
    if (way_index == -1)
    {
        // Get least recently used way
        way_index = lru(sac, set_index);

        // Write memory block to main memory if valid and dirty
        if (sac->cache_set[set_index].ways[way_index].is_valid == 1 &&
                sac->cache_set[set_index].ways[way_index].is_dirty == 1)
            mm_write(sac->mm, sac->cache_set[set_index].ways[way_index].mem_block->start_addr,
                     sac->cache_set[set_index].ways[way_index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(sac->mm, mb_start_addr);
        mb_free(sac->cache_set[set_index].ways[way_index].mem_block);

        sac->cache_set[set_index].ways[way_index].mem_block = mb;
        sac->cache_set[set_index].ways[way_index].is_valid = 1;
        sac->cache_set[set_index].ways[way_index].is_dirty = 0;

        sac->cs.w_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = sac->cache_set[set_index].ways[way_index].mem_block->data + addr_offt;
    *mb_addr = val;
    sac->cache_set[set_index].ways[way_index].is_dirty = 1;

    sac->cache_set[set_index].usage[way_index] = 0;
    for (int i = 0; i < sac->cache_set[set_index].num_ways; i++)
        if (i != way_index)
            sac->cache_set[set_index].usage[i]++;

    // Update statistics
    sac->cs.w_queries++;

    // Normalize and adjust to eliminate potential overflows every NEARING_OVERFLOW queries
    if ((sac->cs.w_queries + sac->cs.r_queries) % NEARING_OVERFLOW == 0)
        normalize_usage_count(sac, set_index);
}

/**
 * Read value at addr (read query)
 * @param sac: pointer to cache
 * @param addr: address where data is stored
 * @return val: data stored at addr
 */
unsigned int sac_load_word(set_associative_cache* sac, void* addr)
{
// Check if read query is a miss

    // Pre-compute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    int set_index = addr_to_set(mb_start_addr);
    int way_index = find_hit(sac, mb_start_addr, set_index);

    // Miss - Addr was not previously loaded into cache
    if (way_index == -1)
    {
        // Get least recently used way
        way_index = lru(sac, set_index);

        // Write memory block to main memory if valid and dirty
        if (sac->cache_set[set_index].ways[way_index].is_valid == 1 &&
            sac->cache_set[set_index].ways[way_index].is_dirty == 1)
            mm_write(sac->mm, sac->cache_set[set_index].ways[way_index].mem_block->start_addr,
                     sac->cache_set[set_index].ways[way_index].mem_block);

        // Load memory block from main memory
        memory_block* mb = mm_read(sac->mm, mb_start_addr);
        mb_free(sac->cache_set[set_index].ways[way_index].mem_block);

        sac->cache_set[set_index].ways[way_index].mem_block = mb;
        sac->cache_set[set_index].ways[way_index].is_valid = 1;
        sac->cache_set[set_index].ways[way_index].is_dirty = 0;

        sac->cs.r_misses++;
    }

    // Extract required word care about
    unsigned int* mb_addr = sac->cache_set[set_index].ways[way_index].mem_block->data + addr_offt;

    sac->cache_set[set_index].usage[way_index] = 0;
    for (int i = 0; i < sac->cache_set[set_index].num_ways; i++)
        if (i != way_index)
            sac->cache_set[set_index].usage[i]++;

    // Update statistics
    sac->cs.r_queries++;

    // Normalize and adjust to eliminate potential overflows every NEARING_OVERFLOW queries
    if ((sac->cs.w_queries + sac->cs.r_queries) % NEARING_OVERFLOW == 0)
        normalize_usage_count(sac, set_index);

    return *mb_addr;
}

/**
 * Free memory allocated to cache
 * @param sac: pointer to cache
 */
void sac_free(set_associative_cache* sac)
{
    for (int i = 0; i < SET_ASSOCIATIVE_NUM_SETS; i++)
    {
        for (int j = 0; j < SET_ASSOCIATIVE_NUM_WAYS; j++)
            mb_free(sac->cache_set[i].ways[j].mem_block);

        free(sac->cache_set[i].ways);
        free(sac->cache_set[i].usage);
    }
    free(sac->cache_set);
    free(sac);
}