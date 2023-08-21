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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_helper_encode.h"

// Return the decimal value of hexadecimal digit (0 for errors)
static uint8_t char_to_decimal(char c, bool* error)
{
    *error = false;
    if (('0' <= c) && ('9' >= c))
    {
        return (uint8_t)(c - '0');
    }
    if (('a' <= c) && ('f' >= c))
    {
        return (uint8_t)(c - 'a' + 10);
    }
    if (('A' <= c) && ('F' >= c))
    {
        return (uint8_t)(c - 'A' + 10);
    }

    *error = true;
    return 0;
}

// You should allocate strlen(src)/2 in dst. n is strlen(dst)
// Returns n the number of translated couples (< 0 for errors)
static int unhexlify(const char* src, unsigned char* dst, size_t n)
{
    SOPC_ASSERT(n <= INT32_MAX);
    bool error = false;

    if (NULL == src || NULL == dst)
    {
        return -1;
    }

    for (size_t i = 0; i < n; ++i)
    {
        uint8_t msb = (uint8_t)(char_to_decimal(src[2 * i], &error) << 4);
        if (error)
        {
            return -2;
        }
        uint8_t lsb = char_to_decimal(src[2 * i + 1], &error);
        if (error)
        {
            return -3;
        }
        dst[i] = (unsigned char) (msb + lsb);
    }

    return (int) n;
}

/* Using https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C_2
 * to decode base64 */
#define WHITESPACE 64
#define EQUALS 65
#define INVALID 66

static const unsigned char d[] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, WHITESPACE, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 62,      INVALID, INVALID, INVALID,    63,
    52,      53,      54,      55,      56,      57,      58,      59,      60,      61,      INVALID,    INVALID,
    INVALID, EQUALS,  INVALID, INVALID, INVALID, 0,       1,       2,       3,       4,       5,          6,
    7,       8,       9,       10,      11,      12,      13,      14,      15,      16,      17,         18,
    19,      20,      21,      22,      23,      24,      25,      INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, 26,      27,      28,      29,      30,      31,      32,      33,      34,      35,         36,
    37,      38,      39,      40,      41,      42,      43,      44,      45,      46,      47,         48,
    49,      50,      51,      INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,    INVALID,
    INVALID, INVALID, INVALID, INVALID};

/* This function decodes a base64 ByteString. Base64 ByteString shall be null terminated.
 * Otherwise, the result is undefined.*/
static bool base64decode(const char* input, unsigned char* out, size_t* outLen)
{
    if (NULL == input || NULL == out || NULL == outLen)
    {
        return false;
    }

    const char* end = input + strlen(input);
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    bool return_status = true;

    while (return_status && input < end)
    {
        unsigned char c = d[(int) *input];
        input++;

        switch (c)
        {
        case WHITESPACE:
            break; /* skip whitespace */
        case INVALID:
            return_status = false; /* invalid input, return error */
            break;
        case EQUALS: /* pad character, end of data */
            input = end;
            break;
        default:
            SOPC_ASSERT(c < 64);
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4)
            {
                len += 3;
                if (len > *outLen)
                {
                    return_status = false; /* buffer overflow */
                }
                else
                {
                    *(out++) = (buf >> 16) & 255;
                    *(out++) = (buf >> 8) & 255;
                    *(out++) = buf & 255;
                    buf = 0;
                    iter = 0;
                }
            }
            break;
        }
    }

    if (return_status && iter == 3)
    {
        if ((len += 2) > *outLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(out++) = (buf >> 10) & 255;
            *(out++) = (buf >> 2) & 255;
        }
    }
    else if (return_status && iter == 2)
    {
        if (++len > *outLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(out++) = (buf >> 4) & 255;
        }
    }

    if (return_status)
    {
        *outLen = len; /* modify to reflect the actual output size */
    }

    return return_status;
}

SOPC_ReturnStatus SOPC_HelperDecode_Base64(const char* input, unsigned char* out, size_t* outLen)
{
    bool res = base64decode(input, out, outLen);
    return (res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

SOPC_ReturnStatus SOPC_HelperDecode_Hex(const char* src, unsigned char* dst, size_t n)
{
    int res = unhexlify(src, dst, n);
    return (0 < res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

SOPC_ReturnStatus SOPC_HelperDecode_Base64_GetPaddingLength(const char* input, size_t* outLen)
{
    size_t padding_length = 0;
    if (NULL == input)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool first_occ = false;
    int i = (int) (strlen(input)) - 1;
    while ((0 <= i) && !first_occ)
    {
        if ('=' == input[i])
        {
            padding_length++;
        }
        else
        {
            first_occ = true;
        }
        i--;
    }
    if (0 != padding_length && 1 != padding_length && 2 != padding_length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        *outLen = padding_length;
        return SOPC_STATUS_OK;
    }
}
