/*
 * platform_deps.c
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include <platform_deps.h>

void little_endian (){
  uint32_t x = 0x0001;
  isLittleEndian = ( x == *((char *)&x));
}

uint32_t Initialize(){
	little_endian();
	return 0;
}
