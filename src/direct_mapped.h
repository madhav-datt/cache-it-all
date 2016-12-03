#ifndef DIRECT_MAPPED_H
#define DIRECT_MAPPED_H

#include "main_memory.h"
#include "cache_stats.h"

#define DIRECT_MAPPED_NUM_SETS 16
#define DIRECT_MAPPED_NUM_SETS_LN 4

typedef struct direct_map_set
{
    int is_valid;
    int is_dirty;
    memory_block* mem_block;
} direct_map_set;

typedef struct direct_mapped_cache
{
    main_memory* mm;
    cache_stats cs;
    direct_map_set* cache_set;
} direct_mapped_cache;

// Do not edit below this line

direct_mapped_cache* dmc_init(main_memory* mm);

void dmc_store_word(direct_mapped_cache* dmc, void* addr, unsigned int val);

unsigned int dmc_load_word(direct_mapped_cache* dmc, void* addr);

void dmc_free(direct_mapped_cache* dmc);

#endif