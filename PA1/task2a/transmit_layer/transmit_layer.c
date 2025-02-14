#include "transmit_layer.h"
#include "../common.h"
#include <fcntl.h>
// #include <math.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void __send_bit(bool bit) {
    size_t start = rdtsc();
    if (bit)
        while (rdtsc() - start < SLOT_LENGTH) {
            onlyflush((void *)probe);
        }
    else
        while (rdtsc() - start < SLOT_LENGTH) {
        }
}

void _send_bit(bool bit) {
    // if (__sent_counter >= HISTORY_SIZE) {
    //     save_history("sentbits.log", __sent_bits, __sent_counter);
    //     __sent_counter = 0;
    // }
    // __sent_bits[__sent_counter++] = value;
    // printf("%d", value);
    for (int i = 0; i < BIT_REPEAT; i++)
        __send_bit(bit);
}

void send_bit(bool bit) {
    // printf("%d", bit);
    static bool prev_bit = -1;
    if (prev_bit == 1 && bit == 0) {
        _send_bit(1);
        _send_bit(0);
    }
    _send_bit(bit);
    prev_bit = bit;
}

void send_one_uint32(uint32_t data) {
    // printf("%08x; ", data);
    send_magic_seq();
    for (int i = sizeof(uint32_t) * 8 - 1; i >= 0; i--) {
        bool bit = (data >> i) & 1;
        send_bit(bit);
    }
    send_magic_seq();
    // printf(" | ");
}

void send_data(uint32_t *datas, size_t length) {
    size_t bits_per_uint32 = sizeof(uint32_t) * 8;
    size_t uint32s_per_chunk = MESSAGE_CHUNK_LEN / bits_per_uint32;
    size_t num_batches =
        (length + uint32s_per_chunk - 1) / uint32s_per_chunk; // Round up
    ull sync_ts;

    for (size_t batch = 0; batch < num_batches; batch++) {
        sync_ts = start_sync();
        printf("\nS:: TS: %lld Round: %ld/%ld\n", sync_ts, batch, num_batches);

        size_t start = batch * uint32s_per_chunk;
        size_t end = start + uint32s_per_chunk;
        if (end > length) {
            end = length;
        }

        for (size_t i = start; i < end; i++) {
            send_one_uint32(datas[i]);
        }
    }
}

size_t compact_bools_2_uint32(bool *bitstream, size_t bitstream_length,
                              uint32_t **datas) {
    size_t bits_per_uint32 = sizeof(uint32_t) * 8;
    size_t num_uint32 =
        (bitstream_length + bits_per_uint32 - 1) / bits_per_uint32; // Round up

    *datas = (uint32_t *)malloc(num_uint32 * sizeof(uint32_t));
    if (*datas == NULL) {
        perror("malloc");
        return 0;
    }

    memset(*datas, 0, num_uint32 * sizeof(uint32_t));
    uint32_t temp = 0;
    for (size_t i = 0; i < bitstream_length; i++) {
        size_t index = i / bits_per_uint32;
        size_t shift = bits_per_uint32 - 1 - (i % bits_per_uint32);
        temp |= ((uint32_t)bitstream[i] << shift);
        if (i % bits_per_uint32 == 0 && i != 0) {
            if (temp != 0x0000)
                (*datas)[index] = temp;
            temp = 0;
            // printf("%08x;", (*datas)[index - 1]);
        }
    }

    return num_uint32;
}

size_t receive_data(uint32_t *received_chunks[MAX_ROUNDS]) {
    ull sync_ts;
    size_t round = 0, try = 0, max_try = 2;
    while (try < max_try) {
        sync_ts = start_sync();
        printf("\nR:: TS: %lld round: %ld\n", sync_ts, round);
        future = -1;
        size_t len = 0;
        bool received[MESSAGE_CHUNK_LEN];

        int freq_10 = wait_magic_seq();

        if (freq_10 < 3) {
            printf("Error: Sequence not found, freq_10: %d try: %ld/%ld \n",
                   freq_10, try, max_try);
            received_chunks[round] = NULL;
            ++try;
            continue;
        }
        try = 0;
        len = 0;
        while (len < MESSAGE_CHUNK_LEN)
            received[len++] = detect_bit();

        compact_bools_2_uint32(received, MESSAGE_CHUNK_LEN,
                               &received_chunks[round]);
        if (!received_chunks[round])
            break;
        ++round;

        // len = 0;
        // while (len < MESSAGE_CHUNK_LEN)
        //     printf("%d", received[len++]);
        // printf("\n");
    }

    // for (int i = 0; i < round; i++)
    //     for (int j = 0; j < MESSAGE_CHUNK_LEN / 32; j++)
    //         if (received_chunks[i][j])
    //             printf("%08x;", received_chunks[i][j]);
    // printf("\n");

    return round;
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