#include "./common.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
void dumps_uint32_bits(uint32_t *bits, size_t length,
                       const char *out_file_name) {
    FILE *file = fopen(out_file_name, "wb");
    if (!file) {
        perror("fopen");
        return;
    }

    size_t written = fwrite(bits, sizeof(uint32_t), length, file);
    if (written != length) {
        perror("fwrite");
    }

    fclose(file);
}
void dumps_bool_bits(bool *bits, size_t length, const char *out_file_name) {
    FILE *file = fopen(out_file_name, "wb");
    if (!file) {
        perror("fopen");
        return;
    }

    uint8_t byte = 0;
    size_t bit_count = 0;

    for (size_t i = 0; i < length; i++) {
        if (bits[i]) {
            byte |= (1 << (7 - bit_count));
        }
        bit_count++;

        if (bit_count == 8) {
            fwrite(&byte, sizeof(uint8_t), 1, file);
            byte = 0;
            bit_count = 0;
        }
    }
    if (bit_count > 0) {
        fwrite(&byte, sizeof(uint8_t), 1, file);
    }

    fclose(file);
}
size_t read_bool_file(const char *filename, bool *bits) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        return 0;
    }

    size_t bit_count = 0;
    uint8_t byte;

    while (fread(&byte, sizeof(uint8_t), 1, file) == 1) {
        for (int i = 0; i < 8; i++) {
            if (bit_count >= MAX_LIMIT_BOOL) {
                perror("bits length longer than limit");
                break;
            }
            bits[bit_count++] = (byte & (1 << (7 - i))) != 0;
        }
    }

    fclose(file);
    return bit_count;
}
void print_bits(bool *bits, size_t bits_len) {
    printf("bitslen: %ld\n", bits_len);
    size_t num_byte = 0;
    for (size_t i = 0; i < bits_len; i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && (i + j) < bits_len; j++) {
            if (bits[i + j]) {
                byte |= (1 << (7 - j));
            }
        }
        printf("%02X", byte);
        ++num_byte;
        if (num_byte % 2 == 0)
            printf(" ");
    }
    printf("\n");
}
int main() {
    bool bits[MAX_LIMIT_BOOL];
    size_t bits_len = read_bool_file("msg.txt", bits);
    for (int i = 0; i < bits_len; i++) {
        printf("%d", bits[i]);
    }
    // dumps_bool_bits(bits, bits_len, "out.bin");
}