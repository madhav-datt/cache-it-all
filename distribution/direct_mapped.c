#include <stdint.h>

#include "memory_block.h"
#include "direct_mapped.h"

direct_mapped_cache* dmc_init(main_memory* mm)
{
    // TODO
    
    return 0;
}

// Optional
/*
static int addr_to_set(void* addr)
{
}
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
    // TODO
}