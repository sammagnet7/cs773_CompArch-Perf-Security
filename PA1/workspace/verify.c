#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

void flush(void *p)
{
    asm volatile("clflush 0(%0)\n"
                 :
                 : "c"(p)
                 : "rax");
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
    int size = 100;
    int limit =  49984;
    int index = 0;
    while (1)
    {
        flush((void *)(base+(index*64)%limit));
    }
    return 0;
}
