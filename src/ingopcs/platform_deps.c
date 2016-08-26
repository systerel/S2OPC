/*
 * platform_deps.c
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include <assert.h>
#include <platform_deps.h>

Endianess endianess  = Endianess_Undefined;
Endianess floatEndianess = Endianess_Undefined;

uint32_t little_endian(){
  uint32_t x = 0x0001;
  return (x == *((char *)&x));
}

uint32_t float_big_endian(){
    float f = -0.0;
    // Check if sign bit is the first
    assert((0b10000000 & *((char*) &f)) == 0b10000000 ||
           (0b10000000 & ((char*) &f)[3]) == 0b10000000);
    return (0b10000000 & *((char*) &f)) == 0b10000000;
}

void InitPlatformDependencies(){
    if(little_endian() == 0){
        endianess = Endianess_BigEndian;
    }else{
        endianess = Endianess_LittleEndian;
    }
    if(float_big_endian() == 0){
        floatEndianess = Endianess_LittleEndian;
    }else{
        floatEndianess = Endianess_BigEndian;
    }
}
