#ifndef TRANSMIT_LAYER_H
#define TRANSMIT_LAYER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern char *base;
extern void *probe;
extern int fd; // shared lib file

int open_transmit();
void close_transmit();

size_t calibrate();

void send_bit(bool bit);
extern int future;
bool detect_bit();

void send_magic_seq();
int wait_magic_seq();

void send_data(uint32_t *data);
void receive_data(uint32_t *data);

#endif // TRANSMIT_LAYER_H