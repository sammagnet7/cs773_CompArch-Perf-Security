#ifndef __RCV__H__
#define __RCV__H__
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "cacheutils.h"
#include "sys_config.h"

#define OUTPUT_FILE "received.txt"
#define DEBUG_LOG_FILE "log.txt"
#define MAX_DATA_COLLECTION_ROUNDS 200
#define SAMPLE_SIZE (MAX_DATA_COLLECTION_ROUNDS * 5)

#define WINDOW_TIME 50 * 1000 // 200 ms
#define SLEEP_TIME 50 * 1000  // 200 ms
#define MINI_ROUNDS 6

int collected_data[SAMPLE_SIZE];

int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// Function to compute the median of 4 elements
long compute_median(long arr[], int size)
{
    qsort(arr, size, sizeof(long), compare);
    return arr[2];
}
void handle_sigterm(int sig)
{
    printf("Received SIGTERM. Writing received data to file and exiting...\n");
    FILE *log;
    FILE *file = fopen(OUTPUT_FILE, "w");
    #ifdef DEBUG_LOG
    
        log = fopen(DEBUG_LOG_FILE, "w");
        if (!log)
        {
            perror("Log file opening failed");
            exit(0);
        }
    #endif
    if (!file)
    {
        perror("Data file opening failed");
        exit(0);
    }

    // Write the entire array to the file
    // Process elements in groups of 4
    for (int i = 0; i < SAMPLE_SIZE; i += 5)
    {
        if (i + 5 > SAMPLE_SIZE)
            break; // Avoid out-of-bounds access

        long temp[5] = {collected_data[i], collected_data[i + 1], collected_data[i + 2], collected_data[i + 3], collected_data[i + 4]};
        #ifdef DEBUG_LOG
        fprintf(log, "[%ld %ld %ld %ld %ld]: ", temp[0], temp[1], temp[2], temp[3], temp[4]);
        #endif
        long median = compute_median(temp, 5);
        #ifdef DEBUG_LOG
        fprintf(log, "%ld\n", median);
        #endif
        fprintf(file, "%ld ", median);
    }
    fprintf(file, "\n");
    
    fclose(file);
    #ifdef DEBUG_LOG
    fclose(log);
    #endif
    exit(0);
}

#endif