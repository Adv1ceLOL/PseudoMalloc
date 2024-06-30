CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: malloc

malloc: projectTest.o buddyAllocatorBitMap.o bitMap.o linked_list.o
	$(CC) $(CFLAGS) -o malloc projectTest.o buddyAllocatorBitMap.o bitMap.o linked_list.o -lm

projectTest.o: projectTest.c buddyAllocatorBitMap.h
	$(CC) $(CFLAGS) -c projectTest.c

buddyAllocatorBitMap.o: buddyAllocatorBitMap.c buddyAllocatorBitMap.h bitMap.h
	$(CC) $(CFLAGS) -c buddyAllocatorBitMap.c

bitMap.o: bitMap.c bitMap.h
	$(CC) $(CFLAGS) -c bitMap.c

linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) -c linked_list.c

clean:
	rm -f *.o malloc