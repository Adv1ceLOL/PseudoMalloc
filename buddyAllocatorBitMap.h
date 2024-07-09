#ifndef BUDDY_ALLOCATOR_BITMAP_H
#define BUDDY_ALLOCATOR_BITMAP_H

#include "bitMap.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

#define BUDDY_LEVELS log2(MEMORY_SIZE / MIN_BLOCK_SIZE)

#define PAGE_SIZE 4096 // Dimensione di una pagina di solito
#define SMALL_REQUEST_THRESHOLD (PAGE_SIZE / 4) // 1/4 della dimensione di una pagina
#define BIT_MAP_SIZE (1 << (MAX_LEVELS + 1)) // Numero di bit richiesti
#define MIN_BLOCK_SIZE 512 // Dimensione blocco all'ultimo livello
#define MAX_LEVELS 12 // log2(MEMORY_SIZE / MIN_BLOCK_SIZE)
#define MEMORY_SIZE 1024 * 1024 // 1 MB
#define TOTAL_BLOCKS MEMORY_SIZE / MIN_BLOCK_SIZE // Numero di blocchi

typedef struct {
    char *memory; // Puntatore alla memoria del buddy
    size_t memory_size; // Dimensione della memoria del buddy
    int minBlock; // Dimensione minima del blocco
    int levels; // livelli da fare
    BitMap bit_map; // Punta alla bitMap
    size_t bitMapSize; // Size dellla BitMap
    char *bitMapBuffer; // Puntatore al buffer per il BitMap
    int* blocks_per_level; // Tiene traccia dei blocchi per livello
} BuddyAllocator;  

//! ---------------------------------------------------------------------------------------

void BuddyAllocator_init(BuddyAllocator* alloc);
void* BuddyAllocator_malloc(BuddyAllocator *alloc, size_t size);
void* BuddyAllocator_free(BuddyAllocator *alloc, void* ptr);
void BuddyAllocator_destroy(BuddyAllocator *alloc);

void Bitmap_print(BitMap *bitMap);

#endif

