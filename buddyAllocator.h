#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include "linked_list.h"
#include "pool_allocator.h"

typedef struct BuddyListItem {
    ListItem list;
    int idx;
    int level;
    char *start;
    int size;
    struct BuddyListItem *buddy_ptr;
    struct BuddyListItem *parent_ptr;
} BuddyListItem;

typedef struct {
    ListHead free[32];  // Array di liste libere, uno per livello
    int num_levels;
    PoolAllocator list_allocator;
    char *memory;  // Memoria gestita dal buddy allocator
    int min_bucket_size;
} BuddyAllocator;

void BuddyAllocator_init(BuddyAllocator *alloc, int num_levels, char *buffer, int buffer_size, char *memory, int min_bucket_size);
void *BuddyAllocator_malloc(BuddyAllocator *alloc, int size);
void BuddyAllocator_free(BuddyAllocator *alloc, void *mem);

#endif
