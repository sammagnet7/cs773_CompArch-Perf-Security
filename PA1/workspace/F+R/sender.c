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
#define MAX_LIMIT_BOOL 1000 * 1024 * 8 // 250 KB
#define CHUNK_SIZE 512

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

size_t read_bool_file(const char *filename, bool *bits)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("fopen");
        return 0;
    }

    size_t bit_count = 0;
    uint8_t byte;

    while (fread(&byte, sizeof(uint8_t), 1, file) == 1)
    {
        for (int i = 0; i < 8; i++)
        {
            if (bit_count >= MAX_LIMIT_BOOL)
            {
                perror("bits length longer than limit");
                break;
            }
            bits[bit_count++] = (byte & (1 << (7 - i))) != 0;
        }
    }

    fclose(file);
    return bit_count;
}

void ssend_bit(int bit)
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
    bool bit_stream[MAX_LIMIT_BOOL];
    size_t bits_len = read_bool_file("msg.txt", bit_stream);
    // for (int i = 0; i < bit_stream_len; i++)
    // {
    //     printf("%d", bit_stream[i]);
    //     if (i % 8 == 7)
    //     {
    //         printf(" ");
    //     }
    // }
    // printf("\n");
    size_t bit_index = 0;
    while (bit_index < bits_len)
    {
        // need to sync here
        // (bit_index & (CHUNK_SIZE - 1)) == CHUNK_SIZE - 1 ? printf("chunk end %ld\n", bit_index) : 0;
        ssend_bit(bit_stream[bit_index]);
        bit_index++;
    }
    // write_to_file("msg.txt", data);
    // char binary[strlen(payload) * 8 + 1];
    // stringToBinary(payload, binary);

    // printf("Binary: %s\n", binary);
    // int binary_len = strlen(binary);
    // size_t index = 0;
    // while (1)
    // {
    //     int bit = binary[index] - 48;
    //     ssend_bit(bit);
    //     if (++index == binary_len)
    //         break;
    // }

    ////////////////////////// Constants //////////////////////////
    // uint32_t *datas;
    // size_t datas_len = string_2_uint32(payload, &datas);
    // for (size_t i = 0; i < count; i++)
    // {
    //     /* code */
    // }

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
