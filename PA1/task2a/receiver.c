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
char *base;
void *probe;

bool __detect_bit() {
    size_t time = rdtsc();
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
    return miss >= hit;
}

bool _detect_bit() {
    int count = 0;
    for (int i = 0; i < BIT_REPEAT; i++) {
        if (__detect_bit()) {
            count++;
        }
    }
    return count > (BIT_REPEAT / 2);
}

bool detect_bit() {
    static int future = -1;
    int curr;
    if (future == -1)
        future = _detect_bit();
    curr = future;
    future = _detect_bit();
    if (curr == 1 && future == 0) {
        future = -1;
        return detect_bit();
    }
    return curr;
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

void receive_message(char **received_message) {
    size_t len = 0, allen = 0, msg_len = 1000;
    size_t try = 0, maxtry = 1000;
    bool received[msg_len], all_recieved[maxtry];
    int prev = 0, freq_10 = 0, curr;
    while (freq_10 < 3 && ++try < maxtry) {
        curr = _detect_bit(base);
        all_recieved[allen++] = curr;
        if (prev == 1 && curr == 0)
            freq_10 += 1;
        prev = curr;
    }
    // printf("R start: %ld\n", start);
    // printf("rBits: ");
    // len = 0;
    // while (len < allen)
    //     printf("%d", all_recieved[len++]);
    // printf("\n");
    if (freq_10 < 3) {
        // printf("Error: Sequence not found, freq_10: %d, tries: %zu\n",
        // freq_10,               try);
        *received_message = NULL;
        return;
    }
    len = 0;
    while (len < msg_len) {
        curr = detect_bit(base);
        received[len++] = curr;
        prev = curr;
    }
    // printf("rbits: ");
    // len = 0;
    // while (len < msg_len)
    //     printf("%d", received[len++]);
    // printf("\n");
    *received_message = reconstruct(received, msg_len);
    // printf("received message: %s\n", received_message);
    // for (int i = 0; received_message[i] != '\0'; i++)
    //     printf("%d%c ", received_message[i], received_message[i]);
    // printf("\n");
    // printf("R start: %ld\n", start);
}

int main() {
    int fd = open("./libshared.so", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    // making the map_size page-aligned i.e. a multiple of 4 KB page
    // in order to grant the address space using mmap
    if (st.st_size & 0xFFF != 0) {
        st.st_size |= 0xFFF;
        st.st_size += 1;
    }

    base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    probe = base + 0x4020;
    if (base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    char *receive_messages[ROUNDS] = {0};
    for (int round = 0; round < ROUNDS; round++) {
        start_sync();
        receive_message(&receive_messages[round]);
        usleep(1000);
    }

    for (int round = 0; round < ROUNDS; round++) {
        if (receive_messages[round]) {
            printf("m%d: %s\n", round, receive_messages[round]);
        } else {
            printf("m%d: NULL\n", round);
        }
    }

    for (int round = 0; round < ROUNDS; round++) {
        free(receive_messages[round]);
    }

    if (munmap(base, st.st_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    close(fd);

    return 0;
}