// #include "cacheutils.h"
#include "include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "include/utils.h"

#define MAX_LIMIT_BOOL 1000 * 1024 * 8 // 250 KB
#define DEAD_DROP_START_INDEX 0x1fff
#define DEAD_DROP_END_INDEX 0x2fff
#define DEAD_DROP_SIZE (DEAD_DROP_END_INDEX - DEAD_DROP_START_INDEX)
size_t read_bool_file(const char *filename, uint8_t *bits)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("fopen");
        return 0;
    }

    size_t bit_count = 0;
    uint8_t byte;

    while (fread(&byte, sizeof(uint8_t), 1, file) == 1)
    {
        for (int i = 0; i < 8; i++)
        {
            if (bit_count >= MAX_LIMIT_BOOL)
            {
                perror("bits length longer than limit");
                break;
            }
            bits[bit_count++] = (byte & (1 << (7 - i))) != 0;
        }
    }

    fclose(file);
    return bit_count;
}

void ssend_bit(int bit)
{
    size_t start = rdtsc(), index = 0;
    if (bit)
        while (rdtsc() - start < SLOT_LENGTH)
        {
            flush((void *)(base + index));
            index = (index + 64) % SHARED_ARRAY_SIZE;
        }
    else
        while (rdtsc() - start < SLOT_LENGTH)
        {
        }
}
void sync_chunk()
{
    // start_sync();
    // send magic sequence
    uint32_t pattern = MAGIC_SEQUENCE;
    printf("S-SYNC START\n");
    for (int i = MAGIC_SEQ_LEN - 1; i >= 0; i--)
    {
        int bit = (pattern >> i) & 1;
        ssend_bit(bit);
    }
    printf("S-SYNC SENT\n");
}
size_t ronlyreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
int check_acknowledgement()
{
    size_t progress = DEAD_DROP_START_INDEX;
    size_t hit = 0, miss = 0;
    while (progress < DEAD_DROP_END_INDEX)
    {
        size_t reload_time = ronlyreload((void *)(base + progress));
        printf("Progress: %d Reload time %ld\n", progress, reload_time);
        if (reload_time > MIN_CACHE_MISS_CYCLES)
        {
            miss++;
        }
        else
        {
            hit++;
        }
        progress += 64;
    }
    printf("Hit: %ld, Miss: %ld\n", hit, miss);
    return miss > hit;
}
int main()
{
    open_transmit("dump.txt"); // opens the shared file
    size_t bit_index = 0;
    uint8_t bit_stream[MAX_LIMIT_BOOL];
    size_t bits_len = read_bool_file("dump.txt", bit_stream);

    while (bit_index < bits_len)
    {
        // After sending a chunk, check for acknowledgement
        if (bit_index != 0 && bit_index % CHUNK_SIZE == 0)
        {
            if (check_acknowledgement()) // if ack received, move to next chunk
            {
                bit_index++;
            }
            else
            {
                bit_index = bit_index - CHUNK_SIZE; // resend the chunk
            }
        }
        // Send preamble at Chunk boundary
        if (bit_index % CHUNK_SIZE == 0)
        {
            sync_chunk();
        }

        // send each bit in the chunk
        ssend_bit(bit_stream[bit_index]);
        bit_index++;
    }
    // free(payload);
    close_transmit();
    printf("Transmission complete\n");
    return 0;
}