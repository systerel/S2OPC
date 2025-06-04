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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_helper_encode.h"
#include "sopc_mem_alloc.h"

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

// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
static int hexlify(const unsigned char* src, char* dst, size_t n)
{
    SOPC_ASSERT(n <= INT32_MAX);
    size_t i;
    char buffer[3];
    int res = 0;

    if (!src || !dst)
        return -1;

    for (i = 0; i < n; ++i)
    {
        res = sprintf(buffer, "%02hhx", src[i]); // sprintf copies the last \0 too
        SOPC_ASSERT(2 == res);
        memcpy(dst + 2 * i, buffer, 2);
    }

    return (int) n;
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

/**
 * \brief  Get the number of padding characters for base64 ('=').
 *
 * \param input     A valid pointer to the input.
 *
 * \param outLen    the number of padding characters (0, 1 or 2)
 *
 * \return   SOPC_STATUS_OK when successful otherwise SOPC_STATUS_INVALID_PARAMETERS if
 *           \p input is NULL, if there is one or multiple characters after padding or has not a number of padding
 * characters equal to 0, 1 or 2.
 */

static SOPC_ReturnStatus decode_base64_get_paddinglength(const char* input, size_t* outLen)
{
    size_t padding_length = 0;
    if (NULL == input)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    int i = (int) (strlen(input)) - 1;

    while ((0 <= i))
    {
        if ('=' == input[i])
        {
            if (input[i + 1] != '\000' && input[i + 1] != '=') /* verify that there is no character after an equal */
            {
                return SOPC_STATUS_INVALID_PARAMETERS;
            }

            padding_length++;
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

/* Using https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C
 * to encode base64 */
static bool base64encode(const SOPC_Byte* pInput, size_t pInputLen, char* ppOut, size_t pOutLen)
{
    const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t pOutIndex = 0;
    uint32_t n = 0;
    size_t padCount = pInputLen % 3;
    uint8_t n0;
    uint8_t n1;
    uint8_t n2;
    uint8_t n3;

    /* increment over the length of the string, three characters at a time */
    for (size_t x = 0; x < pInputLen; x += 3)
    {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = ((uint32_t) pInput[x]) << 16; // parenthesis needed, compiler depending on flags can do the shifting before
                                          // conversion to uint32_t, resulting to 0

        if ((x + 1) < pInputLen)
        {
            n += ((uint32_t) pInput[x + 1]) << 8; // parenthesis needed, compiler depending on flags can do the shifting
                                                  // before conversion to uint32_t, resulting to 0
        }

        if ((x + 2) < pInputLen)
        {
            n += pInput[x + 2];
        }

        /* this 24-bit number gets separated into four 6-bit numbers */
        n0 = (uint8_t)(n >> 18) & 63;
        n1 = (uint8_t)(n >> 12) & 63;
        n2 = (uint8_t)(n >> 6) & 63;
        n3 = (uint8_t) n & 63;

        /*
         * if we have one byte available, then its encoding is spread
         * out over two characters
         */
        if (pOutIndex >= pOutLen)
        {
            return false; /* indicate failure: buffer too small */
        }
        ppOut[pOutIndex] = base64chars[n0];
        pOutIndex++;
        if (pOutIndex >= pOutLen)
        {
            return false; /* indicate failure: buffer too small */
        }
        ppOut[pOutIndex] = base64chars[n1];
        pOutIndex++;

        /*
         * if we have only two bytes available, then their encoding is
         * spread out over three chars
         */
        if ((x + 1) < pInputLen)
        {
            if (pOutIndex >= pOutLen)
            {
                return false; /* indicate failure: buffer too small */
            }
            ppOut[pOutIndex] = base64chars[n2];
            pOutIndex++;
        }

        /*
         * if we have all three bytes available, then their encoding is spread
         * out over four characters
         */
        if ((x + 2) < pInputLen)
        {
            if (pOutIndex >= pOutLen)
            {
                return false; /* indicate failure: buffer too small */
            }
            ppOut[pOutIndex] = base64chars[n3];
            pOutIndex++;
        }
    }

    /*
     * create and add padding that is required if we did not have a multiple of 3
     * number of characters available
     */
    if (padCount > 0)
    {
        for (; padCount < 3; padCount++)
        {
            if (pOutIndex >= pOutLen)
            {
                return false; /* indicate failure: buffer too small */
            }
            ppOut[pOutIndex] = '=';
            pOutIndex++;
        }
    }
    if (pOutIndex >= pOutLen)
    {
        return false; /* indicate failure: buffer too small */
    }
    ppOut[pOutIndex] = 0;
    return true; /* indicate success */
}

SOPC_ReturnStatus SOPC_HelperEncode_Base64(const SOPC_Byte* pInput, size_t inputLen, char** ppOut, size_t* pOutLen)
{
    if (NULL == pInput || NULL == ppOut || NULL == pOutLen)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *pOutLen = 1 + 4 * ((inputLen + 2) / 3); // + 1 for '\0'
    if (INT32_MAX < *pOutLen)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    *ppOut = (char*) SOPC_Calloc(*pOutLen, sizeof(char));
    if (NULL == *ppOut)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    bool res = base64encode(pInput, inputLen, *ppOut, *pOutLen);
    return (res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
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
static bool base64decode(const char* input, unsigned char** ppOut, size_t* outLen)
{
    if (NULL == input || NULL == outLen)
    {
        return false;
    }

    size_t inputLen = strlen(input);
    const char* end = input + inputLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    size_t paddingLength = 0;
    bool return_status = true;
    SOPC_ReturnStatus status = decode_base64_get_paddinglength(input, &paddingLength);

    if (SOPC_STATUS_OK != status)
    {
        return false;
    }

    size_t resIter = (inputLen - paddingLength) % 4;

    if (resIter > 0)
    {
        resIter--;
    }

    size_t expectedLen = 3 * ((inputLen - paddingLength) / 4) + resIter;

    unsigned char* pBuffer = SOPC_Calloc(expectedLen + 1, sizeof(char)); // +1 for \0 end character

    if (NULL == pBuffer)
    {
        return false;
    }

    unsigned char* pHeadBuffer = pBuffer;

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
                if (len > expectedLen)
                {
                    return_status = false; /* buffer overflow */
                }
                else
                {
                    *(pBuffer++) = (buf >> 16) & 255;
                    *(pBuffer++) = (buf >> 8) & 255;
                    *(pBuffer++) = buf & 255;
                    buf = 0;
                    iter = 0;
                }
            }
            break;
        }
    }

    if (return_status && iter == 3)
    {
        if ((len += 2) > expectedLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(pBuffer++) = (buf >> 10) & 255;
            *(pBuffer++) = (buf >> 2) & 255;
        }
    }
    else if (return_status && iter == 2)
    {
        if (++len > expectedLen)
        {
            return_status = false; /* buffer overflow */
        }
        else
        {
            *(pBuffer++) = (buf >> 4) & 255;
        }
    }

    if (expectedLen != len)
    {
        return_status = false;
    }

    if (return_status)
    {
        pHeadBuffer[expectedLen] = '\0';
        *ppOut = pHeadBuffer;
        *outLen = expectedLen;
    }
    else
    {
        *ppOut = NULL;
        *outLen = 0;
        return_status = false;
        SOPC_Free(pHeadBuffer);
    }

    return return_status;
}

SOPC_ReturnStatus SOPC_HelperDecode_Base64(const char* pInput, unsigned char** ppOut, size_t* pOutLen)
{
    bool res = base64decode(pInput, ppOut, pOutLen);
    return (res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

SOPC_ReturnStatus SOPC_HelperEncode_Hex(const unsigned char* pInput, char* pOut, size_t inputLen)
{
    int res = hexlify(pInput, pOut, inputLen);
    return (0 < res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

SOPC_ReturnStatus SOPC_HelperDecode_Hex(const char* pInput, unsigned char* pOut, size_t outputLen)
{
    int res = unhexlify(pInput, pOut, outputLen);
    return (0 < res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}
