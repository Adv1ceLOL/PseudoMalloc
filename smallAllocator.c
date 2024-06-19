#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smallAllocator.h"
#include "bitMap.h"

#define MEMORYSIZE 1024*1024 //1 MB
#define MAXBLOCKSIZE 4096 //4 KB


//Variabili statiche
static char memory[MEMORYSIZE];
// 1024 * 1024 = 1 MB diviso in blocchi da 4 KB
static uint8_t bitMapBuffer[MEMORYSIZE / MAXBLOCKSIZE / 8];
static BitMap bitMap;

//Settiamo il buddy allocator nello stato necessario
void initAllocator() {
    //Calcoliamo il numero di bit necessari per il bitMap
    int numBits = MEMORYSIZE / MAXBLOCKSIZE; 
    //Inizializziamo il bitMap
    BitMap_init(&bitMap, numBits, bitMapBuffer);
    //Azzeriamo il buffer di bitMap
    memset(bitMap.buffer, 0, bitMap.buffer_size);
}

//Alloca memoria blocco di memoria di dimensione specifica
void *allocator(size_t size) {
    if (size > MAXBLOCKSIZE) return NULL;
    
    for (int i=0; i < bitMap.buffer_size  * 8; ++i) {
        //Dobbiamo controllare se il blocco è libero
        if (!BitMap_bit(&bitMap, i)) {
            //Se è libero il blocco cambiamo lo stato a occupato
            BitMap_setBit(&bitMap, i, 1);
            return memory + (i * MAXBLOCKSIZE);
        }
    }
    //Non c'è il blocco
    return NULL;
}

//Libera il blocco di memoria allocato
void freeAlloc(void *ptr) {
    int index = ((char *)ptr - memory) / MAXBLOCKSIZE;
    if (index >= 0 && index < bitMap.buffer_size * 8) {
        //Blocco diventa libero cambiando lo stato
        BitMap_setBit(&bitMap, index, 0); 
    }
}
