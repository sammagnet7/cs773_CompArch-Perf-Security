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

size_t CHUNKS = 0;
char *received_messages[MAX_CHUNKS] = {0};
uint32_t *received_chunks[MAX_CHUNKS] = {0};

char *reconstruct(uint32_t *received_bits, size_t length)
{
    size_t num_chars = length * 4;
    char *output = (char *)malloc(num_chars + 1);
    if (!output)
    {
        printf("unable to allocate buffer on heap");
        return NULL;
    }
    num_chars = 0;
    char c[4];
    for (size_t i = 0; i < length; i++)
    {
        uint32_t value = received_bits[i];
        if (value == 0x0000)
            continue;
        // ++num_chars;
        // printf("%08x;", value);
        c[0] = (value >> 24) & 0xFF;
        c[1] = (value >> 16) & 0xFF;
        c[2] = (value >> 8) & 0xFF;
        c[3] = value & 0xFF;
        for (int j = 0; j < 4; j++)
        {
            if (c[j] >= 32 && c[j] <= 126)
            {
                output[num_chars++] = c[j];
                // printf(".%c", c[j]);
            }
        }
    }
    // printf("\n");

    output[num_chars] = '\0';
    // printf("recon %s", output);
    return output;
}

void append_to_file(const char *filename, const char *message)
{
    FILE *file = fopen(filename, "a");
    if (!file)
    {
        perror("fopen");
        return;
    }
    fprintf(file, "%s", message);
    fclose(file);
}

void clear_file(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("fopen");
    }
    fclose(file);
}

void cleanup()
{
    clear_file("received.txt");

    for (int i = 0; i <= CHUNKS; i++)
    {
        if (received_messages[i])
        {
            printf("\nchunk %d : %s", i, received_messages[i]);
            append_to_file("received.txt", received_messages[i]);
            free(received_messages[i]);
        }
    }

    close_transmit();
    exit(0);
}

void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        cleanup();
    }
}
size_t ronlyreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
bool rdetect_bit()
{
    size_t hit = 0, miss = 0, i = 0;
    size_t time = rdtsc();
    while (rdtsc() - time < SLOT_LENGTH)
    {
        size_t reload_time = ronlyreload((void *)(base + i));
        i = (i + 64) % SHARED_ARRAY_SIZE; // bytes in shared text file in nearest multiple of 64        // max_rld = MAX(max_rld, reload_time);
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
    // printf("R:: miss: %d hit: %d bit: %d\n", miss, hit, miss >= hit);
    return miss >= hit;
}

int main()
{
    signal(SIGINT, signal_handler);

    open_transmit(); // opening shared file

    // calibrate();
    // exit(0);

    CHUNKS = 0;

    // for (int i = 0; i < MAX_CHUNKS; i++)
    //     received_messages[i] = 0;
    // // ull sync_ts;
    // uint32_t *received_chunks[MAX_CHUNKS] = {0};
    // CHUNKS = receive_data(received_chunks);

    int size = 0;
    char dump[25000];
    while (1)
    {
        int bit = rdetect_bit((__uint64_t)base);
        dump[size++] = bit + '0';
        if (size == 25000)
        {
            dump[size] = '\0';
            printf("%s\n", dump);
            break;
        }
    }
    // printf("chunks %ld\n", CHUNKS);
    // if (CHUNKS <= 0)
    // {
    //     printf("error in receiving");
    //     exit(1);
    // }
    // for (int i = 0; i < MAX_CHUNKS; i++)
    // {
    //     if (!received_chunks[i])
    //         continue;
    //     received_messages[i] =
    //         reconstruct(received_chunks[i], MESSAGE_CHUNK_LEN / 32);
    //     free(received_chunks[i]);
    // }

    cleanup();
}