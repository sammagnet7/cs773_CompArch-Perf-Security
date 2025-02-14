#include "receiver.h"

volatile char **buckets;
long long WINDOW_LEN;
size_t WINDOW_CYCLES;
long SLEEP_CYCLE;
void handle_sigterm(int sig);

size_t count_window_cycles(long win_time)
{
    size_t end;
    size_t start = rdtsc();
    usleep(win_time);
    end = rdtsc();
    return end - start;
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, handle_sigterm); // Catch SIGTERM
    buckets = (volatile char **)malloc(NUM_CHUNKS * sizeof(volatile char *));
    for (size_t i = 0; i < NUM_CHUNKS; i++)
    {
        buckets[i] = (volatile char *)malloc(CHUNK_SIZE);
    }

    WINDOW_CYCLES = 100000 * (count_window_cycles(WINDOW_TIME) / 100000);

    printf("WINDOW_CYCLE: %ld\n", WINDOW_CYCLES);
    long g_counter, mini_round = 0;

    short data_collection_rounds = 0, index = 0;
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
    }

    FILE *file = fopen(OUTPUT_FILE, "w");

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
        long median = compute_median(temp, 5);
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
