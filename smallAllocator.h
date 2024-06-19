#define BUDDY_ALLOCATOR_H

#include <stddef.h>

void initAllocator();
void *allocator(size_t size);
void freeAlloc(void *ptr);