#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "smallAllocator.h"
#include "bitMap.h"
#include "linked_list.h"

#define PAGESIZE 4096
#define MAXBLOCKSIZE 512
#define LARGE_REQUEST (PAGESIZE / 4)
#define MEMORY_SIZE (1024*1024)
#define BUFFER_SIZE 1024

extern BitMap bitMap;
char memory[MEMORY_SIZE];
char buffer[BUFFER_SIZE];

void setBit(BitMap *bitMap, int index) {
    // Calcola l'indice dell'array e la posizione del bit all'interno del byte
    int byteIndex = index / 8;
    int bitPosition = index % 8;
    // Imposta il bit come occupato
    bitMap->buffer[byteIndex] |= (1 << bitPosition);
}

//Una funzione che decide il metodo di allocazione basandosi sulla dimensione della richiesta (< 1/4, >=1/4)
void *mallocModel(size_t size){
    // < 1/4 grandezza pagina
    if(size < PAGESIZE / 4){
        return allocator(size);
    }else{
        // >=1/4 grandezza pagina
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("mmappati %zu byte di memoria\n", size);
        if (ptr == MAP_FAILED) {
            perror("mmap error :( \n");
            return NULL;
        }
        return ptr;
    }

}

void *listMallocModel(size_t size){
    // < 1/4 grandezza pagina
    if(size < PAGESIZE / 4){
        return BuddyAllocator_malloc(size);
    }else{
        // >=1/4 grandezza pagina
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("mmappati %zu byte di memoria\n", size);
        if (ptr == MAP_FAILED) {
            perror("mmap error :( \n");
            return NULL;
        }
        return ptr;
    }

}

//dealloca dipendendo dal caso
void freeModel(void *ptr, size_t size) {
    if (size >= PAGESIZE / 4) {
        int rt = munmap(ptr, size);
        if (rt == -1) {
            perror("munmap");
            return;
        }else{
            printf("Oggetto di size %zu byte mappato è stato liberato\n", size);
            return;
        } 
    }  
} 


int main(){
    char input[100];
    printf("Che tipo di struttura dati vuoi usare? comandi: (bitmap, linked, pool) \n");
    printf(" ----> ");
    if(scanf("%s", input) == 1){
        if(strcmp(input,"bitmap") == 0){
            initAllocator();

             for(int i = 0; i < 8; i++) { // N è il numero di blocchi che vuoi marcare come occupati
                setBit(&bitMap, i); 
            }

            //Piccola richiesta
            size_t smallRequestSize = 512;
            void * smallRequest = mallocModel(smallRequestSize);
            freeModel(smallRequest, smallRequestSize);

            //Grande richiesta 
            size_t largeRequestSize = 2048;
            void * largeRequest = mallocModel(largeRequestSize);
            freeModel(largeRequest, largeRequestSize);
        }
        else if (strcmp(input, "linked") == 0) {
            // Inizializza il buddy allocator
            BuddyAllocator_init(memory, MEMORY_SIZE);

            printMemoryBlocksStatus();

            // Occupa manualmente i primi 5 blocchi
            void* ptr;
            for (int i = 0; i < 5; i++) {
                ptr = BuddyAllocator_malloc(MAXBLOCKSIZE);
                if (ptr == NULL) {
                    printf("Errore nell'occupare il blocco %d\n", i);
                    return -1;
                }
            }

            // Piccola richiesta tramite buddy allocator
            size_t smallRequestSize = 256;
            void *smallRequest = listMallocModel(smallRequestSize);
            if (smallRequest) {
                printf("Allocato smallRequest di %zu byte tramite buddy allocator\n", smallRequestSize);
            } else {
                printf("Allocazione fallita tramite buddy allocator\n");
            }
            printMemoryBlocksStatus();
            
            BuddyAllocator_free(smallRequest, smallRequestSize);

            printMemoryBlocksStatus();

            // Libera manualmente i primi 5 blocchi
            for (int i = 0; i < 5; i++) {
                //BuddyAllocator_freeLastBlock();
            }

            printMemoryBlocksStatus();

            // Grande richiesta tramite buddy allocator
            size_t largeRequestSize = 2048;
            void *largeRequest = listMallocModel(largeRequestSize);
            if (largeRequest) {
                printf("Allocato largeRequest di %zu byte tramite mmap\n", largeRequestSize);
            } else {
                printf("Allocazione fallita tramite mmap\n");
            }
            BuddyAllocator_free(largeRequest, largeRequestSize);
        }

    }


    return 0;
}






