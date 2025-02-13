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

void send_string(const char *str) {
    send_magic_seq();
    while (*str) {
        char c = *str++;
        for (int i = ASCII_BITS - 1; i >= 0; i--) {
            send_bit((c >> i) & 1);
        }
    }
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *content = (char *)malloc(length + 1);
    if (!content) {
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

int main() {
    open_transmit();
    char *test_string = read_file("msg.txt");
    if (!test_string) {
        return EXIT_FAILURE;
    }
    size_t length = strlen(test_string);
    size_t batch_size = (MESSAGE_CHUNK_LEN / ASCII_BITS), batches = 0,
           num_batches = length / batch_size;
    ull sync_ts;
    char batch[batch_size + 1];
    for (size_t i = 0; i < length; i += batch_size) {
        strncpy(batch, test_string + i, batch_size);
        sync_ts = start_sync();
        // printf("\nS:: TS: %lld Round: %ld/%ld\n", sync_ts, batches,
        // num_batches);
        batch[batch_size] = '\0'; // Null-terminate the batch
        send_string(batch);
        // usleep(10000);
        batches++;
        // printf("\n");
    }
    printf("Sent content in %ld batches of %ld bits\n", batches,
           batch_size * 7);
    free(test_string);
    close_transmit();
    return 0;
}
