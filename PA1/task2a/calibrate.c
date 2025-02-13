#include "transmit_layer/transmit_layer.h"
#include <stddef.h>
#include <stdio.h>

int main() {
    size_t avg_thresh = 0, sum_thresh = 0, rounds = 100;
    open_transmit();
    for (int i = 0; i < rounds; i++)
        sum_thresh += calibrate();
    close_transmit();
    avg_thresh = (sum_thresh / rounds);
    printf("avg threshold %ld\n", avg_thresh);
}