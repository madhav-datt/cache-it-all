#include <stdint.h>

#include "memory_block.h"
#include "set_associative.h"

set_associative_cache* sac_init(main_memory* mm)
{
    // TODO
    
    return 0;
}

// Optional
/*
static int addr_to_set(void* addr)
{
}

static void mark_as_used(set_associative_cache* sac, int set, int way)
{
}

static int lru(set_associative_cache* sac, int set)
{
}
*/

void sac_store_word(set_associative_cache* sac, void* addr, unsigned int val)
{
    // TODO
}


unsigned int sac_load_word(set_associative_cache* sac, void* addr)
{
    // TODO
    return 0;
}

void sac_free(set_associative_cache* sac)
{
    // TODO
}