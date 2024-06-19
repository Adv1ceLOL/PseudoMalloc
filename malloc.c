#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#define PAGESIZE 2048
#define MAXSIZEBLOCK 2048

void *mallocModel(size_t size){
    if(size < PAGESIZE / 4){
        return allocator(size);
    }else{
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap error :( \n");
            return NULL;
        }
        return ptr;
    }

}

void freeModel(void *ptr, size_t size) {
    if (size < PAGE_SIZE / 4) {
        //allocatore free funzione
    } else {
        if (munmap(ptr, size) == -1) {
            perror("munmap");
        }
    }
}


int main(){
    size_t smallRequestSize = 100;
    void * smallRequest = mallocModel(smallRequestSize);
    printf("Allocato smmallRequest di %zu \n", smallRequestSize);
    freeModel(smallRequest, smallRequestSize);

    size_t largeRequestSize = 10000;
    void * largeRequest = mallocModel(largeRequestSize);
    printf("Allocato largeRequest di %zu \n", largeRequestSize);
    freeModel(largeRequest, largeRequestSize);

    return 0;
}






