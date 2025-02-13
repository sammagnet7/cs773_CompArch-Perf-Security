#include "utils/cacheutils.h"

#ifndef COMMON_H
#define COMMON_H

#define ull unsigned long long
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define SLOT_LENGTH 15000000
#define MIN_CACHE_MISS_CYCLES 230
#define MESSAGE_CHUNK_LEN 700 // bits; /7 chars

#define BIT_REPEAT 3 // send each bit how many times
#define ROUNDS 1     // send the whole payload how many times

#define ASCII_BITS 7
#define START_SEQ_LEN 5

#define TIME_MASK 0x3FFFFFFF
#define SYNC_INTERVAL 536870912
extern ull start_sync();

#endif
