#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include "cacheutils.h"
#include "sys_config.h"
#define WINDOW_TIME 200 * 1000 // 200 ms
size_t WINDOW_CYCLES;
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
size_t count_window_cycles()
{
    size_t end;
    size_t start = rdtsc();
    usleep(WINDOW_TIME);
    end = rdtsc();
    return end - start;
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <float>\n", argv[0]);
        return 1;
    }

    // Convert string argument to float
    char *endptr;
    float thrash_level = strtof(argv[1], &endptr);

    // Error handling: check if the conversion was successful
    if (*endptr != '\0') {
        printf("Invalid float input: %s\n", argv[1]);
        return 1;
    }
    // Allocate memory buffer larger than LLC
    volatile char *buffer = (volatile char *)aligned_alloc(BLOCK_SIZE, THRASH_SIZE*1.5);
    if (!buffer)
    {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    WINDOW_CYCLES = 100000 * (count_window_cycles() / 100000);
    printf("Thrashing LLC...\n");
    short bits[5] = {0, 1, 3, 4, 2};
    short i = 0, t_level=0;
    // Continuous LLC thrashing loop
    while (1)
    {
        size_t start_window = rdtsc();
        do
        {
            switch (bits[i])
            {
            case 0:
                t_level = 0.15;
                break;
            case 1:
                t_level = 0.5;
                break;
            case 2:
                t_level = 0.75;
                break;
            case 3:
                t_level = 1.5;
                break;
            case 4:
                t_level = 0;
                break;
            }
            thrash_cache(buffer, thrash_level);

        } while (rdtsc() - start_window < WINDOW_CYCLES);
    }
    printf("SENDER end\n");

    free((void *)buffer);
    return 0;
}
