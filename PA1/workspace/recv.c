#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

unsigned long long rdtsc()
{
    unsigned long long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

void flush(void *p)
{
    asm volatile("clflush 0(%0)\n"
                 :
                 : "c"(p)
                 : "rax");
}
#define MIN_CACHE_MISS_CYCLES 245
size_t onlyreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}

#define SLOT_LENGTH 1500000

int detect(__uint64_t base)
{
    size_t hit = 0, miss = 0, i = 0;
    size_t time = rdtsc();
    while (rdtsc() - time < SLOT_LENGTH)
    {
        size_t reload_time = onlyreload((void *)(base + (i * 64) % 49984));
        if (reload_time > MIN_CACHE_MISS_CYCLES)
        {
            // printf("CACHE MISS %ld\n", reload_time);
            miss++;
        }
        else
        {
            hit++;
            // printf("CACHE HIT %ld\n", reload_time);
        }
    }

    return hit > miss;
}
int main()
{
    int fd = open("./dump.txt", O_RDONLY);
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        return EXIT_FAILURE;
    }

    // making the map_size page-aligned i.e. a multiple of 4 KB page
    // in order to grant the address space using mmap
    if (st.st_size & 0xFFF != 0)
    {
        st.st_size |= 0xFFF;
        st.st_size += 1;
    }
    // Memory map the shared library
    char *base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    int size = 0;
    char dump[500];
    while (1)
    {
        int bit = detect((__uint64_t)base);
        dump[size++] = bit + '0';
        if (size == 500)
        {
            dump[size] = '\0';
            printf("%s\n", dump);
            break;
        }
    }
    return 0;
}
