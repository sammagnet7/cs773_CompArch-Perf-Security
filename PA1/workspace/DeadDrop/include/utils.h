#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern char *base;
extern int fd; // shared lib file
extern struct stat st;
int open_transmit(char *shared_file);
void close_transmit();

unsigned long long start_sync();

#endif // UTILS_H