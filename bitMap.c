#include <assert.h>
#include "bitmap.h"

// returns the number of bytes to store bits booleans
int BitMap_getBytes(int bits){
  return bits/8 + ((bits%8)!=0); //senno dava solo il resto
}

// initializes a bitmap on an external array
void BitMap_init(BitMap* bit_map, int num_bits, char* buffer){
  bit_map->buffer=buffer;
  bit_map->num_bits=num_bits;
  bit_map->buffer_size=BitMap_getBytes(num_bits);
}

// sets a the bit bit_num in the bitmap
// status= 0 or 1
void BitMap_setBit(BitMap* bit_map, int bit_num, int status){
  // get byte
  int byte_num=bit_num>>3;
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte=bit_num&0x07; // Vogliamo isolare i 3 byte meno significativi, non 2. Per trovare bit_num
  if (status) {
    bit_map->buffer[byte_num] |= (1<<bit_in_byte);
  } else {
    bit_map->buffer[byte_num] &= ~(1<<bit_in_byte);
  }
}

// inspects the status of the bit bit_num
int BitMap_bit(const BitMap* bit_map, int bit_num){
  int byte_num=bit_num>>3;      //numero byte = numero_bit/2^3
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte=bit_num&0x07; // Vogliamo isolare i 3 bit meno significativi, non 2. Per trovare bit_num. Con 3 bit lavoriamo da 0 a 7 (8 bit = 1 byte)
  return (bit_map->buffer[byte_num] & (1<<bit_in_byte))!=0;
}