#ifndef BUDDY_ALLOCATOR_BITMAP_H
#define BUDDY_ALLOCATOR_BITMAP_H

#include "bitMap.h"
#include <stddef.h>

#define PAGE_SIZE 4096 // Dimensione di una pagina
#define SMALL_REQUEST_THRESHOLD (PAGE_SIZE / 4) // 1/4 della dimensione di una pagina
#define MAX_LEVELS 12
#define BIT_MAP_SIZE (1 << (MAX_LEVELS + 1)) // Numero di bit richiesti

typedef struct LargeAllocation {
    void* ptr;
    size_t size;
    struct LargeAllocation* next;
} LargeAllocation;

typedef struct {
    char *memory; // Puntatore alla memoria
    size_t memory_size; // Dimensione della memoria
    BitMap bit_map;
    uint8_t *bit_map_buffer; // Puntatore al buffer per il BitMap
    LargeAllocation* large_allocations; // Puntatore alla lista delle allocazioni grandi
} BuddyAllocator;

void BuddyAllocator_init(BuddyAllocator *alloc);
void* BuddyAllocator_malloc(BuddyAllocator *alloc, size_t size);
void BuddyAllocator_free(BuddyAllocator *alloc, void* ptr);
void BuddyAllocator_destroy(BuddyAllocator *alloc);

#endif
