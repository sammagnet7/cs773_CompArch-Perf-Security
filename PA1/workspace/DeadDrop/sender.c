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
#define DEAD_DROP_START_INDEX 0x6fff
#define DEAD_DROP_END_INDEX 0xAfff
#define DEAD_DROP_SIZE (DEAD_DROP_END_INDEX - DEAD_DROP_START_INDEX)
size_t bit_index = 0;
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
//	printf("%d\n", bit);
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

void sync_preamble()
{
    // start_sync();
    // send magic sequence
    uint32_t pattern = MAGIC_PREAMBLE;
    for (int i = MAGIC_SEQ_LEN - 1; i >= 0; i--)
    {
        int bit = (pattern >> i) & 1;
        ssend_bit(bit);
    }
    // printf("S-SYNC PREAMBLE: ID: %ld\n", bit_index);
}
void sync_postamble()
{
    uint32_t pattern = MAGIC_POSTAMBLE;
    for (int i = MAGIC_SEQ_LEN - 1; i >= 0; i--)
    {
        int bit = (pattern >> i) & 1;
        ssend_bit(bit);
    }
    // printf("S-SYNC POSTAMBLE: ID: %ld\n", bit_index);
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
        //printf("Progress: %d Reload time %ld\n", progress, reload_time);
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
    //printf("Hit: %ld, Miss: %ld\n", hit, miss);
    return miss > hit;
}
int main()
{
    open_transmit("dump.txt"); // opens the shared file
    uint8_t bit_stream[MAX_LIMIT_BOOL] = {0};
    size_t bits_len = read_bool_file("processed.bin", bit_stream);
    uint32_t pattern = MAGIC_POSTAMBLE;
    size_t num_chunks = bits_len/CHUNK_SIZE;
    while (bit_index < bits_len){
//	check_acknowledgement();
        // Send preamble at Chunk boundary
        sync_preamble();

        for (int i = 0; i < CHUNK_SIZE; i++)
        {
            ssend_bit(bit_stream[bit_index + i]); // send each bit in the chunk
        }

        // Send postamble at Chunk boundary
	
    	for (int i = MAGIC_SEQ_LEN - 1; i >= 0; i--)
    	{
        	int bit = (pattern >> i) & 1;
        	ssend_bit(bit);
    	}
    	// printf("S-SYNC POSTAMBLE: ID: %ld\n", bit_index);
        size_t start = rdtsc();
	while(rdtsc()-start<ACK_THRESHOLD*1000){};
	if (check_acknowledgement()) // if ack received, move to next chunk
        {
	    // printf("Got ACK\n");
            bit_index += CHUNK_SIZE;
        }
	printf("CHUNK %d/%d\r", bit_index/CHUNK_SIZE, num_chunks);
	fflush(stdout);
    }
    // free(payload);
    close_transmit();
    // printf("Transmission complete\n");
    return 0;
}
