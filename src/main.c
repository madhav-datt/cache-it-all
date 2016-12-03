#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main_memory.h"
#include "simple.h"
#include "direct_mapped.h"
#include "fully_associative.h"
#include "set_associative.h"

#define MODE_SC 0
#define MODE_DMC 1
#define MODE_FAC 2
#define MODE_SAC 3

void print_stats(main_memory* mm, cache_stats cs)
{   
    int w_hits = cs.w_queries - cs.w_misses;
    int r_hits = cs.r_queries - cs.r_misses;
    
    double whr = (double) w_hits / (double) cs.w_queries * 100;
    double rhr = (double) r_hits / (double) cs.r_queries * 100;

    int t_hits = w_hits + r_hits;
    unsigned int t_queries = cs.w_queries + cs.r_queries;
    double thr = (double) t_hits / (double) t_queries * 100;
    
    printf("*******************************************\n");
    printf("Write Hit Rate:\t\t%.0lf%% (%d/%d)\n", whr, w_hits, cs.w_queries);
    printf("Read Hit Rate:\t\t%.0lf%% (%d/%d)\n", rhr, r_hits, cs.r_queries);
    printf("Total Hit Rate:\t\t%.0lf%% (%d/%d)\n", thr, t_hits, t_queries);
    printf("Writes to Main Memory:\t%d\n", mm->w_queries);
    printf("Reads from Main Memory:\t%d\n", mm->r_queries);
    printf("*******************************************\n");
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s sc|dmc|fac|sac input_file\n", argv[0]);
        exit(1);
    }
    
    int mode;
    if (strcmp(argv[1], "sc") == 0)
        mode = MODE_SC;
    else if (strcmp(argv[1], "dmc") == 0)
        mode = MODE_DMC;
    else if (strcmp(argv[1], "fac") == 0)
        mode = MODE_FAC;
    else if (strcmp(argv[1], "sac") == 0)
        mode = MODE_SAC;
    else
    {
        fprintf(stderr, "Error: Mode must be sc, dmc, fac, or sac.\n");
        exit (2);
    }
    
    FILE* input_file = fopen(argv[2], "r");
    if (input_file == 0)
    {
        fprintf(stderr, "Error: Could not open %s.\n", argv[2]);
        exit(3);
    }
    
    main_memory* mm = mm_init();
    simple_cache* sc = 0;
    direct_mapped_cache* dmc = 0;
    fully_associative_cache* fac = 0;
    set_associative_cache* sac = 0;
    if (mode == MODE_SC)
        sc = sc_init(mm);
    else if (mode == MODE_DMC)
        dmc = dmc_init(mm);
    else if (mode == MODE_FAC)
        fac = fac_init(mm);
    else if (mode == MODE_SAC)
        sac = sac_init(mm);
    
    char* line = 0;
    size_t line_len = 0;
    unsigned int line_num = 0;
    while (getline(&line, &line_len, input_file) != -1)
    {
        ++line_num;
        
        char RW;
        void* addr;
        unsigned int val;
        
        int tolkens_found = sscanf(line, "%c %p %d", &RW, &addr, &val);

        if (strlen(line) != 1 && RW != '#')
        {
            if ((RW != 'R' && RW != 'W')
                || (RW == 'R' && tolkens_found != 2)
                || (RW == 'W' && tolkens_found !=3))
                fprintf(stderr, "Warning: Format error on line %d: %s", line_num,
                        line);
            else
            {
                if (RW == 'W')
                {
                    if (mode == MODE_SC)
                        sc_store_word(sc, addr, val);
                    else if (mode == MODE_DMC)
                        dmc_store_word(dmc, addr, val);
                    else if (mode == MODE_FAC)
                        fac_store_word(fac, addr, val);
                    else if (mode == MODE_SAC)
                        sac_store_word(sac, addr, val);
                    printf("Wrote to %p: %d\n\n", addr, val);
                }
                else
                {
                    if (mode == MODE_SC)
                        val = sc_load_word(sc, addr);
                    else if (mode == MODE_DMC)
                        val = dmc_load_word(dmc, addr);
                    else if (mode == MODE_FAC)
                        val = fac_load_word(fac, addr);
                    else if (mode == MODE_SAC)
                        val = sac_load_word(sac, addr);
                    printf("Read from %p: %d\n\n", addr, val);
                }
            }
        }
        
    }
    free(line);
    
    fclose(input_file);
    
    if (mode == MODE_SC)
    {
        print_stats(mm, sc->cs);
        sc_free(sc);
    }
    else if (mode == MODE_DMC)
    {
        print_stats(mm, dmc->cs);
        dmc_free(dmc);
    }
    else if (mode == MODE_FAC)
    {
        print_stats(mm, fac->cs);
        fac_free(fac);
    }
    else if (mode == MODE_SAC)
    {
        print_stats(mm, sac->cs);
        sac_free(sac);
    }
    mm_free(mm);
    
    return 0;
}