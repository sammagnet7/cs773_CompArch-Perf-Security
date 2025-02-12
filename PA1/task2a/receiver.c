#include "cacheutils.h"
#include "common.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

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

void calibrate(char *base) {
    size_t start = rdtsc();
    size_t freq = 0, max_cycle = 0, sum_cycle = 0;
    void *probe = base + 0x4020;
    while (rdtsc() - start < SLOT_LENGTH) {
        size_t reload_time = onlyreload(probe);
        freq++;
        max_cycle = max(max_cycle, reload_time);
        sum_cycle += reload_time;
    }
    size_t avg_cycle = sum_cycle / freq;
    printf("freq: %ld avg: %ld max: %ld thresh %ld\n", freq, avg_cycle,
           max_cycle, avg_cycle);
    exit(0);
}

bool detect_bit(char *base) {
    size_t time = rdtsc();
    void *probe = base + 0x4020;
    size_t hit = 0, miss = 0, max_rld = 0;
    while (rdtsc() - time < SLOT_LENGTH) {
        size_t reload_time = onlyreload(probe);
        max_rld = max(max_rld, reload_time);
        if (reload_time > MIN_CACHE_MISS_CYCLES) {
            miss++;
        } else {
            hit++;
        }
    }
    // printf("\nreceiver-slot end %ld hit %ld miss %ld max:%ld\n", rdtsc() -
    // time, hit,
    //       miss, max_rld);
    return miss >= hit;
}

char *reconstruct(bool *received_bits, size_t length) {
    size_t num_chars = length / ASCII_BITS;
    char *output = (char *)malloc(num_chars + 1);
    if (!output)
        return NULL;
    for (size_t i = 0; i < num_chars; i++) {
        char c = 0;
        for (int j = 0; j < ASCII_BITS; j++) {
            c = (c << 1) | received_bits[i * ASCII_BITS + j];
        }
        output[i] = c;
    }
    output[num_chars] = '\0'; // Null terminate the string
    return output;
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
    char *base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

    // calibrate(base);

    size_t len = 0, msg_len = 200;
    size_t allen = 0;
    size_t try = 0, maxtry = 1000;
    bool received[msg_len], all_recieved[msg_len + maxtry];
    int prev = 0, freq_10 = 0, curr;
    ull start = start_sync();
    while (freq_10 < 3 && ++try < maxtry) {
        curr = detect_bit(base);
        all_recieved[allen++] = curr;
        if (prev == 1 && curr == 0)
            freq_10 += 1;
        prev = curr;
    }
    printf("R start: %ld\n", start);
    // printf("rBits: ");
    // len = 0;
    // while (len < allen)
    //     printf("%d", all_recieved[len++]);
    // printf("\n");
    if (freq_10 < 3) {
        printf("seq not found freq_10: %d trys %d \n", freq_10, try);
        exit(1);
    }
    len = 0;
    while (len < msg_len) {
        curr = detect_bit(base);
        // strip all '10's
        if (prev == 1 && curr == 0)
            --len;
        else
            received[len++] = curr;
        prev = curr;
    }
    printf("rbits: ");
    len = 0;
    while (len < msg_len)
        printf("%d", received[len++]);
    printf("\n");
    char *received_message = reconstruct(received, msg_len);
    printf("received message: %s\n", received_message);
    for (int i = 0; received_message[i] != '\0'; i++)
        printf("%d%c ", received_message[i], received_message[i]);
    printf("\n");
    printf("R start: %ld\n", start);

    return 0;
}
