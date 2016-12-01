#include <stdint.h>
#include <stdio.h>

#include "memory_block.h"
#include "set_associative.h"

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
 * Change fac->usage values to small quantities while maintaining relative usage order
 * (ie. least recently used, 2nd least recently used etc. are maintained)
 * @param fac: pointer to cache
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

void sac_store_word(set_associative_cache* sac, void* addr, unsigned int val)
{
    // TODO
}


unsigned int sac_load_word(set_associative_cache* sac, void* addr)
{
    // TODO
    return 0;
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