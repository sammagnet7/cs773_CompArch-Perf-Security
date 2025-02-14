#ifndef SYSCONFIG_H
#define SYSCONFIG_H

// common
#define LLC_SIZE_MB 30
#define LLC_SIZE LLC_SIZE_MB * 1024 * 1024 // MB
#define BLOCK_SIZE 64                      // constant accross intel?

//
#define NUM_CHUNKS 512 
#define CHUNK_SIZE (LLC_SIZE / NUM_CHUNKS)

// used in occupancy
#define ARRAY_SIZE LLC_SIZE
#define NUM_BLOCKS_OCC (ARRAY_SIZE / BLOCK_SIZE)
#define CALIBRATION_ROUNDS 1000
#define WINDOW_ROUNDS 10
#define NUM_LINES (CHUNK_SIZE / BLOCK_SIZE)
// used in thrasher
#define THRASH_SIZE (LLC_SIZE) // Thrash with 20x LLC size
#define NUM_BLOCKS_THR (THRASH_SIZE / BLOCK_SIZE)

#endif
