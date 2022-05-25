/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hexlify.h"

// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
int hexlify(const unsigned char* src, char* dst, size_t n)
{
    assert(n <= INT32_MAX);
    size_t i;
    char buffer[3];
    int res = 0;

    if (!src || !dst)
        return -1;

    for (i = 0; i < n; ++i)
    {
        res = sprintf(buffer, "%02" SCNx8, src[i]); // sprintf copies the last \0 too
        assert(2 == res);
        memcpy(dst + 2 * i, buffer, 2);
    }

    return (int) n;
}

// You should allocate strlen(src)/2 in dst. n is strlen(dst)
// Returns n the number of translated couples (< 0 for errors)
int unhexlify(const char* src, unsigned char* dst, size_t n)
{
    assert(n <= INT32_MAX);
    static unsigned int buf;
    size_t i;

    if (!src || !dst)
        return -1;

    for (i = 0; i < n; ++i)
    {
        if (sscanf(&src[2 * i], "%02x", &buf) < 1)
        {
            return (int) i;
        }
        else
        {
            dst[i] = (unsigned char) buf;
        }
    }

    return (int) n;
}
