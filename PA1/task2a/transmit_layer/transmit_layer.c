#include "transmit_layer.h"
#include "../common.h"
#include <fcntl.h>
// #include <math.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int fd;
char *base = NULL;
char *probe = NULL;
void *handle = NULL;

#define HISTORY_SIZE 160000
size_t __sent_counter = 0, __received_counter = 0;
bool __sent_bits[HISTORY_SIZE], __received_bits[HISTORY_SIZE];

// size_t flushandreload(void *addr) {
//     size_t time = rdtsc();
//     maccess(addr);
//     size_t delta = rdtsc() - time;
//     flush(addr);
//     return delta;
// }

size_t onlyflush(char *addr) {
    for (int i = 0; i < SHARED_ARRAY_SIZE; i += 8)
        flush((void *)(addr + i));
    return 0;
}

size_t onlyreload(char *addr) {
    size_t delta = 0, time;
    for (int i = 0; i < SHARED_ARRAY_SIZE; i += 8) {
        time = rdtsc();
        maccess((void *)(addr + i));
        delta += rdtsc() - time;
    }
    return (delta / SHARED_ARRAY_SIZE);
}

int open_transmit() {
    __sent_counter = 0;
    __received_counter = 0;
    fd = open("./libshared.so", O_RDONLY);
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
    if ((st.st_size & 0xFFF) != 0) {
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
    if (munmap(base, st.st_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void save_history(char *filename, bool *history, size_t length) {
    if (length <= 0)
        return;
    printf("\nhistroy: %s %ld\n", filename, length);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }
    for (size_t i = 0; i < length; i += 8) {
        unsigned char byte = 0;
        for (size_t j = 0; j < 8 && (i + j) < length; j++) {
            byte |= (history[i + j] << (7 - j));
        }
        fprintf(file, "%02x", byte);
        if (i % 32 == 0 && i != 0)
            fprintf(file, " ");
    }
    fclose(file);
}

void close_transmit() {
    // if (handle) {
    //     dlclose(handle);
    //     handle = NULL;
    // }
    if (base != MAP_FAILED) {
        struct stat st;
        if (fstat(fd, &st) == 0) {
            if ((st.st_size & 0xFFF) != 0) {
                st.st_size |= 0xFFF;
                st.st_size += 1;
            }
            munmap(base, st.st_size);
        }
    }
    close(fd);
    save_history("sentbits.log", __sent_bits, __sent_counter);
    save_history("receivebits.log", __received_bits, __received_counter);
}

size_t calibrate() {
    size_t start = rdtsc();
    size_t freq = 0, max_cycle = 0, sum_cycle = 0;
    while (rdtsc() - start < SLOT_LENGTH) {
        size_t reload_time = onlyreload(probe);
        freq++;
        max_cycle = MAX(max_cycle, reload_time);
        sum_cycle += reload_time;
    }
    size_t avg_cycle = sum_cycle / freq;
    // printf("freq: %ld avg: %ld max: %ld thresh %ld\n", freq, avg_cycle,
    //    max_cycle, avg_cycle);
    return avg_cycle;
}

bool __detect_bit() {
    size_t time = rdtsc();
    size_t hit = 0, miss = 0, max_rld = 0;
    while (rdtsc() - time < SLOT_LENGTH) {
        size_t reload_time = onlyreload(probe);
        max_rld = MAX(max_rld, reload_time);
        if (reload_time > MIN_CACHE_MISS_CYCLES) {
            miss++;
        } else {
            hit++;
        }
    }
    // printf("R:: miss: %d hit: %d bit: %d\n", miss, hit, miss >= hit);
    return miss >= hit;
}

bool _detect_bit() {
    int count = 0;
    for (int i = 0; i < BIT_REPEAT; i++) {
        if (__detect_bit()) {
            count++;
        }
    }
    bool val = count > (BIT_REPEAT / 2);
    // printf("%d", val);
    // if (__received_counter >= HISTORY_SIZE) {
    //     save_history("receivebits.log", __received_bits, __received_counter);
    //     __received_counter = 0;
    // }
    // __received_bits[__received_counter++] = val;
    return val;
}

int future = -1;
bool detect_bit() {
    int curr;
    if (future == -1)
        future = _detect_bit();
    curr = future;
    future = _detect_bit();
    if (curr == 1 && future == 0) {
        future = -1;
        return detect_bit();
    }
    // printf("%d", curr);
    return curr;
}

void __send_bit(bool value) {
    size_t start = rdtsc();
    if (value)
        while (rdtsc() - start < SLOT_LENGTH) {
            onlyflush((void *)probe);
        }
    else
        while (rdtsc() - start < SLOT_LENGTH) {
        }
}

void _send_bit(bool value) {
    // if (__sent_counter >= HISTORY_SIZE) {
    //     save_history("sentbits.log", __sent_bits, __sent_counter);
    //     __sent_counter = 0;
    // }
    // __sent_bits[__sent_counter++] = value;
    // printf("%d", value);
    for (int i = 0; i < BIT_REPEAT; i++)
        __send_bit(value);
}

void send_bit(bool value) {
    // printf("%d", value);
    static bool prev_value = -1;
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

int wait_magic_seq() {
    size_t maxtry = 1000, try = 0;
    // size_t allen = 0;
    // bool all_recieved[maxtry];
    int prev = 0, freq_10 = 0, curr;
    while (freq_10 < 3 && ++try < maxtry) {
        curr = _detect_bit(base);
        // all_recieved[allen++] = curr;
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

    return freq_10;
}