#include "common.h"
#include "transmit_layer/transmit_layer.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_RECEIVED_BITS_SIZE 1000 * 1024 * 8
size_t CHUNKS = 0;
size_t received_till_now = 0;
int received_bits[MAX_RECEIVED_BITS_SIZE];
#define OUTPUT_FILE "received.txt"
void dump_bits(int *bits, size_t length, const char *out_file_name)
{
    printf("Dumping bits to file %s\n", out_file_name);
    FILE *file = fopen(out_file_name, "wb");
    if (!file)
    {
        perror("fopen");
        return;
    }

    uint8_t byte = 0;
    size_t bit_count = 0;

    for (size_t i = 0; i < length; i++)
    {
        if (bits[i])
        {
            byte |= (1 << (7 - bit_count));
        }
        bit_count++;

        if (bit_count == 8)
        {
            fwrite(&byte, sizeof(uint8_t), 1, file);
            byte = 0;
            bit_count = 0;
        }
    }
    if (bit_count > 0)
    {
        fwrite(&byte, sizeof(uint8_t), 1, file);
    }

    fclose(file);
}

size_t ronlyreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
int rdetect_bit()
{
    size_t hit = 0, miss = 0, i = 0;
    size_t time = rdtsc();
    while (rdtsc() - time < SLOT_LENGTH)
    {
        size_t reload_time = ronlyreload((void *)(base + i));
        i = (i + 64) % SHARED_ARRAY_SIZE; // bytes in shared text file in nearest multiple of 64
        if (reload_time > MIN_CACHE_MISS_CYCLES)
        {
            miss++;
        }
        else
        {
            hit++;
        }
        i++;
    }
    return miss >= hit;
}
void sigint_handler(int signum)
{
    dump_bits(received_bits, received_till_now, OUTPUT_FILE);
    exit(0);
}
void sync_chunk()
{
    size_t maxtry = 1000, try = 0;
    int curr;
    unsigned char pattern_so_far = 0;
    unsigned char pattern = 0b101010;
    // start_sync();
    // wait for magic sequence
    while (pattern_so_far != pattern)
    {
        curr = rdetect_bit((__uint64_t)base);
        pattern_so_far = (pattern_so_far << 1) | curr;
    }
}

int main()
{
    signal(SIGINT, sigint_handler);
    open_transmit(); // opening shared file

    while (1)
    {
        if (received_till_now % CHUNK_SIZE == 0)
        {
            sync_chunk();
        }
        int bit = rdetect_bit((__uint64_t)base);
        received_bits[received_till_now] = bit;
        received_till_now++;
        // if (received_till_now == MAX_RECEIVED_BITS_SIZE)
        // {
        //     dump_bits(received_bits, received_till_now, OUTPUT_FILE);
        //     break;
        // }
    }
}
