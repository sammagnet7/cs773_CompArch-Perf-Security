#include "cacheutils.h"
#include "common.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
int touch();
extern unsigned int shared_array[80];
void stringToBinary(const char *str, char *binary) {
    char temp[9];     // Each character needs 8 bits + 1 null terminator
    binary[0] = '\0'; // Initialize the binary string

    for (size_t i = 0; i < strlen(str); i++) {
        unsigned char ch = str[i];
        // Convert the character to binary
        for (int j = 7; j >= 0; j--) {
            temp[7 - j] = (ch & (1 << j)) ? '1' : '0';
        }
        temp[8] = '\0';       // Null-terminate the temporary string
        strcat(binary, temp); // Append the binary representation to the result
        if (i < strlen(str) - 1) {
            strcat(
                binary,
                " "); // Add space between binary representations of characters
        }
    }
}

void binaryToString(const char *binary, char *result) {
    char temp[9];   // Buffer to hold each 8-bit binary string + null terminator
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

char *base, *probe;

void __send_bit(int value) {
    size_t start = rdtsc();
    if (value)
        while (rdtsc() - start < SLOT_LENGTH) {
            flush((void *)probe);
        }
    else
        while (rdtsc() - start < SLOT_LENGTH) {
        }
}
void _send_bit(int value) {
    for (int i = 0; i < BIT_REPEAT; i++)
        __send_bit(value);
}
void send_bit(int value) {
    static int prev_value = -1;
    if (prev_value == 1 && value == 0) {
        _send_bit(1);
        _send_bit(0);
    }
    _send_bit(value);
    prev_value = value;
}

void send_string(const char *str) {
    for (int i = 0; i < START_SEQ_LEN; i++) {
        _send_bit(1);
        _send_bit(0);
    }
    while (*str) {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--) {
            send_bit((c >> i) & 1);
        }
    }
    for (int i = 0; i < START_SEQ_LEN; i++) {
        _send_bit(1);
        _send_bit(0);
    }
}

void print_string(const char *str) {
    printf("sbits: ");
    // for (int i = 0; i < START_SEQ_LEN; i++) {
    //     printf("1");
    //     printf("0");
    // }
    int prev_bit = -1, curr_bit = -1;
    while (*str) {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--) {
            curr_bit = (c >> i) & 1;
            // if (prev_bit == 1 && curr_bit == 0) {
            //     printf("1");
            //     printf("0");
            // }
            printf("%d", curr_bit);
            prev_bit = curr_bit;
        }
    }
    printf("\n");
}

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
    base = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    probe = base + 0x4020;
    const char *test_string = "Hello, world!!";
    // usleep(100);
    for (int round = 0; round < ROUNDS; round++) {
        start_sync();
        send_string(test_string);
        usleep(1000);
    }
    // printf("S start: %ld\n", start);
    // print_string(test_string);
    // printf("S start: %ld\n", start);

    return 0;
}
