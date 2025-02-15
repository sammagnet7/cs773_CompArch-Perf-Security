#include "utils.h"
#include "common.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int fd;
char *base = NULL;
struct stat st;
char *SHARED_FILE_NAME;
int open_transmit(char *shared_file)
{
    fd = open(shared_file, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    // making the map_size page-aligned i.e. a multiple of 4 KB page
    // in order to grant the address space using mmap
    if ((st.st_size & 0xFFF) != 0)
    {
        st.st_size |= 0xFFF;
        st.st_size += 1;
    }
    base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }
    SHARED_FILE_NAME = shared_file;
    return EXIT_SUCCESS;
}

void close_transmit()
{
    if (base != MAP_FAILED)
        munmap(base, st.st_size);

    close(fd);
}

ull start_sync()
{
    ull ts;
    while (((ts = rdtsc()) & TIME_MASK) > SYNC_INTERVAL)
    {
    }
    return ts;
}

void dumps_uint32_bits(uint32_t *bits, size_t length,
                       const char *out_file_name)
{
    FILE *file = fopen(out_file_name, "wb");
    if (!file)
    {
        perror("fopen");
        return;
    }

    size_t written = fwrite(bits, sizeof(uint32_t), length, file);
    if (written != length)
    {
        perror("fwrite");
    }

    fclose(file);
}
void dumps_bool_bits(bool *bits, size_t length, const char *out_file_name)
{
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

void print_bits(bool *bits, size_t bits_len)
{
    printf("bitslen: %ld\n", bits_len);
    size_t num_byte = 0;
    for (size_t i = 0; i < bits_len; i += 8)
    {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && (i + j) < bits_len; j++)
        {
            if (bits[i + j])
            {
                byte |= (1 << (7 - j));
            }
        }
        printf("%02X", byte);
        ++num_byte;
        if (num_byte % 2 == 0)
            printf(" ");
    }
    printf("\n");
}