#include "common.h"
#include "utils.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_RECEIVED_BITS_SIZE 1000 * 1024 * 8
#define DEAD_DROP_START_INDEX 0x6fff
#define DEAD_DROP_END_INDEX 0xAfff
#define DEAD_DROP_SIZE (DEAD_DROP_END_INDEX - DEAD_DROP_START_INDEX)
size_t CHUNKS = 0;
size_t received_till_now = 0;
size_t data_till_now = 0;
int received_bits[MAX_RECEIVED_BITS_SIZE];
#define OUTPUT_FILE "received.txt"
void dump_bits(int *bits, size_t length, const char *out_file_name) {
    printf("Dumping bits to file %s\n", out_file_name);
    FILE *file = fopen(out_file_name, "wb");
    if (!file) {
        perror("fopen");
        return;
    }

    uint8_t byte = 0;
    size_t bit_count = 0;

    for (size_t i = 0; i < length; i++) {
        if (bits[i]) {
            byte |= (1 << (7 - bit_count));
        }
        bit_count++;

        if (bit_count == 8) {
            fwrite(&byte, sizeof(uint8_t), 1, file);
            byte = 0;
            bit_count = 0;
        }
    }
    if (bit_count > 0) {
        fwrite(&byte, sizeof(uint8_t), 1, file);
    }

    fclose(file);
}

size_t ronlyreload(void *addr) {
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
int rdetect_bit() {
    size_t hit = 0, miss = 0, i = 0;
    size_t time = rdtsc();
    while (rdtsc() - time < SLOT_LENGTH) {
        size_t reload_time = ronlyreload((void *)(base + i));
        i = (i + 64) % SHARED_ARRAY_SIZE; // bytes in shared text file in
                                          // nearest multiple of 64
        if (reload_time > MIN_CACHE_MISS_CYCLES) {
            miss++;
        } else {
            hit++;
        }
    }
    return miss >= hit;
}
void sigint_handler(int signum) {
    dump_bits(received_bits, received_till_now, OUTPUT_FILE);
    exit(0);
}

void sync_preamble() {
    // size_t maxtry = 1000, try = 0;
    int curr;
    uint32_t pattern_history = 0;
    // start_sync();
    // wait for magic sequence
    while ((pattern_history & MAGIC_MASK) != MAGIC_PREAMBLE) {
        curr = rdetect_bit((__uint64_t)base);
        pattern_history = (pattern_history << 1) | (curr & 1);
    }
    printf("R-GOT PREAMBLE: ID: %ld\n", received_till_now);
}
void sync_postamble() {
    int curr;
    uint32_t pattern_history = 0;
    // start_sync();
    // wait for magic sequence
    while ((pattern_history & MAGIC_MASK) != MAGIC_POSTAMBLE) {
        curr = rdetect_bit((__uint64_t)base);
        pattern_history = (pattern_history << 1) | (curr & 1);
    }
    printf("R-GOT POSTAMBLE: ID: %ld\n", received_till_now);
}

int send_acknowledgement() {
    size_t progress = DEAD_DROP_START_INDEX;
    size_t hit = 0, miss = 0;
    while (progress < DEAD_DROP_END_INDEX) {
        flush((void *)(base + progress));
        progress += 64;
    }
    // printf("Hit: %ld, Miss: %ld\n", hit, miss);
    return miss > hit;
}

int main() {
    signal(SIGINT, sigint_handler);
    open_transmit("dump.txt"); // opening shared file
    memset(received_bits, -1, sizeof received_bits);
    printf("Initialised buffer\n");
    uint32_t pattern_history = 0;
    size_t EXTENDED_CHUNK_SIZE = CHUNK_SIZE + MAGIC_SEQ_LEN;
    while (1) {
        if (received_till_now % EXTENDED_CHUNK_SIZE == 0) {
            sync_preamble();
        }

        int bit = rdetect_bit((__uint64_t)base);
        pattern_history = (pattern_history << 1) | bit;
        received_bits[data_till_now] = bit;
        if (received_till_now % EXTENDED_CHUNK_SIZE ==
            EXTENDED_CHUNK_SIZE - 1) {

            if ((pattern_history & MAGIC_MASK) == MAGIC_POSTAMBLE) {
                data_till_now -= (MAGIC_SEQ_LEN + 1);
                printf("GOT POSTAMBLE\n");
                send_acknowledgement();
            } else {
                data_till_now -= (EXTENDED_CHUNK_SIZE + 1);
                printf("NO POSTAMBLE\n");
            }
            printf("INDEX: %ld\n", received_till_now);
        }
        received_till_now++;
        data_till_now++;
    }
}
