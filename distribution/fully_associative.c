#include "memory_block.h"
#include "fully_associative.h"

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
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        result->cache_set[i].is_valid = 0;
        result->cache_set[i].is_dirty = 0;
        result->cache_set[i].mem_block = malloc(sizeof(memory_block));
    }
    return result;
}

// Optional
/*
static void mark_as_used(fully_associative_cache* fac, int way)
{
}

static int lru(fully_associative_cache* fac)
{
}
*/

void fac_store_word(fully_associative_cache* fac, void* addr, unsigned int val)
{
    // TODO
}


unsigned int fac_load_word(fully_associative_cache* fac, void* addr)
{
    // TODO
    
    return 0;
}

/**
 * Free memory allocated to cache
 * @param fac: pointer to cache
 */
void fac_free(fully_associative_cache* fac)
{
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
        mb_free(fac->cache_set[i].mem_block);

    free(fac->cache_set);
    free(fac);
}
