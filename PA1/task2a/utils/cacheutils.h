#ifndef CACHEUTILS_H
#define CACHEUTILS_H

#include <stddef.h>
#include <stdint.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

extern size_t probe_timing(void *addr);
extern unsigned long long rdtsc();
extern void maccess(void *p);
extern void flush(void *p);

#endif // CACHEUTILS_H