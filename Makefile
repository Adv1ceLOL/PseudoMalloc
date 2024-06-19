CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: malloc

malloc: malloc.o smallAllocator.o bitMap.o
	$(CC) $(CFLAGS) -o malloc malloc.o smallAllocator.o bitMap.o

malloc.o: malloc.c smallAllocator.h
	$(CC) $(CFLAGS) -c malloc.c

smallAllocator.o: smallAllocator.c smallAllocator.h bitMap.h
	$(CC) $(CFLAGS) -c smallAllocator.c

bitMap.o: bitMap.c bitMap.h
	$(CC) $(CFLAGS) -c bitMap.c

clean:
	rm -f *.o malloc