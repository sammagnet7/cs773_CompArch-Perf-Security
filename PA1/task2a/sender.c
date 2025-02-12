#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "cacheutils.h"
int touch();
extern unsigned int shared_array[80];
void stringToBinary(const char *str, char *binary) {
    char temp[9]; // Each character needs 8 bits + 1 null terminator
    binary[0] = '\0'; // Initialize the binary string

    for (size_t i = 0; i < strlen(str); i++) {
        unsigned char ch = str[i];
        // Convert the character to binary
        for (int j = 7; j >= 0; j--) {
            temp[7 - j] = (ch & (1 << j)) ? '1' : '0';
        }
        temp[8] = '\0'; // Null-terminate the temporary string
        strcat(binary, temp); // Append the binary representation to the result
        if (i < strlen(str) - 1) {
            strcat(binary, " "); // Add space between binary representations of characters
        }
    }
}

void binaryToString(const char *binary, char *result) {
    char temp[9]; // Buffer to hold each 8-bit binary string + null terminator
    temp[8] = '\0'; // Null-terminate it
    size_t binaryLength = strlen(binary);
    size_t index = 0, resultIndex = 0;

    while (index < binaryLength) {
        int bitCount = 0;

        // Extract the next 8 bits from the binary string
        for (bitCount = 0; bitCount < 8 && index < binaryLength; index++) {
            if (binary[index] == '1' || binary[index] == '0') {
                temp[bitCount] = binary[index];
                bitCount++;
            } else if (isspace(binary[index])) {
                // Skip spaces between binary segments
                continue;
            } else {
                fprintf(stderr, "Invalid binary string\n");
                return;
            }
        }
        
        // Ensure we've extracted exactly 8 bits
        if (bitCount == 8) {
            unsigned char character = (unsigned char)strtol(temp, NULL, 2);
            result[resultIndex++] = character;
        }
    }

    result[resultIndex] = '\0'; // Null-terminate the resulting string
}

size_t flushandreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    flush(addr);
    return delta;
}

size_t onlyreload(void *addr)
{
    size_t time = rdtsc();
    maccess(addr);
    size_t delta = rdtsc() - time;
    return delta;
}
#define TIME_MASK 0x1f
#define SLOT_LENGTH 1500000
#define SYNC_INTERVAL 15
#define SEND_BIT 1
int main()
{
    int fd = open("./libshared.so", O_RDONLY);
    struct stat st;
    if (fstat(fd, &st) == -1) {
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
    char *base = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    char* probe = base + 0x4020;
    // while(1)
    // touch();
    while((rdtsc()&TIME_MASK) > SYNC_INTERVAL){}
    size_t time = rdtsc();
    // printf("%u\n", shared_array[0]);
    // printf("sender-SYNC %ld\n", time);
    size_t delta = rdtsc() - time;
    while (delta < SLOT_LENGTH)
    {
        if(SEND_BIT)
        flush((void*)probe);
        else{

        }
        delta = rdtsc() - time;
    }
    printf("\nsender-slot end %ld\n", delta);
    return 0;
}

