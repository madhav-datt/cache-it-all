CC=gcc
MODE=normal

ifeq ($(MODE),drmem)
	CFLAGS=-std=c11 -Wall -O3 -g -m32 -fno-inline -fno-omit-frame-pointer
else
	CFLAGS=-std=c11 -Wall -O3 -g
endif

all: main

main: memory_block.o main_memory.o cache_stats.o simple.o direct_mapped.o fully_associative.o set_associative.o main.c
	$(CC) $(CFLAGS) memory_block.o main_memory.o cache_stats.o simple.o direct_mapped.o fully_associative.o set_associative.o main.c -o main

clean:
	rm *o main