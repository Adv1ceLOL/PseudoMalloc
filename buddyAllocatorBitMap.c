#include "buddyAllocatorBitMap.h"
#include "bitMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MIN_BLOCK_SIZE 128
#define MAX_LEVELS 12 // log2(MEMORY_SIZE / MIN_BLOCK_SIZE)
#define SMALL_REQUEST_THRESHOLD (PAGE_SIZE / 4)
#define PAGE_SIZE 4096

void BuddyAllocator_init(BuddyAllocator *alloc) {
    alloc->memory_size = 1 << 20; // 2^20 Byte == 1 MB
    // Mappiamo la memoria del BuddyAllocator di 1 MB
    alloc->memory = (char *)mmap(NULL, alloc->memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (alloc->memory == MAP_FAILED) {
        fprintf(stderr, "buddyAllocator: Fallita l'allocazione con mmap alla memoria\n");
        exit(EXIT_FAILURE);
    }
    // Calcoliamo il # di byte necessari per il BitMap
    int bit_map_bytes = BitMap_getBytes(BIT_MAP_SIZE);
    // Il BitMap tiene conto dello stato dei blocchi
    printf("buddyAllocator: BIT_MAP_SIZE: %d, bit_map_bytes: %d\n", BIT_MAP_SIZE, bit_map_bytes);
    // Mappa la memoria per il buffer del BitMap
    alloc->bit_map_buffer = (uint8_t *)mmap(NULL, bit_map_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (alloc->bit_map_buffer == MAP_FAILED) {
        fprintf(stderr, "buddyAllocator: Fallita l'allocazione con mmap al buffer\n");
        exit(EXIT_FAILURE);
    }
    // Inizializza il BitMap
    BitMap_init(&alloc->bit_map, BIT_MAP_SIZE, alloc->bit_map_buffer);
    memset(alloc->memory, 0, alloc->memory_size);
    memset(alloc->bit_map_buffer, 0, bit_map_bytes);

    printf("buddyAllocator: BuddyAllocator inizializza con %zu byte di memoria\n", alloc->memory_size);
}

// Prende un Buddy allocator e una grandezza. Returna il puntatore dell'inizio del blocco
void* BuddyAllocator_malloc(BuddyAllocator *alloc, size_t size) {
    // Calcoliamo il livello dell'allocatore per la richiesta "size"
    int level = 0;
    while ((MIN_BLOCK_SIZE << level) < (int)size) level++; // Il blocco piu piccolo possibile ma abbastanza grande
    level = MAX_LEVELS - level - 1;

    // Ora cerchiamo un blocco libero nel livello calcolato
    for (int i = (1 << level) - 1; i < (1 << (level + 1)) - 1; i++) {
        if (i >= BIT_MAP_SIZE) break; // Assicurati di non superare BIT_MAP_SIZE
        int byte_num = i >> 3;
        printf("buddyAllocator: Controllo BitMap a index %d, byte_num %d, buffer_size %d\n", i, byte_num, alloc->bit_map.buffer_size);
        // Libero ? 
        if (!BitMap_bit(&alloc->bit_map, i)) {
            // Si? Segnamolo come occupato
            BitMap_setBit(&alloc->bit_map, i, 1);
            // Calcoliamo l'offset nel buffer (Indice blocco, dim blocchi al livello)
            int offset = (i - ((1 << level) - 1)) * (1 << (MAX_LEVELS - level - 1)) * MIN_BLOCK_SIZE;
            printf("buddyAllocator: Allocati %zu byte usando buddy allocator a %p (level: %d, index: %d)\n", size, &alloc->memory[offset], level, i);
            return &alloc->memory[offset];
        }
    }
    fprintf(stderr, "buddyAllocator: Fallito allocare %zu byte usando buddy allocator\n", size);
    return NULL;
}




void BuddyAllocator_free(BuddyAllocator *alloc, void* ptr) {
    if (ptr == NULL) return;
    
    // Controllo del puntatore: Si trova all'interno della zona di memoria del buddy allocator
    if (ptr >= (void*)alloc->memory && ptr < (void*)(alloc->memory + alloc->memory_size)) {
        // Offset del ptr all'inizio dell'area di memoria
        int offset = (char*)ptr - alloc->memory;
        // Uso l'offset per calcolare l'indice del BitMap (blocco)
        int index = offset / MIN_BLOCK_SIZE + (1 << MAX_LEVELS) - 1;

        // Controlla il caso incui l'index suepera il BIT_MAP_SIZE
        if (index >= BIT_MAP_SIZE) return;

        // Setta il bit a 0 ==> stato blocco a libero
        BitMap_setBit(&alloc->bit_map, index, 0);
        printf("buddyAllocator: Liberata memoria gestita dal buddy allocator a %p (index: %d)\n", ptr, index);

        // Una volta liberato il blocco, vanno riuniti i blocchi se necessario
        while (index > 0) {
            // Calcolo l'indice del buddyAllocator
            int buddy = (index % 2 == 0) ? index - 1 : index + 1;
            if (buddy >= BIT_MAP_SIZE) break;
            // Libero ? Uniscili
            if (!BitMap_bit(&alloc->bit_map, buddy)) {
                BitMap_setBit(&alloc->bit_map, buddy, 0);
                index = (index - 1) / 2;
            } else {
                break;
            }
        }
    } 
}



