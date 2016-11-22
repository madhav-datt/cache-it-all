#include "memory_block.h"
#include "fully_associative.h"

fully_associative_cache* fac_init(main_memory* mm)
{
    // TODO
    
    return 0;
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

void fac_free(fully_associative_cache* fac)
{
    // TODO
}