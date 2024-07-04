#include "buddyAllocatorBitMap.h"
#include <stdio.h>

int main() {
    BuddyAllocator alloc;
    BuddyAllocator_init(&alloc);

    // Testa small allocation
    void* ptr1 = BuddyAllocator_malloc(&alloc, (size_t)(0.5 * 1024));  // 512 B request
    if (ptr1 != NULL) {
        printf("Allocato 512 B a %p\n", ptr1);
    } else {
        printf("Fallita l'allocazione 512 B\n");
    }

    // Testa small allocation
    void* ptr2 = BuddyAllocator_malloc(&alloc, (size_t)(1023));  // 1023 B request
    if (ptr2 != NULL) {
        printf("Allocati 1023 B a %p\n", ptr2);
    } else {
        printf("Fallita l'allocazione 1023 B\n");
    }

    // Testa large allocation
    void* ptr3 = BuddyAllocator_malloc(&alloc, (size_t)(90 * 1024));  // 90KB request
    if (ptr3 != NULL) {
        printf("Allocati 90KB a %p\n", ptr3);
    } else {
        printf("Fallita l'allocazione 90KB\n");
    }

    // Testa large allocation
    void* ptr4 = BuddyAllocator_malloc(&alloc, (size_t)(300 * 1024)); // 300KB request
    if (ptr4 != NULL) {
        printf("Allocati 300KB a %p\n", ptr4);
    } else {
        printf("Fallita l'allocazione 300KB\n");
    }

    // Libera small allocation
    BuddyAllocator_free(&alloc, ptr1);
    printf("Liberati 512 B da %p\n", ptr1);

     // Libera small allocation
    BuddyAllocator_free(&alloc, ptr2);
    printf("Liberati 1203 B da %p\n", ptr2);

    // Libera large allocation
    BuddyAllocator_free(&alloc, ptr3);
    printf("Liberati 90KB da %p\n", ptr3);

    // Libera large allocation
    BuddyAllocator_free(&alloc, ptr4);
    printf("Liberati 300KB da %p\n", ptr4);

    BuddyAllocator_destroy(&alloc); // Dealloca il buffer del BitMap

    return 0;
}
