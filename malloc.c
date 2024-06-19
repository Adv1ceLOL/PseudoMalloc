#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include "smallAllocator.h"

#define PAGESIZE 4096
#define MAXBLOCKSIZE 4096

//Una funzione che decide il metodo di allocazione basandosi sulla dimensione della richiesta (< 1/4, >=1/4)
void *mallocModel(size_t size){
    // < 1/4 grandezza pagina
    if(size < PAGESIZE / 4){
        return allocator(size);
    }else{
        // >=1/4 grandezza pagina
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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
    initAllocator();

    //Piccola richiesta
    size_t smallRequestSize = 100;
    void * smallRequest = mallocModel(smallRequestSize);
    if(smallRequest)printf("Allocato smmallRequest di %zu \n", smallRequestSize);
    else printf("C'è stato un problema con il buddy allocator \n");
    freeModel(smallRequest, smallRequestSize);

    //Grande richiesta 
    size_t largeRequestSize = 10000;
    void * largeRequest = mallocModel(largeRequestSize);
    if(largeRequest)printf("Allocato largeRequest di %zu \n", largeRequestSize);
    else printf("C'è stato un problema con il mmap \n");
    freeModel(largeRequest, largeRequestSize);

    return 0;
}






