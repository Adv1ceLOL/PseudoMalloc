#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buddy_allocator.h"

#define MAX_BLOCK_SIZE 2048


void initAllocator() {
    //memset
}

void *allocator(size_t size) {
    if (size > MAX_BLOCK_SIZE) return NULL; 
    //trovare il blocco libero
    return NULL; 
}

void freeAlloc(void *ptr) {
    // TODO
}
