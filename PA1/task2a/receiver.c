#include "common.h"
#include "transmit_layer/transmit_layer.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

size_t round = 0;
char *receive_messages[ROUNDS];

char *reconstruct(bool *received_bits, size_t length) {
    size_t num_chars = length / ASCII_BITS;
    char *output = (char *)malloc(num_chars + 1);
    if (!output) {
        printf("unable to allocate buffer on heap");
        return NULL;
    }
    for (size_t i = 0; i < num_chars; i++) {
        char c = 0;
        for (int j = 0; j < ASCII_BITS; j++) {
            c = (c << 1) | received_bits[i * ASCII_BITS + j];
        }
        output[i] = c;
        // printf(".%c", c);
    }
    output[num_chars] = '\0'; // Null terminate the string
    return output;
}

void receive_message(char **received_message) {
    future = -1;
    size_t len = 0, msg_len = MESSAGE_CHUNK_LEN;
    bool received[msg_len];

    int freq_10 = wait_magic_seq();

    if (freq_10 < 3) {
        printf("Error: Sequence not found, freq_10: %d\n", freq_10);
        *received_message = NULL;
        return;
    }

    len = 0;
    while (len < msg_len)
        received[len++] = detect_bit(base);

    msg_len = len;
    // printf("10s %d + rbits: ", freq_10);
    // len = 0;
    // while (len < msg_len)
    //     printf("%d", received[len++]);
    // printf("\n");
    *received_message = reconstruct(received, msg_len);
    // printf("\nreceived message: %s\n", *received_message);
}

void append_to_file(const char *filename, const char *message) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        perror("fopen");
        return;
    }
    fprintf(file, "%s", message);
    fclose(file);
}

void clear_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("fopen");
    }
    fclose(file);
}

void cleanup() {
    clear_file("received.txt");

    for (int i = 0; i < round; i++) {
        if (receive_messages[i]) {
            printf("round %d : %s", i, receive_messages[i]);
            append_to_file("received.txt", receive_messages[i]);
            free(receive_messages[i]);
        }
    }

    close_transmit();
    exit(0);
}

void signal_handler(int signum) {
    if (signum == SIGINT) {
        cleanup();
    }
}

int main() {
    signal(SIGINT, signal_handler);

    open_transmit();

    // calibrate();
    // exit(0);

    ull sync_ts;
    round = 0;
    for (int i = 0; i < ROUNDS; i++)
        receive_messages[i] = 0;

    do {
        sync_ts = start_sync();
        printf("\nR:: TS: %lld round: %ld\n", sync_ts, round);
        receive_message(&receive_messages[round++]);
        // if (receive_messages[round - 1])
        // printf("\nreceived %ld : %s", round, receive_messages[round - 1]);
        // printf("\n");
    } while (receive_messages[round - 1]);

    cleanup(receive_messages);
}