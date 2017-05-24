/** \file
 *
 */
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


#include <stdio.h>
#include <string.h>

#include "hexlify.h"


// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
int hexlify(const unsigned char *src, char *dst, size_t n)
{
    size_t i;
    char buffer[3];

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i) {
        sprintf(buffer, "%02hhx", src[i]); // sprintf copies the last \0 too
        memcpy(dst+2*i, buffer, 2);
    }

    return n;
}

// You should allocate strlen(src)/2 in dst. n is strlen(dst)
// Returns n the number of translated couples (< 0 for errors)
int unhexlify(const char *src, unsigned char *dst, size_t n)
{
    static unsigned int buf;
    size_t i;

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i)
    {
        if(sscanf(&src[2*i], "%02x", &buf) < 1){
            return i;
        }else{
        	dst[i] = (char)buf;
        }
    }

    return n;
}




