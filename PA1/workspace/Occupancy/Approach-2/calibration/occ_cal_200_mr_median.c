#include "sys_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile char **buckets;

#define WINDOW_TIME 50 * 1000 // 50 ms
#define SLEEP_TIME 50 * 1000   // 200 ms
#define SAMPLING_ROUNDS 500
#define SAMPLE_SIZE (SAMPLING_ROUNDS*3)

long SLEEP_CYCLE;
size_t WINDOW_CYCLES;

unsigned long long rdtsc()
{
    unsigned long long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

size_t count_window_cycles(long win_time)
{
    size_t end;
    size_t start = rdtsc();
    usleep(win_time);
    end = rdtsc();
    return end - start;
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Function to compute the median of 4 elements
long compute_median(long arr[], int size) {
    qsort(arr, size, sizeof(long), compare);
    return (arr[1] + arr[2]) / 2;  // Median of 4 sorted elements
}

int main()
{
    // -------------------------- Array allocate -------------------------------------------
    buckets = (volatile char **)malloc(NUM_CHUNKS * sizeof(volatile char *));
    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        buckets[i] = (volatile char *)malloc(CHUNK_SIZE);
    }
    long category_samples[SAMPLE_SIZE];
    // -------------------------------------------------------------------------------------
    // -------------------------- Init vars ------------------------------------------------

    size_t NUM_LINES = CHUNK_SIZE / BLOCK_SIZE;
    WINDOW_CYCLES = 100000 * (count_window_cycles(WINDOW_TIME) / 100000);
    SLEEP_CYCLE = 100000 * (count_window_cycles(SLEEP_TIME) / 100000);
    long g_counter;
    size_t thrash_rounds = 0, chunk, i, start_window, progress, mini_round, index = 0;
    // --------------------------- Data Collection loop ------------------------------------
    while (thrash_rounds < SAMPLING_ROUNDS)
    {
        chunk = progress = mini_round = 0;
        while (mini_round < 4)
        {
            chunk = g_counter = 0;
            start_window = rdtsc();
            while (chunk < NUM_CHUNKS)
            {
                i = 0;
                while (i < NUM_LINES)
                {
                    buckets[chunk][i * BLOCK_SIZE] += 1; // Touch each cache line
                    i++;
                }
                if (rdtsc() - start_window > WINDOW_CYCLES)
                {
                    break;
                }
                chunk = (chunk + 1) % NUM_CHUNKS;
                // increment counter
                g_counter++;
            }
            if(mini_round != 0)
            category_samples[index++] = g_counter;
            mini_round++;
        }
 
        thrash_rounds++;
    }
    // Open file in binary mode for writing
    FILE *file = fopen("output.txt", "a+");
    if (!file)
    {
        perror("File opening failed");
        return 1;
    }

    // Process elements in groups of 4
    for (int i = 0; i < SAMPLE_SIZE; i += 3) {
        if (i + 3 > SAMPLE_SIZE) break;  // Avoid out-of-bounds access

        long temp[3] = {category_samples[i], category_samples[i+1], category_samples[i+2]};
        long median = compute_median(temp, 3);

        fprintf(file, "%ld ", median);
    }

    fprintf(file, "\n");
    fclose(file);

    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        free((void *)buckets[i]);
    }
    free((void *)buckets);
    return 0;
}
