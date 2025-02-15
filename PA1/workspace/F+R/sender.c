// #include "cacheutils.h"
#include "common.h"
#include "transmit_layer/transmit_layer.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_LIMIT_BOOL 1000 * 1024 * 8 // 250 KB

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
size_t global_count = 0;
void sync_chunk()
{
    // start_sync();
    // send magic sequence
    uint32_t pattern = MAGIC_SEQUENCE;
    //printf("%ld S-SYNC START\n", global_count);
    for (int i = MAGIC_SEQ_LEN - 1; i >= 0; i--)
    {
	int bit = (pattern >> i) & 1;
        ssend_bit(bit);
    }
	//printf("S-SYNC SENT\n");
}
int main()
{
    open_transmit(); // opens the shared file
    uint8_t bit_stream[MAX_LIMIT_BOOL];
    size_t bits_len = read_bool_file("processed.bin", bit_stream);
    // for (int i = 0; i < bits_len; i++)
    // {
    //     printf("%d", bit_stream[i]);
    // }
    size_t bit_index = 0;

    while (bit_index < bits_len)
    {

        // need to sync here
        if (bit_index % CHUNK_SIZE == 0)
        {
            sync_chunk();
        }
        // start_sync();
        ssend_bit(bit_stream[bit_index]);
        bit_index++;
    }

    // free(payload);
    close_transmit();
    printf("Transmission complete\n");
    return 0;
}
