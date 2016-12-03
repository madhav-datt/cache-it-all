#ifndef SIMPLE_H
#define SIMPLE_H

#include "main_memory.h"
#include "cache_stats.h"

typedef struct simple_cache
{
    main_memory* mm;
    cache_stats cs;
} simple_cache;

simple_cache* sc_init(main_memory* mm);

void sc_store_word(simple_cache* sc, void* addr, unsigned int val);

unsigned int sc_load_word(simple_cache* sc, void* addr);

void sc_free(simple_cache* sc);

#endif