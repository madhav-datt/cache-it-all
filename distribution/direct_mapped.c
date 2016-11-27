#include <stdint.h>

#include "memory_block.h"
#include "direct_mapped.h"

#define ADDR_SIZE_BITS 16

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
    result->cache_set = malloc(DIRECT_MAPPED_NUM_SETS * sizeof(set));
    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++)
    {
        result->cache_set[i].is_valid = 0;
        result->cache_set[i].data = malloc(MAIN_MEMORY_BLOCK_SIZE);
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
    //TODO
    return 0;
}


/**
 * Convert decimal to 16-bit binary
 * @param n: decimal value
 * @return int array with binary representation
 */
static int* to_binary(int n)
{
    int count = 0;
    int* pointer;

    pointer = malloc((ADDR_SIZE_BITS) * sizeof(int));

    for (int c = ADDR_SIZE_BITS - 1 ; c >= 0 ; c--)
    {
        int d = n >> c;
        if (d & 1)
            pointer[count] = 1;
        else
            pointer[count] = 0;
        count++;
    }
    return pointer;
}

/**
 * Convert binary representation to decimal
 * @param bin: 16-bit binary in integer array
 * @return integer decimal value
 */
static int to_decimal(int* bin)
{
    int dec = 0;
    for (int i = 0; i < ADDR_SIZE_BITS; i++)
    {
        if (bin[i] == 1)
            dec = dec * 2 + 1;
        else
            dec *= 2;
    }
    return dec;
}

/**
 * Store val at addr
 * @param dmc: pointed to cache
 * @param addr: address where data is to be stored (always properly aligned)
 * @param val: data
 */
void dmc_store_word(direct_mapped_cache* dmc, void* addr, unsigned int val)
{
    // TODO
}

unsigned int dmc_load_word(direct_mapped_cache* dmc, void* addr)
{   
    // TODO
    
    return 0;
}

void dmc_free(direct_mapped_cache* dmc)
{
    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++)
        free(dmc->cache_set[i].data);

    free(dmc->cache_set);
    free(dmc);
}