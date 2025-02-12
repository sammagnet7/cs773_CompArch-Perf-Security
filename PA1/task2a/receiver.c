#include "cacheutils.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MIN_CACHE_MISS_CYCLES 100
size_t flushandreload(void *addr) {
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    flush(addr);
    return delta;
}

size_t onlyreload(void *addr) {
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
#define TIME_MASK 0x1f
#define SLOT_LENGTH 1500000
#define SYNC_INTERVAL 15
int main() {
    int fd = open("./libshared.so", O_RDONLY);
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        return EXIT_FAILURE;
    }

    // making the map_size page-aligned i.e. a multiple of 4 KB page
    // in order to grant the address space using mmap
    if (st.st_size & 0xFFF != 0) {
        st.st_size |= 0xFFF;
        st.st_size += 1;
    }
    // Memory map the shared library
    char *base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

    while (rdtsc() & TIME_MASK > SYNC_INTERVAL) {
    }
    size_t time = rdtsc();
    size_t delta = rdtsc() - time;
    void *probe = base + 0x4020;
    size_t hit = 0, miss = 0;
    while (delta < SLOT_LENGTH) {
        size_t reload_time = onlyreload(probe);
        if (reload_time > MIN_CACHE_MISS_CYCLES) {
            // printf("CACHE MISS %ld\n", reload_time);
            miss++;
        } else {
            hit++;
            // printf("CACHE HIT %ld\n", reload_time);
        }
        delta = rdtsc() - time;
    }
    printf("\nreceiver-slot end %ld hit %ld miss %ld\n", delta, hit, miss);
    return 0;
}
