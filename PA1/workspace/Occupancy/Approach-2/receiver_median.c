#include "cacheutils.h"
#include "sys_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile char **buckets;
size_t counter;

long long WINDOW_LEN;         // will be calibrated
#define WINDOW_TIME 50 * 1000 // 200 ms
#define SLEEP_TIME 50 * 1000  // 200 ms
#define MINI_ROUNDS 6
size_t WINDOW_CYCLES;

int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// Function to compute the median of 4 elements
long compute_median(long arr[], int size)
{
    qsort(arr, size, sizeof(long), compare);
    return  arr[2]; 
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

size_t count_window_cycles(long win_time)
{
    size_t end;
    size_t start = rdtsc();
    usleep(win_time);
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
typedef struct
{
    int category, lower, upper;
} Range;

// Function to classify the sample point
int classifySample(int sample, const Range *ranges, int size)
{
    int closestCategory = -1, minDiff = __INT_MAX__;

    for (int i = 0; i < size; i++)
    {
        if (sample >= ranges[i].lower && sample <= ranges[i].upper)
            return ranges[i].category; // Sample is within range

        int diff = sample < ranges[i].lower ? ranges[i].lower - sample : sample - ranges[i].upper;
        if (diff < minDiff)
            minDiff = diff, closestCategory = ranges[i].category;
    }
    return closestCategory;
}

#define MAX_DATA_COLLECTION_ROUNDS 200
#define SAMPLE_SIZE (MAX_DATA_COLLECTION_ROUNDS * 5)
long SLEEP_CYCLE;
int main(int argc, char *argv[])
{
    buckets = (volatile char **)malloc(NUM_CHUNKS * sizeof(volatile char *));
    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        buckets[i] = (volatile char *)malloc(CHUNK_SIZE);
    }
    volatile char vol_var;

    WINDOW_CYCLES = 100000 * (count_window_cycles(WINDOW_TIME) / 100000);
    SLEEP_CYCLE = 100000 * (count_window_cycles(SLEEP_TIME) / 100000);

    printf("WINDOW_CYCLE: %ld\n", WINDOW_CYCLES);
    long g_counter, mini_round = 0;
    size_t NUM_LINES = CHUNK_SIZE / BLOCK_SIZE;
    short nil, bin_00, bin_01, bin_10, bin_11;
    size_t thresh_rounds = 0;
    short data_collection_rounds = 0, index = 0;
    int collected_data[SAMPLE_SIZE];
    // Time whole access

    size_t chunk, i, start_window, progress;
    while (data_collection_rounds++ < MAX_DATA_COLLECTION_ROUNDS)
    {
        chunk = progress = mini_round = 0;
        while (mini_round < MINI_ROUNDS)
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
            if (mini_round != 0)
            collected_data[index++] = g_counter;
            mini_round++;
        }
        // Write to array

        // printf("%ld\n", g_counter);
    }

    // Open file in binary mode for writing
    FILE *file = fopen("input.txt", "w");
    FILE *log = fopen("log.txt", "w");
    if (!file)
    {
        perror("File opening failed");
        return 1;
    }

    // Write the entire array to the file
    // Process elements in groups of 4
    for (int i = 0; i < SAMPLE_SIZE; i += 5)
    {
        if (i + 5 > SAMPLE_SIZE)
            break; // Avoid out-of-bounds access

        long temp[5] = {collected_data[i], collected_data[i + 1], collected_data[i + 2], collected_data[i + 3], collected_data[i + 4]};
        fprintf(log, "[%ld %ld %ld %ld %ld]: ",temp[0],temp[1],temp[2], temp[3], temp[4]);
        long median = compute_median(temp, 5);
        fprintf(log, "%ld\n", median);
        fprintf(file, "%ld ", median);
    }
    fprintf(file, "\n");
    fclose(file);
    fclose(log);

    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        free((void *)buckets[i]);
    }
    free((void *)buckets);
    return 0;
}
