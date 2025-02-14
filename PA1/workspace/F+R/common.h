#include "utils/cacheutils.h"

#ifndef COMMON_H
#define COMMON_H

#define ull unsigned long long
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define SLOT_LENGTH 15000000
#define MIN_CACHE_MISS_CYCLES 200
#define SHARED_ARRAY_SIZE 49984
#define MESSAGE_CHUNK_LEN 4800 // bits; multiple of 8

#define BIT_REPEAT 3 // send each bit how many times
#define MAX_ROUNDS 1 // send the whole payload how many times
#define ENCODED_MESSAGE_LEN MESSAGE_CHUNK_LEN
#define HISTORY_SIZE (MESSAGE_CHUNK_LEN * MAX_ROUNDS)

#define ASCII_BITS 8
#define START_SEQ_LEN 5

#define TIME_MASK 0x3FFFFFFF
#define SYNC_INTERVAL 536870912
extern ull start_sync();

#endif
