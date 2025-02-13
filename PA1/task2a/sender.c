#include "cacheutils.h"
#include "common.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

size_t flushandreload(void *addr) {
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    flush(addr);
    return delta;
}

size_t onlyreload(void *addr) {
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}

char *base, *probe;

void __send_bit(int value) {
    size_t start = rdtsc();
    if (value)
        while (rdtsc() - start < SLOT_LENGTH) {
            flush((void *)probe);
        }
    else
        while (rdtsc() - start < SLOT_LENGTH) {
        }
}
void _send_bit(int value) {
    for (int i = 0; i < BIT_REPEAT; i++)
        __send_bit(value);
}
void send_bit(int value) {
    static int prev_value = -1;
    if (prev_value == 1 && value == 0) {
        _send_bit(1);
        _send_bit(0);
    }
    _send_bit(value);
    prev_value = value;
}

void send_magic_seq() {
    for (int i = 0; i < START_SEQ_LEN; i++) {
        _send_bit(1);
        _send_bit(0);
    }
}

void send_string(const char *str) {
    send_magic_seq();
    while (*str) {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--) {
            send_bit((c >> i) & 1);
        }
    }
}

void print_string(const char *str) {
    printf("sbits: ");
    // for (int i = 0; i < START_SEQ_LEN; i++) {
    //     printf("1");
    //     printf("0");
    // }
    int prev_bit = -1, curr_bit = -1;
    while (*str) {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--) {
            curr_bit = (c >> i) & 1;
            // if (prev_bit == 1 && curr_bit == 0) {
            //     printf("1");
            //     printf("0");
            // }
            printf("%d", curr_bit);
            prev_bit = curr_bit;
        }
    }
    printf("\n");
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *content = (char *)malloc(length + 1);
    if (!content) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    // Read the file content into the allocated memory
    fread(content, 1, length, file);
    content[length] = '\0'; // Null-terminate the string

    fclose(file);
    return content;
}

int main() {
    int fd = open("./libshared.so", O_RDONLY);
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        return EXIT_FAILURE;
    }

    // making the map_size page-aligned i.e. a multiple of 4 KB page
    // in order to grant the address space using mmap
    if (st.st_size & 0xFFF != 0) {
        st.st_size |= 0xFFF;
        st.st_size += 1;
    }
    // Memory map the shared library
    base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    probe = base + 0x4020;
    char *test_string = read_file("msg.txt");
    if (!test_string) {
        return EXIT_FAILURE;
    }
    size_t length = strlen(test_string);
    size_t batch_size = 100, batches = 0;
    char batch[batch_size + 1];
    for (size_t i = 0; i < length; i += batch_size) {
        strncpy(batch, test_string + i, batch_size);
        start_sync();
        batch[batch_size] = '\0'; // Null-terminate the batch
        send_string(batch);
        // usleep(10000);
        batches++;
        // printf("\n");
    }
    // printf("S start: %ld\n", start);
    // print_string(test_string);
    // printf("S start: %ld\n", start);
    printf("Sent content in %d batches", batches);
    free(test_string);

    return 0;
}
