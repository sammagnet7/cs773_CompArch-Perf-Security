#include "common.h"
#include <stdio.h>

ull start_sync() {
    ull ts;
    while (((ts = rdtsc()) & TIME_MASK) > SYNC_INTERVAL) {
    }
    return ts;
}