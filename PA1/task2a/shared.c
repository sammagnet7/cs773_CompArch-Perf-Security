// #include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char shared_array[700] = {'g'};

// char *shared_array = NULL;

// void initialize_shared_array() {
//     if (shared_array != NULL)
//         return;
//     shared_array = (char *)malloc(SHARED_ARRAY_SIZE * sizeof(char));
//     if (shared_array == NULL) {
//         // Handle allocation failure
//         perror("Failed to allocate memory for shared_array");
//         exit(EXIT_FAILURE);
//     }
//     // Initialize the array if needed
//     memset(shared_array, 'a', SHARED_ARRAY_SIZE);
// }

// void cleanup_shared_array() { free(shared_array); }