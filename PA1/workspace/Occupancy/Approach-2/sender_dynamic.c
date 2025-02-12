#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <x86intrin.h>
#include <unistd.h>
#include "cacheutils.h"

#include "sys_config.h"

// Random memory accesses to maximize cache thrashing
void thrash_cache(volatile char *buffer, float thrash_level)
{
    for (size_t i = 0; i < NUM_BLOCKS_THR * thrash_level; i++)
    {
        // size_t index = (rand() % NUM_BLOCKS_THR) * BLOCK_SIZE;
        size_t index = i * BLOCK_SIZE; // Sequentially touch cache lines
        // maccess(&buffer[index]);
        buffer[index] += 1; // Touch random cache lines
    }
}
#define SYNC_MASK 0x1F
#define BIT_WINDOW 6500000
#define SYNC_INTERVAL 15
#define WINDOW_TIME 300 * 1000 // 200 ms
size_t WINDOW_CYCLES;

size_t count_window_cycles(long win_time)
{
    size_t end;
    size_t start = rdtsc();
    usleep(win_time);
    end = rdtsc();
    return end - start;
}
int main()
{

    // Allocate memory buffer larger than LLC
    volatile char *buffer = (volatile char *)aligned_alloc(BLOCK_SIZE, THRASH_SIZE*1.5);
    if (!buffer)
    {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    WINDOW_CYCLES = 100000 * (count_window_cycles(WINDOW_TIME) / 100000);

    // Continuous LLC thrashing loop
    size_t i = 0;
    float thrash_level = 0;
    // hello -> 01 10 10 00 01 10 01 01 01 10 11 00 01 10 11 00 01 10 11 11 
    // hello world! I love cs773
    // array([1, 2, 2, 1, 2, 1, 1, 1, 2, 3, 1, 2, 3, 1, 2, 3, 3])
    short bits[100] = {1,2,2,0,1,2,1,1,1,2,3,0,1,2,3,0,1,2,3,3,0,2,0,0,1,3,1,3,1,2,3,3,1,3,0,2,1,2,3,0,1,2,1,0,0,2,0,1,0,2,0,0,1,0,2,1,0,2,0,0,1,2,3,0,1,2,3,3,1,3,1,2,1,2,1,1,0,2,0,0,1,2,0,3,1,3,0,3,0,3,1,3,0,3,1,3,0,3,0,3};
    // printf("SSYNC: %ld\n", i++);
    while (i < 100)
    {
        size_t start_window = rdtsc();
        do
        {
            switch (bits[i])
            {
            case 0:
                thrash_level = 0.20;
                break;
            case 1:
                thrash_level = 0.35;
                break;
            case 2:
                thrash_level = 0.6;
                break;
            case 3:
                thrash_level = 1.0;
                break;
            case 4:
                thrash_level = 0;
                break;
            }
            thrash_cache(buffer, thrash_level);

        } while (rdtsc() - start_window < WINDOW_CYCLES);
        i++;
    }
    printf("SENDER end\n");

    free((void *)buffer);
    return 0;
}
