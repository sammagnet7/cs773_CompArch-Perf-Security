// #include "cacheutils.h"
#include "common.h"
#include "transmit_layer/transmit_layer.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void send_string(const char *str)
{
    send_magic_seq();
    while (*str)
    {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--)
        {
            send_bit((c >> i) & 1);
        }
    }
    send_magic_seq();
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("fopen");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *content = (char *)malloc(length + 1);
    if (!content)
    {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    // Read the file content into the allocated memory
    fread(content, 1, length, file);
    content[length] = '\0'; // Null-terminate the string

    fclose(file);
    return content;
}

uint32_t PACK(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3)
{
    return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}

size_t string_2_uint32(char *string, uint32_t **datas)
{
    size_t length = strlen(string);
    size_t num_uint32 = (length + 3) / 4; // Round up

    *datas = (uint32_t *)malloc(num_uint32 * sizeof(uint32_t));
    if (*datas == NULL)
    {
        perror("malloc");
        return -1;
    }

    memset(*datas, 0, num_uint32 * sizeof(uint32_t));

    for (size_t i = 0; i < num_uint32; i++)
    {
        uint8_t c0 = (i * 4 < length) ? string[i * 4] : '\0';
        uint8_t c1 = (i * 4 + 1 < length) ? string[i * 4 + 1] : '\0';
        uint8_t c2 = (i * 4 + 2 < length) ? string[i * 4 + 2] : '\0';
        uint8_t c3 = (i * 4 + 3 < length) ? string[i * 4 + 3] : '\0';
        (*datas)[i] = PACK(c0, c1, c2, c3);

        // printf("%c%c%c%c : %08x; ", c0, c1, c2, c3, (*datas)[i]);
    }

    return num_uint32;
}

void stringToBinary(const char *str, char *binary)
{
    char temp[9];     // Each character needs 8 bits + 1 null terminator
    binary[0] = '\0'; // Initialize the binary string
    for (size_t i = 0; i < strlen(str); i++)
    {
        unsigned char ch = str[i];
        // Convert the character to binary
        for (int j = 7; j >= 0; j--)
        {
            temp[7 - j] = (ch & (1 << j)) ? '1' : '0';
        }
        temp[8] = '\0';       // Null-terminate the temporary string
        strcat(binary, temp); // Append the binary representation to the result
    }
}

int ssend_bit(int bit)
{
    size_t start = rdtsc(), index = 0;
    if (bit)
        while (rdtsc() - start < SLOT_LENGTH)
        {
            flush((void *)(base + index));
            index = (index + 64) % SHARED_ARRAY_SIZE;
        }
    else
        while (rdtsc() - start < SLOT_LENGTH)
        {
        }
}
int main()
{
    open_transmit(); // opens the shared file
    // char *payload = read_file("msg.txt"); // read input to send
    char payload[] = "Hello, World!";
    if (!payload)
    {
        return EXIT_FAILURE;
    }
    char binary[strlen(payload) * 8 + 1];
    stringToBinary(payload, binary);

    printf("Binary: %s\n", binary);
    int binary_len = strlen(binary);
    size_t index = 0;
    while (1)
    {
        int bit = binary[index] - 48;
        ssend_bit(bit);
        if (++index == binary_len)
            break;
    }

    ////////////////////////// Constants //////////////////////////
    // size_t bits_per_uint32 = sizeof(uint32_t) * 8;
    // size_t uint32s_per_chunk = MESSAGE_CHUNK_LEN / bits_per_uint32;
    // size_t num_chunks =
    //     (datas_len + uint32s_per_chunk - 1) / uint32s_per_chunk; // Round up

    /////////////////// Send the message in chunks ///////////////////
    // for (size_t chunk = 0; chunk < num_chunks; chunk++)
    // {
    //     ////////// Index calculation for each chunk //////////
    //     size_t start = chunk * uint32s_per_chunk;
    //     size_t end = start + uint32s_per_chunk;
    //     if (end > datas_len)
    //     {
    //         end = datas_len;
    //     }
    //     // Need to sync here
    //     ////////// Looping over each 32 bit integer in chunk //////////
    //     for (size_t i = start; i < end; i++)
    //     {
    //         //////// Looping over all bits in 32 bit integer ////////
    //         for (int j = sizeof(uint32_t) * 8 - 1; j >= 0; j--)
    //         {
    //         }
    //         printf("\n");
    //     }
    // }

    // free(payload);
    close_transmit();
    return 0;
}
