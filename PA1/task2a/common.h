#include "cacheutils.h"

#ifndef COMMON_H
#define COMMON_H

#define ull unsigned long long

#define SLOT_LENGTH 1500000 // 1500000
#define MIN_CACHE_MISS_CYCLES 200

#define BIT_REPEAT 3 // send each bit
#define ROUNDS 1     // send the whole charater

#define ASCII_BITS 7
#define START_SEQ_LEN 5

#define TIME_MASK 0x3FFFFFFF
#define SYNC_INTERVAL 536870912
ull start_sync() {
    ull ts;
    while (((ts = rdtsc()) & TIME_MASK) > SYNC_INTERVAL) {
    }
    return ts;
}

#endif
