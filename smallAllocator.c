#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "smallAllocator.h"
#include "bitMap.h"
#include "linked_list.h"

#define MEMORYSIZE 1024*1024 //1 MB
#define MAXBLOCKSIZE 4096 //4 KB


//Variabili statiche
static char memory[MEMORYSIZE];
// 1024 * 1024 = 1 MB diviso in blocchi da 4 KB
static uint8_t bitMapBuffer[MEMORYSIZE / MAXBLOCKSIZE / 8];
BitMap bitMap;

//Settiamo il buddy allocator nello stato necessario
void initAllocator() {
    //Calcoliamo il numero di bit necessari per il bitMap
    int numBits = MEMORYSIZE / MAXBLOCKSIZE; 
    printf("numBits: %d\n", numBits);
    //Inizializziamo il bitMap
    BitMap_init(&bitMap, numBits, bitMapBuffer);
    //Azzeriamo il buffer di bitMap
    memset(bitMap.buffer, 0, bitMap.buffer_size);
    printf("Allocator inizializzato. Memoria azzerata. Spazio disponibile %d\n", bitMap.buffer_size);
}



void* allocator(size_t size) {
    if (size > MAXBLOCKSIZE) {
        printf("Tentativo di allocazione di %zu byte, superiore al limite di %d.\n", size, MAXBLOCKSIZE);
        return NULL;
    }

    int totalBlocks = bitMap.buffer_size * 8;  // Numero totale di blocchi nella bitmap

    for (int i = 0; i < totalBlocks; ++i) {
        printf("i = %d / totalBlocks = %d\n", i, totalBlocks);
        // Dobbiamo controllare se il blocco è libero
        if (!BitMap_bit(&bitMap, i)) {
            // Se è libero il blocco cambiamo lo stato a occupato
            BitMap_setBit(&bitMap, i, 1);
            printf("Memoria allocata all'indice %d.\n", i);
            return memory + (i * MAXBLOCKSIZE);
        }
    }
    // Non c'è il blocco
    printf("Allocazione fallita: memoria insufficiente.\n");
    return NULL;
}

//Libera il blocco di memoria allocato
void freeAlloc(void *ptr) {
    int index = ((char *)ptr - memory) / MAXBLOCKSIZE;
    if (index >= 0 && index < bitMap.buffer_size * 8) {
        //Blocco diventa libero cambiando lo stato
        BitMap_setBit(&bitMap, index, 0); 
        printf("Blocco liberato all'indice: %d\n", index);
    }else {
        printf("Tentativo di liberare un puntatore non valido.\n");
    }
}
