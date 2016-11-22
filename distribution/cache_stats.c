#include <string.h>

#include "cache_stats.h"

cache_stats cs_init()
{
    cache_stats result;
    memset(&result, 0, sizeof(result));
    return result;
}