#ifndef TRANSMIT_LAYER_H
#define TRANSMIT_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern char *base;
extern int fd; // shared lib file
extern struct stat st;

int open_transmit();
void close_transmit();

void send_bit(bool bit);
extern int future;
bool detect_bit();

void send_magic_seq();
int wait_magic_seq();

void send_data(uint32_t *data, size_t length);
size_t receive_data(uint32_t **chunks);

#endif // TRANSMIT_LAYER_H