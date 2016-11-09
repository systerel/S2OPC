/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include "platform_deps.h"

Endianess endianess  = P_Endianess_Undefined;
Endianess floatEndianess = P_Endianess_Undefined;

uint32_t little_endian(){
  uint32_t x = 0x0001;
  return (x == *((uint8_t *)&x));
}

uint32_t float_big_endian(){
    float f = -0.0;
    // Check if sign bit is the first
// GCC version with binary constants extension
//    assert((0b10000000 & *((char*) &f)) == 0b10000000 ||
//           (0b10000000 & ((char*) &f)[3]) == 0b10000000);
//    return (0b10000000 & *((char*) &f)) == 0b10000000;
        assert((0x80 & *((char*) &f)) == 0x80 ||
               (0x80 & ((char*) &f)[3]) == 0x80);
        return (0x80 & *((char*) &f)) == 0x80;
}

void InitPlatformDependencies(){
    if(little_endian() == 0){
        endianess = P_Endianess_BigEndian;
    }else{
        endianess = P_Endianess_LittleEndian;
    }
    if(float_big_endian() == 0){
        floatEndianess = P_Endianess_LittleEndian;
    }else{
        floatEndianess = P_Endianess_BigEndian;
    }
}
