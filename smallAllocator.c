#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

#include "smallAllocator.h"

#define MEMORYSIZE 1024*1024 // 1 MB
#define MAXBLOCKSIZE 512 // 512 byte (1/8 di una pagina di 4KB)
#define BUFFER_SIZE 1024
#define PAGESIZE 4096

//! BITMAP

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


//! LINKED LIST
ListHead freeList;
ListHead allBlocksList;

void BuddyAllocator_init(char* memory, int memory_size) {
    // Inizializza le linked list
    List_init(&freeList);
    List_init(&allBlocksList);

    // Inizializza il primo blocco di memoria
    MemoryBlock* initialBlock = (MemoryBlock*)memory;
    initialBlock->start = memory;
    initialBlock->size = memory_size;
    initialBlock->isOccupied = 0; // Assicurati che il blocco sia marcato come libero
    List_pushFront(&freeList, (ListItem*)initialBlock); // Pushamo il blocco nel front della lista freeList
    List_pushFront(&allBlocksList, (ListItem*)initialBlock); // Pushamo il blocco nel front della lista allBlocksList
}

void* BuddyAllocator_malloc(size_t size) {
    // Il blockNumber serve per capire che blocco è stato allocato
    static int blockNumber = 0;

    if (size > MAXBLOCKSIZE) {
        printf("Tentativo di allocazione di %zu byte, superiore al limite di %d.\n", size, MAXBLOCKSIZE);
        return NULL;
    }

    // Mette in item il primo blocco in allBlocksList
    ListItem* item = allBlocksList.first; 
    while (item) {
        MemoryBlock* block = (MemoryBlock*)item;
        //Controlla che il blocco sia libero o occupato e che il size da allocare entri nel blocco
        if (!block->isOccupied && block->size >= size) {
            // Se il blocco è più grande della richiesta, dividilo
            if (block->size > size + sizeof(MemoryBlock)) {
                // Calcoliamo lo spazio libero rimanente
                size_t remainingSize = block->size - size - sizeof(MemoryBlock);
                block->size = size;
                // Crea un nuovo blocco libero
                MemoryBlock* newBlock = (MemoryBlock*)(block->start + size);
                newBlock->start = block->start + size;
                newBlock->size = remainingSize;
                newBlock->isOccupied = 0;
                //Pusha il nuovo blocco nella lista
                List_pushFront(&allBlocksList, (ListItem*)newBlock);
            }
            block->isOccupied = 1;
            printf("Memoria allocata nel blocco numero %d\n", blockNumber);
            blockNumber++;
            return block->start;
        }
        item = item->next;
    }

    printf("Allocazione fallita: memoria insufficiente.\n");
    return NULL;
}

void BuddyAllocator_free(void *ptr, size_t size) {
    if (size >= PAGESIZE / 4) {
        // Gestione della liberazione di grandi blocchi allocati con mmap
        int rt = munmap(ptr, size);
        if (rt == -1) {
            perror("munmap");
            return;
        } else {
            printf("Oggetto di size %zu byte mappato è stato liberato\n", size);
            return;
        }
    } else {
        int trovato = 0;
        ListItem* item = allBlocksList.first;
        // Naviga la Lista per trovare il blocco da liberare
        while (item) {
            MemoryBlock* currentBlock = (MemoryBlock*)item;
            // Verifica se il blocco corrente corrisponde al blocco da liberare
            if (currentBlock->start == ptr && currentBlock->size == size) {
                // Se il blocco corrente è il blocco da liberare, aggiorna lo stato a libero
                currentBlock->isOccupied = 0;
                trovato = 1; // Imposta trovato a 1 per evitare l'aggiunta di un nuovo blocco
                printf("Blocco di memoria liberato.\n");
                // Aggiunge il blocco nella lista dei blocchi liberi
                List_pushBack(&freeList, (ListItem*)currentBlock);
                // Rimuove il blocco dalla lista di tutti i blocchi
                List_detach(&allBlocksList, (ListItem*)currentBlock);
                // Riaggiungiamo lo spazio liberato dal blocco
                ListItem* itemLibero = allBlocksList.first;
                MemoryBlock* currentBlockLibero = (MemoryBlock*)itemLibero;
                if(currentBlockLibero->isOccupied == 0)currentBlockLibero->size += size + sizeof(MemoryBlock);
                break;
            }
            item = item->next;
        }
        if (!trovato) {
            // Se non è stato trovato il blocco corrispondente
            printf("Errore: tentativo di liberare un blocco non trovato.\n");
        }
    }
}

void BuddyAllocator_freeLastBlock() {
    // Prende l'ultimo elemento della lista
    ListItem* lastItem = List_popBack(&allBlocksList);
    if (lastItem == NULL) {
        printf("Errore: nessun blocco da liberare.\n");
        return;
    }
    MemoryBlock* lastBlock = (MemoryBlock*)lastItem;
    // Controlla se l'ultimo blocco è occupato
    if (lastBlock->isOccupied) {
        // Se è occupato, lo libera
        lastBlock->isOccupied = 0;
        printf("L'ultimo blocco di memoria è stato liberato.\n");
    } else {
        printf("L'ultimo blocco di memoria era già libero.\n");
    }
    // Cerca blocchi liberi adiacenti e uniscili
    ListItem* item = freeList.first;
    while (item) {
        MemoryBlock* currentBlock = (MemoryBlock*)item;
        item = item->next; // Assicurati di avanzare al prossimo elemento prima di qualsiasi return
        if (currentBlock != lastBlock && !currentBlock->isOccupied &&
            (currentBlock->start + currentBlock->size) == lastBlock->start) {
            // Unisci i blocchi
            currentBlock->size += lastBlock->size; // Aggiorna la dimensione senza aggiungere sizeof(MemoryBlock) a meno che non sia necessario
            // Non è necessario aggiungere lastBlock alla lista dei liberi perché è già stato unito
            printf("Blocchi adiacenti uniti, nuova dimensione: %zu.\n", currentBlock->size);
            return;
        }
    }
    // Se non ci sono blocchi adiacenti da unire, aggiungi lastBlock alla lista dei liberi
    List_pushBack(&freeList, lastItem);
}

void printMemoryBlocksStatus() {
    ListItem* item = allBlocksList.first; // Utilizza allBlocksList che include tutti i blocchi
    printf("Stato dei blocchi nella memoria:\n");
    while (item) {
        MemoryBlock* block = (MemoryBlock*)item; // Utilizza direttamente il cast a MemoryBlock*
        if (block->isOccupied) { // Controlla se il blocco è occupato
            printf(" ---> [ Blocco: %p | Dimensione: %zu | Stato: Occupato ]\n", block->start, block->size);
        }
        else{
            printf(" ---> [ Blocco: %p | Dimensione: %zu | Stato: Libero ]\n", block->start, block->size);
        }
        item = item->next;
    }

    // Aggiunta per stampare i blocchi nella freeList
    printf("Stato dei blocchi liberi nella memoria:\n");
    item = freeList.first; // Ora inizia l'iterazione sulla freeList
    while (item) {
        MemoryBlock* block = (MemoryBlock*)item;
        printf(" ---> [ Blocco: %p | Dimensione: %zu | Stato: Libero ]\n", block->start, block->size);
        item = item->next;
    }
}