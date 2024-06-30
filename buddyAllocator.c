#include "buddyAllocator.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

// Funzioni ausiliarie per gestire l'indice degli alberi binari
int levelIdx(size_t idx) {
    return (int)floor(log2(idx));
}

int buddyIdx(int idx) {
    return (idx & 1) ? idx - 1 : idx + 1;
}

int parentIdx(int idx) {
    return idx / 2;
}

// Inizializza il Buddy Allocator
void BuddyAllocator_init(BuddyAllocator *alloc, int num_levels, char *buffer, int buffer_size, char *memory, int min_bucket_size) {
    alloc->num_levels = num_levels;
    alloc->memory = memory;
    alloc->min_bucket_size = min_bucket_size;

    // Inizializza il pool allocator per le strutture di gestione
    PoolAllocator_init(&alloc->list_allocator, sizeof(BuddyListItem), buffer_size / sizeof(BuddyListItem), buffer, buffer_size);

    for (int i = 0; i <= num_levels; i++) {
        List_init(&alloc->free[i]);
    }

    // Crea un unico blocco grande quanto tutta la memoria gestita
    BuddyListItem *initial_item = (BuddyListItem *)PoolAllocator_getBlock(&alloc->list_allocator);
    initial_item->idx = 1;
    initial_item->level = num_levels;
    initial_item->start = memory;
    initial_item->size = min_bucket_size << num_levels;
    initial_item->buddy_ptr = NULL;
    initial_item->parent_ptr = NULL;

    List_pushFront(&alloc->free[num_levels], (ListItem *)initial_item);
}

// Allocazione della memoria
void *BuddyAllocator_malloc(BuddyAllocator *alloc, int size) {
    int mem_size = (1 << alloc->num_levels) * alloc->min_bucket_size;
    int level = floor(log2(mem_size / (size + 8)));

    if (level > alloc->num_levels)
        level = alloc->num_levels;

    BuddyListItem *buddy = BuddyAllocator_getBuddy(alloc, level);
    if (!buddy)
        return NULL;

    BuddyListItem **target = (BuddyListItem **)(buddy->start);
    *target = buddy;
    return buddy->start + 8;
}

BuddyListItem *BuddyAllocator_getBuddy(BuddyAllocator *alloc, int level) {
    if (level < 0)
        return NULL;
    if (!alloc->free[level].size) {
        BuddyListItem *parent_ptr = BuddyAllocator_getBuddy(alloc, level - 1);
        if (!parent_ptr)
            return NULL;

        int left_idx = parent_ptr->idx << 1;
        int right_idx = left_idx + 1;

        BuddyListItem *left_ptr = BuddyAllocator_createListItem(alloc, left_idx, parent_ptr);
        BuddyListItem *right_ptr = BuddyAllocator_createListItem(alloc, right_idx, parent_ptr);

        left_ptr->buddy_ptr = right_ptr;
        right_ptr->buddy_ptr = left_ptr;
    }
    if (alloc->free[level].size) {
        BuddyListItem *item = (BuddyListItem *)List_popFront(alloc->free + level);
        return item;
    }
    return NULL;
}

BuddyListItem *BuddyAllocator_createListItem(BuddyAllocator *alloc, int idx, BuddyListItem *parent_ptr) {
    BuddyListItem *item = (BuddyListItem *)PoolAllocator_getBlock(&alloc->list_allocator);
    item->idx = idx;
    item->level = levelIdx(idx);
    item->start = alloc->memory + (idx << (alloc->num_levels - item->level) * alloc->min_bucket_size);
    item->size = alloc->min_bucket_size << item->level;
    item->buddy_ptr = NULL;
    item->parent_ptr = parent_ptr;
    return item;
}

// Deallocazione della memoria
void BuddyAllocator_free(BuddyAllocator *alloc, void *mem) {
    char *p = (char *)mem - 8;
    BuddyListItem **buddy_ptr = (BuddyListItem **)p;
    BuddyListItem *buddy = *buddy_ptr;
    assert(buddy->start == p);

    BuddyAllocator_releaseBuddy(alloc, buddy);
}

void BuddyAllocator_releaseBuddy(BuddyAllocator *alloc, BuddyListItem *item) {
    BuddyListItem *parent_ptr = item->parent_ptr;
    BuddyListItem *buddy_ptr = item->buddy_ptr;

    List_pushFront(&alloc->free[item->level], (ListItem *)item);

    if (!parent_ptr)
        return;

    if (buddy_ptr->list.prev == 0 && buddy_ptr->list.next == 0)
        return;

    BuddyAllocator_destroyListItem(alloc, item);
    BuddyAllocator_destroyListItem(alloc, buddy_ptr);
    BuddyAllocator_releaseBuddy(alloc, parent_ptr);
}

void BuddyAllocator_destroyListItem(BuddyAllocator *alloc, BuddyListItem *item) {
    List_detach((ListItem *)item);
    PoolAllocator_releaseBlock(&alloc->list_allocator, item);
}
