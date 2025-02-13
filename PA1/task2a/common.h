#include "cacheutils.h"

#ifndef COMMON_H
#define COMMON_H

#define ull unsigned long long

#define SLOT_LENGTH 15000
#define MIN_CACHE_MISS_CYCLES 200
#define MESSAGE_CHUNK_LEN 7000 // bits === 100 ascii chars

#define BIT_REPEAT 3 // send each bit how many times
#define ROUNDS 1     // send the whole payload how many times

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
