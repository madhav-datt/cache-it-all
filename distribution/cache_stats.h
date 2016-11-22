#ifndef CACHE_STATS_H
#define CACHE_STATS_H

typedef struct cache_stats
{
    unsigned int w_queries;
    unsigned int r_queries;
    unsigned int w_misses;
    unsigned int r_misses;
} cache_stats;

cache_stats cs_init();

#endif