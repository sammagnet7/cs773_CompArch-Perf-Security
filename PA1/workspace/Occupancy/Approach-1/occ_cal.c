#include "sys_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile char **buckets;
size_t counter;

long long WINDOW_LEN;          // will be calibrated
#define WINDOW_TIME 200 * 1000 // 200 ms
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

void calibrate()
{
    size_t start, end, total_cycles;
    int k = 0;
    while (++k <= CALIBRATION_ROUNDS)
    {
        start = rdtsc();
        // occupy_cache();
        ++counter;
        end = rdtsc();
        total_cycles += end - start;
    }
    long long avg_cycles_to_access_LLC = (total_cycles / CALIBRATION_ROUNDS);

    WINDOW_LEN = avg_cycles_to_access_LLC;
}

size_t count_window_cycles()
{
    size_t end;
    size_t start = rdtsc();
    usleep(WINDOW_TIME);
    end = rdtsc();
    return end - start;
}

void occupy_cache(size_t start)
{

    size_t chunk = 0, i = 0, l_counter = 0;
    size_t NUM_LINES = CHUNK_SIZE / BLOCK_SIZE;

    while (chunk < NUM_CHUNKS)
    {
        i = 0;

        while (i < NUM_LINES)
        {

            // if (rdtsc() - start > WINDOW_CYCLES){
            //     printf("local-counter %ld\n", l_counter);
            //     return;
            // }

            buckets[chunk][i * BLOCK_SIZE] += 1; // Touch each cache line

            i++;
        }
        // l_counter++;
        // chunk = (chunk + 1) % NUM_CHUNKS;
        chunk++;
    }
}
#define SAMPLING_ROUNDS 1000
#define NUM_CATEGORIES 5
#define LOWER_PERCENTILE 5  // 5th percentile
#define UPPER_PERCENTILE 95 // 95th percentile

// Structure for storing category ranges
typedef struct
{
    long lower;
    long upper;
} Range;

// Function to compare doubles for sorting
int compare(const void *a, const void *b)
{
    long diff = (*(long *)a - *(long *)b);
    return (diff > 0) - (diff < 0);
}

// Function to compute range based on percentiles
Range compute_category_range(long samples[SAMPLING_ROUNDS])
{
    // Sort the samples
    qsort(samples, SAMPLING_ROUNDS, sizeof(double), compare);

    // Compute indices for percentiles
    int lower_index = (LOWER_PERCENTILE / 100.0) * SAMPLING_ROUNDS;
    int upper_index = (UPPER_PERCENTILE / 100.0) * SAMPLING_ROUNDS;

    // Define range
    Range range;
    range.lower = samples[lower_index];
    range.upper = samples[upper_index];
    return range;
}

int main()
{
    buckets = (volatile char **)malloc(NUM_CHUNKS * sizeof(volatile char *));
    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        buckets[i] = (volatile char *)malloc(CHUNK_SIZE);
    }
    long category_samples[SAMPLING_ROUNDS];

    size_t NUM_LINES = CHUNK_SIZE / BLOCK_SIZE;
    WINDOW_CYCLES = 100000 * (count_window_cycles() / 100000);
    long g_counter;
    size_t thrash_rounds = 0;
    size_t chunk, i, start_window, progress, mini_round, index = 0;
    while (thrash_rounds < SAMPLING_ROUNDS)
    {

        chunk = progress = mini_round = 0;
            g_counter = 0;
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
            category_samples[index++] = g_counter;
        thrash_rounds++;
    }
    // Compute and print ranges
    Range category_ranges;
    category_ranges = compute_category_range(category_samples);
    // Open file in binary mode for writing
    FILE *file = fopen("output.txt", "a+");
    if (!file) {
        perror("File opening failed");
        return 1;
    }

    // Write the entire array to the file
    for (size_t i = 0; i < SAMPLING_ROUNDS; i++) {
        fprintf(file, "%ld ", category_samples[i]); // Write each integer in a new line
    }
    fprintf(file, "\n");
    fclose(file);

    printf("%ld %ld", category_ranges.lower, category_ranges.upper);

    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        free((void *)buckets[i]);
    }
    free((void *)buckets);
    return 0;
}
