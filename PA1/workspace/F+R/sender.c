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
#define CHUNK_SIZE 512

size_t read_bool_file(const char *filename, bool *bits)
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

    for (int i = 0; i < 3; i++)
    {
        ssend_bit(1);
        ssend_bit(0);
    }
}
int main()
{
    open_transmit(); // opens the shared file
    bool bit_stream[MAX_LIMIT_BOOL];
    size_t bits_len = read_bool_file("msg.txt", bit_stream);
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
