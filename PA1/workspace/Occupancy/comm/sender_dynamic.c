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
#define MAX_SEND_BUFFER_SIZE 4096

short send_buffer[MAX_SEND_BUFFER_SIZE];
int main(int argc, char *argv[])
{
    short actual_send_buffer_size = argc - 1;
    if (actual_send_buffer_size > MAX_SEND_BUFFER_SIZE) {
        printf("Error: Too many arguments! Maximum allowed is %d.\n", MAX_SEND_BUFFER_SIZE);
        return 0;
    }
    // Convert command-line arguments to integers
    for (int i = 0; i < actual_send_buffer_size; i++) {
        send_buffer[i] = atoi(argv[i + 1]);  // Convert string to integer
    } 

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
    // hello world! I love cs773
    short default_str[100] = {1,2,2,0,1,2,1,1,1,2,3,0,1,2,3,0,1,2,3,3,0,2,0,0,1,3,1,3,1,2,3,3,1,3,0,2,1,2,3,0,1,2,1,0,0,2,0,1,0,2,0,0,1,0,2,1,0,2,0,0,1,2,3,0,1,2,3,3,1,3,1,2,1,2,1,1,0,2,0,0,1,2,0,3,1,3,0,3,0,3,1,3,0,3,1,3,0,3,0,3};

    while (i < actual_send_buffer_size)
    {
        size_t start_window = rdtsc();
        do
        {
            switch (send_buffer[i])
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
