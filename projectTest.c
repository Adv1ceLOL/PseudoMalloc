#include "buddyAllocatorBitMap.h"
#include <stdio.h>


int main() {
    BuddyAllocator alloc;

    BuddyAllocator_init(&alloc);  

    // Testa small allocation
    void* ptr1 = BuddyAllocator_malloc(&alloc, (size_t)(508));  // 508 B request
    if (ptr1 != NULL) {
        printf("Allocato 512 B a %p\n", ptr1);
    } else {
        printf("Fallita l'allocazione 512 B\n");
    }

    // Testa small allocation
    void* ptr2 = BuddyAllocator_malloc(&alloc, (size_t)(1019));  // 1019 B request
    if (ptr2 != NULL) {
        printf("Allocati 1019 B a %p\n", ptr2);
    } else {
        printf("Fallita l'allocazione 1019 B\n");
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

    // Testa small allocation
    void* ptr5 = BuddyAllocator_malloc(&alloc, (size_t)(32));  // 32 B request
    if (ptr5 != NULL) {
        printf("--> Allocati 32 B a %p\n", ptr5);
    } else {
        printf("Fallita l'allocazione 32 B\n");
    }

    // Testa small allocation
    void* ptr6 = BuddyAllocator_malloc(&alloc, (size_t)(1));  // 1 B request
    if (ptr6 != NULL) {
        printf("--> Allocati 1 B a %p\n", ptr6);
    } else {
        printf("Fallita l'allocazione 1 B\n");
    }

     // Testa small allocation
    void* ptr7 = BuddyAllocator_malloc(&alloc, (size_t)(1023));  // 1023 B request
    if (ptr7 != NULL) {
        printf("--> Allocati 1023 B a %p\n", ptr7);
    } else {
        printf("Fallita l'allocazione 1023 B\n");
    }

    // Testa errore troppo grande la richiesta
    void* ptr8 = BuddyAllocator_malloc(&alloc, (size_t)(1024*1024));  // 1 MB request
    if (ptr8 != NULL) {
        printf("--> Allocati 1 MB a %p\n", ptr8);
    } else {
        printf("Fallita l'allocazione 1 MB\n");
    }

    // Testa errore 0 bit
    void* ptr9 = BuddyAllocator_malloc(&alloc, (size_t)(0));  // 0 B request
    if (ptr9 != NULL) {
        printf("--> Allocati 0 B a %p\n", ptr9);
    } else {
        printf("Fallita l'allocazione 0 B\n");
    }

    // Libera small allocation
    BuddyAllocator_free(&alloc, ptr1);
    printf("Liberati 508 B da %p\n", ptr1);

     // Libera small allocation
    BuddyAllocator_free(&alloc, ptr2);
    printf("Liberati 1019 B da %p\n", ptr2);

    // Libera small allocation
    printf("Tentativo di double free 1\n");
    BuddyAllocator_free(&alloc, ptr2);
    printf("Liberati 1023 B da %p\n", ptr2); 

    // Libera large allocation
    if(ptr3 != NULL) ptr3 = BuddyAllocator_free(&alloc, ptr3);
    else printf("Gia stata liberata questa porzione di memoria \n");

    // Libera large allocation
    if(ptr4 != NULL) ptr4 = BuddyAllocator_free(&alloc, ptr4);
    else printf("Gia stata liberata questa porzione di memoria \n");

    printf("Provo double free per large allocation -------> ");
    // Libera large allocation
    if(ptr4 != NULL) ptr4 = BuddyAllocator_free(&alloc, ptr4);
    else printf("Gia stata liberata questa porzione di memoria \n");
    
    // Libera small allocation
    BuddyAllocator_free(&alloc, ptr5);
    printf("Liberati 32 B da %p\n", ptr5);  

    // Libera small allocation
    BuddyAllocator_free(&alloc, ptr6);
    printf("Liberati 1 B da %p\n", ptr6); 

    // Libera small allocation
    BuddyAllocator_free(&alloc, ptr7);
    printf("Liberati 1023 B da %p\n", ptr7); 

    // Libera small allocation
    printf("Tentativo di double free 2\n");
    BuddyAllocator_free(&alloc, ptr7);
    printf("Liberati 1023 B da %p\n", ptr7); 


    BuddyAllocator_destroy(&alloc);


    return 0;
}
