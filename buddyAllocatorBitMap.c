#include "buddyAllocatorBitMap.h"

// Mettiamo i blocchi ad 1 per i parenti del blocco 
void bitSetParent(BitMap *bit_map, int bit_num, int LiberoOccupato){
    // Settiamo il blocco a occupato
    BitMap_setBit(bit_map, bit_num, LiberoOccupato);
    // Se bit_num non è il blocco a livello 0, continua a salire
    if (bit_num != 0) bitSetParent(bit_map, (bit_num - 1) / 2 , LiberoOccupato); 
}

// Mettiamo i blocchi ad 1 per i figli del blocco 
void bitSetFiglio(BitMap *bit_map, int bit_num, int LiberoOccupato){
    // Non voglioamo uscire dalla BitMap
    if (bit_num < bit_map->num_bits){ 
        BitMap_setBit(bit_map, bit_num, LiberoOccupato); // Settiamo il blocco a occupato
        bitSetFiglio(bit_map, bit_num * 2 + 1, LiberoOccupato);  // Figlio di sinistra
        bitSetFiglio(bit_map, bit_num * 2 + 2, LiberoOccupato);  // Figlio di destra
    }
}
  /*        /
         padre
         /   \
       sx     dx
       /\     /\
*/

void BuddyAllocator_init(BuddyAllocator* alloc) {
    // Inizializziamo tutti i dati necessari per il buddy allocator (nella struct)
    char bufferAllocator[MEMORY_SIZE]; 
    char bufferBitMap[MEMORY_SIZE];

    int levels = BUDDY_LEVELS;    
    int minBlock = MEMORY_SIZE >> (levels); // (MEMORY_SIZE) / (2^levels)

    alloc->levels=levels;
    alloc->memory = bufferAllocator;
    alloc->memory_size = MEMORY_SIZE;
    alloc->minBlock=minBlock;
    alloc->bitMapBuffer = bufferBitMap;
    alloc->bitMapSize = MEMORY_SIZE;

    // Generazione del numero di bit necessari per la bit_map con tot livelli
    int num_bits = (1 << (levels + 1)) - 1 ;  

    // Creamo una BitMap con il nostro buddy allocator
    BitMap_init(&alloc->bit_map, num_bits, alloc->bitMapBuffer);

    //Bitmap_print(&alloc->bit_map);  
    return;
}


// Prende un Buddy allocator e una grandezza. Returna il puntatore dell'inizio del blocco
void* BuddyAllocator_malloc(BuddyAllocator *alloc, size_t size) {
    printf("BuddyAllocator_malloc: Richiesta di %zu byte\n", size);
    if (size >= (alloc->memory_size)) { // Controlli non si sa mai
        printf("ERRORE: Non abbastanza spazio per gestire questa richesta \n");
        return NULL;
    }
    if (size == 0)return NULL;
    if (size >= SMALL_REQUEST_THRESHOLD) {
        // Large request, usiamo mmap direttamente
        void* ptr = mmap(NULL, size + sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (ptr == MAP_FAILED) {
            printf("ERRORE: mmap di %zu bytes\n", size);
            return NULL;
        }
        
        ((int*) ptr)[0] = (int) size;

        printf("Allocati %zu bytes usando mmap in %p\n", size, ptr);
        return (void *)(ptr);
    }
    else {
        // Necessitiamo di un metodo per capire il livello incui bisonga occupare blocchi
        int level = alloc->levels; // Livello che cerchiamo
        int dimBlockLivello; // Spazio disponibile nel livello
         
        // PArtiamo dal livello piu basso e andiamo salendo
        dimBlockLivello = alloc->minBlock;
        // Possiamo fare cosi perche sappiamo la dimensione dei blocchi ad ogni livello
        while (dimBlockLivello < (int) size) {  // Continua finche non troviamo un blocco abbastanza grande per contenere l'allocazione
            dimBlockLivello *= 2;  // Al livello successivo avremo il doppio della dimensione del blocco precedente
            level--;  // Saliamo di livello
        }

        printf("Livello: %d \n", level);

        // Troviamo un blocco libero nel livello scelto
        int bloccoLibero;  
        int trovato = 0; // Controlliamo se lo abbiamo trovato o meno alla fine dei cicli

        if (level == 0){ // Se stiamo a livello 0 (caso apparte)
            if (!BitMap_bit(&alloc->bit_map, (1 << level) - 1)){ // Controlliamo se è libero l'unico blocco
                bloccoLibero = 0;
                trovato = 1;
            }
        }
        else{ 
            // Andiamo dal primo index ((1 << level) - 1) fino all'ultimo che sara il primo del livello successivo (non incluso)
            for(int j = (1 << level) - 1; j < (1 << (level + 1)) - 1; j++){
                if (j >= BIT_MAP_SIZE){ // Controlliamo di non superare il limite
                    printf("Superata la BITMAP SIZE\n");
                    break;
                } 
                // Se libero il blocco, lo abbiamo trovato, senno si continua a cercare
                if (!BitMap_bit(&alloc->bit_map, j)){ 
                    bloccoLibero=j; 
                    trovato = 1;
                    printf("Blocco Libero: %d \n", bloccoLibero);
                    break;
                }
            } 
        }

        if(!trovato){  
            printf("Nessun blocco libero trovato \n");
            return NULL;
        }

        // Avendo trovato il blocco, siccome è un albero, bisogna occupare anche tutti i blocchi imparentati, perche saranno occupati anche loro
        bitSetParent(&alloc->bit_map, bloccoLibero, 1);
        bitSetFiglio(&alloc->bit_map, bloccoLibero, 1);

        // Calcoliamo l'indirizzo del blocco che vogliamo occupare (indirizzo base + indirizzo blocco trovato - offset del bloccoLibero * dimensione del blocco al livello)
                                                                                                                                     // ^ Perche cosi otteniamo l'offset in byte dall'inizio dell'area di memoria                             
        char *indirizzo = alloc->memory + (bloccoLibero - ((1 << (int)floor(log2(bloccoLibero+1))) - 1)) * dimBlockLivello;
        // Per il malloc è inutile, ma siccome dovremmo liberare il blocco in tot indice eventualmente, va messo da parte che indice andra liberato
        ((int *)indirizzo)[0] = bloccoLibero;
        printf("Indice = %d , Puntatore returnato = %p \n", bloccoLibero, indirizzo);
        printf("Dimensione Blocco = %d , Dimensione effettivamente allocata = %zu \n",dimBlockLivello, size);
        Bitmap_print(&alloc->bit_map); // Controlliamo se è fatto bene o meno
        return (void *)(indirizzo + sizeof(int)); // Bisogna lasciare spazio per l'indice di 4 bit senno i blocchi liberati possono essere errati
    }
}


void* BuddyAllocator_free(BuddyAllocator *alloc, void* ptr) {
    if (ptr == NULL) return;
    printf("BuddyAllocator_free: Richiesta di liberare %p \n", ptr);
    // Controllo del puntatore: Si trova all'interno della zona di memoria del buddy allocator
    if (ptr >= (void*)alloc->memory && ptr < (void*)(alloc->memory + alloc->memory_size)) {
        // Recuperiamo l'indice del blocco memorizzato prima dell'indirizzo (quello fatto prima per il free nel malloc)
        int libero = ((int *)ptr)[-1];

        if (!BitMap_bit(&alloc->bit_map, libero)){
            printf("Questo blocco è stato gia liberato \n");
            return;
        }

        // Liberiamo tutti i figli sotto di lui (indice)
        bitSetFiglio(&alloc->bit_map, libero, 0);
        
        // Facciamo il Merge se serve
        while (libero != 0) { // Bisogna fare il ciclo finche non arriviamo al primo blocco (livello 0)
            int index; // Calcoliamo l'indice del buddy
            if (libero == 0)index = 0; // Se il blocco è libero, allora i due blocchi possono essere uniti
            // Il primo blocco dei due buddys è pari, secondo è dispari
            else if (libero % 2) index = libero + 1; // Se libero è pari: l'index del buddy sara libero - 1
            else index = libero - 1; // Se libero è dispari: l'index del buddy sara libero + 1

              /*       /
                    padre
           2*i + 1 /   \ 2*i + 2
                sx     dx
                /\     /\
            */

            printf("Buddy = %d , index = %d \t",index, libero);
            // 
            if (!BitMap_bit(&alloc->bit_map, index)) { // Se è libero ? 
                printf("Merge\n");
                int parentIndex = (libero - 1) / 2; // Indice del genitore preso dal index di sinistra
                BitMap_setBit(&alloc->bit_map, parentIndex, 0); // Libera il blocco del padre operando il merge
                libero = parentIndex; // Va a controllare il prossimo livello per il merge continuando il ciclo
            } else {
                // Non è disponibile il merge per questo livello quindi fermiamo il ciclo
                printf("Niente Merge\n");
                break; 
            }
        }

        Bitmap_print(&(alloc->bit_map));
        
    } else {
        int size = ((int *)ptr)[0];
        printf("size = %d \n", size);

        int ret = munmap(ptr, size + sizeof(int));
        if(ret == -1){
            printf("ERROR: munmap\n");
            return NULL;
        }
        printf("%p\n", ptr);

        printf("Liberata memoria allocata da mmap in %p di size = %d\n", ptr, size);
        return NULL;
    }
}

void BuddyAllocator_destroy(BuddyAllocator *alloc) {
    // Svuotiamo la struct
    alloc->levels = 0;
    alloc->memory = NULL;
    alloc->memory_size = 0;
    alloc->minBlock = 0;
    alloc->bitMapBuffer = NULL;
    alloc->bitMapSize = 0;

    printf("BuddyAllocator destroyed\n");
}

void Bitmap_print(BitMap *bit_map){
    // Settiamo le variabili che ci servono
    int restanoQuanti = 0; // Fa da cursore per sapere dove sto e quanto manca
    int livello = -1; // Tiene traccia del livello dove mi trovo
    int fine = floor(log2(bit_map->num_bits + 1))- 1; // Calcola il livello finale ( piu profondo )

    // Iteriamo su ogni bit della BitMap per stamparli
    for (int i = 0; i < bit_map->num_bits; i++){  
        if (restanoQuanti == 0){ // Finito la print di questo livello, passiamo al successivo
            if(livello == fine) break; // Finita l'intera stampa
            livello++;
            printf("\n Livello %d: \t", livello);
            restanoQuanti = 1 << livello; // Aumentiamo la variabile del cursore perche sappiamo quanti bit avremo al prossimo livello ( 2 ^ livello )
        }
        if (BitMap_bit(bit_map, i)==0) printf("%d ", BitMap_bit(bit_map, i)); // Se il blocco è libero, lo stampa normale in bianco
        else printf("\x1b[36m%d\x1b[0m ", BitMap_bit(bit_map, i)); // Se è occupato, per contrasto lo cambiamo di colore a ciano
        
        restanoQuanti--;  // Muoviamo il cursore
    }
    printf("\n");
};

