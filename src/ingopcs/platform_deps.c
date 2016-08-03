/*
 * platform_deps.c
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include <platform_deps.h>

uint32_t pendianess  = Endianess_Undefined;

uint32_t little_endian(){
  uint32_t x = 0x0001;
  return ( x == *((char *)&x));
}

void Initialize_Platform_Dependencies(){
    if(little_endian() == 0){
        pendianess = Endianess_BigEndian;
    }else{
        pendianess = Endianess_LittleEndian;
    }
}
