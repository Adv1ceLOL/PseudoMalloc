#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "smallAllocator.h"
#include "bitMap.h"
#include "linked_list.h"

#define PAGESIZE 4096
#define MAXBLOCKSIZE 4096
#define LARGE_REQUEST (PAGE_SIZE / 4)

extern BitMap bitMap;

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

//dealloca dipendendo dal caso
void freeModel(void *ptr, size_t size) {
    if (size < PAGESIZE / 4) {
        return freeAlloc(ptr);
    } else {
        if (munmap(ptr, size) == -1) {
            perror("munmap");
        }
    }
}


int main(){
    //char input[100];
    //printf("Che tipo di struttura dati vuoi usare? comandi: (bitmap, linked, pool) \n");
    //if(scanf("%s", input) == 1){
        //if(strcmp(input,"bitmap") == 0){
            initAllocator();

             for(int i = 0; i < 8; i++) { // N è il numero di blocchi che vuoi marcare come occupati
                setBit(&bitMap, i); 
            }

            //Piccola richiesta
            size_t smallRequestSize = 512;
            void * smallRequest = mallocModel(smallRequestSize);
            //if(smallRequest)printf("Allocato smmallRequest di %zu \n", smallRequestSize);
            //else printf("C'è stato un problema con il buddy allocator \n");
            freeModel(smallRequest, smallRequestSize);

            //Grande richiesta 
            size_t largeRequestSize = 2048;
            void * largeRequest = mallocModel(largeRequestSize);
            //if(largeRequest)printf("Allocato largeRequest di %zu \n", largeRequestSize);
            //else printf("C'è stato un problema con il mmap \n");
            freeModel(largeRequest, largeRequestSize);
        //}
    //}

    
    

    return 0;
}






