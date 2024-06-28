#define BUDDY_ALLOCATOR_H

#include <stddef.h>
#include "linked_list.h"
#include "bitMap.h"


typedef struct MemoryBlock {
    ListItem list;
    char* start;
    size_t size;
    int isOccupied;
} MemoryBlock;

//BitMap
void initAllocator();
void *allocator(size_t size);
void freeAlloc(void *ptr);

//LinkedList
void BuddyAllocator_init(char*, int);
void* BuddyAllocator_malloc(size_t);
void BuddyAllocator_free(void*,size_t);
void printMemoryBlocksStatus();
void BuddyAllocator_freeLastBlock();
