#ifndef COMMON_H
#define COMMON_H

#include "cacheutils.h"
#include "utils.h"

#define ull unsigned long long
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define SLOT_LENGTH 1500000
#define MIN_CACHE_MISS_CYCLES 135
#define SHARED_ARRAY_SIZE 8192
#define MESSAGE_CHUNK_LEN 4800 // bits; multiple of 8
#define CHUNK_SIZE 256
#define BIT_REPEAT 3 // send each bit how many times
#define MAX_CHUNKS 1 // send the whole payload how many times
#define ENCODED_MESSAGE_LEN MESSAGE_CHUNK_LEN
#define HISTORY_SIZE (MESSAGE_CHUNK_LEN * MAX_CHUNKS)


#define ACK_THRESHOLD 25000
#define MAGIC_PREAMBLE 0xABCDEFAB
#define MAGIC_SEQ_LEN 32
#define MAGIC_MASK 0xFFFFFFFF
#define MAGIC_POSTAMBLE 0x13579BDF
#define ASCII_BITS 8
#define START_SEQ_LEN 5

#define TIME_MASK 0x3FF
#define SYNC_INTERVAL 0x1FF

#endif
