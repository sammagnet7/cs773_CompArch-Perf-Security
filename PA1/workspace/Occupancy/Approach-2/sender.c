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
#define STABILIZE_WINDOW
unsigned long long int WINDOW_LEN = BIT_WINDOW;
#define SYNC_INTERVAL 15
#define SEND_BIT 1
#define PER_BIT_WINDOW 10000
#define WINDOW_TIME 200 * 1000 // 200 ms
size_t WINDOW_CYCLES;

size_t count_window_cycles()
{
    size_t end;
    size_t start = rdtsc();
    usleep(WINDOW_TIME);
    end = rdtsc();
    return end - start;
}
int main()
{
    // Pin process to a single core for consistent behavior
    // cpu_set_t mask;
    // CPU_ZERO(&mask);
    // CPU_SET(0, &mask);
    // sched_setaffinity(0, sizeof(mask), &mask);

    // Allocate memory buffer larger than LLC
    volatile char *buffer = (volatile char *)aligned_alloc(BLOCK_SIZE, THRASH_SIZE*1.5);
    if (!buffer)
    {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    WINDOW_CYCLES = 100000 * (count_window_cycles() / 100000);

    printf("Thrashing LLC...\n");

    // Continuous LLC thrashing loop
    size_t i = 2, j = 0, thrash_level = 0;
    short bits[5] = {0, 1, 3, 4, 2};
    // printf("SSYNC: %ld\n", i++);
    while (1)
    {
        size_t start_window = rdtsc();
        do
        {
            switch (bits[i])
            {
            case 0:
                thrash_level = 0.12;
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
        // i++;
    }
    printf("SENDER end\n");

    free((void *)buffer);
    return 0;
}
