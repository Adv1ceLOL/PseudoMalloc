#include "buddyAllocatorBitMap.h"
#include <stdio.h>

int main() {
    BuddyAllocator alloc;
    BuddyAllocator_init(&alloc);

    // Test small allocation
    void* ptr1 = BuddyAllocator_malloc(&alloc, (size_t)(0.5 * 1024));  // 512 B request
    if (ptr1 != NULL) {
        printf("Main: Allocati 512 B a %p\n", ptr1);
    } else {
        printf("Main: Fallito l'allocazione di 512 B\n");
    }

    // Free small allocation
    BuddyAllocator_free(&alloc, ptr1);
    printf("Main: Liberati 512 B da %p\n", ptr1);

    return 0;
}
