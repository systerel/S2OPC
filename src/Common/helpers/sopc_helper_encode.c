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
#include "sopc_common_constants.h"
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

// Scan src up to maxLen and return its length if a '\0' is found
static size_t sopc_strnlen(const char* src, size_t maxLen)
{
    const void* p = memchr(src, '\0', maxLen);
    return p ? (size_t)((const char*) p - src) : maxLen;
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

/* The following functions have been extracted and slightly adapted from MbedTLS library. */

/** Output buffer too small. */
#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL -0x002A
/** Invalid character in input. */
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER -0x002C

static const unsigned char base64_enc_map[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static const unsigned char base64_dec_map[128] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 62,
    127, 127, 127, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  127, 127, 127, 64,  127, 127, 127, 0,
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  127, 127, 127, 127, 127, 127, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  127, 127, 127, 127, 127};

#define BASE64_SIZE_T_MAX ((size_t) -1) /* SIZE_T_MAX is not standard */

/*
 * Encode a buffer into base64 format
 */
static int mbedtls_base64_encode(unsigned char* dst, size_t dlen, size_t* olen, const unsigned char* src, size_t slen)
{
    size_t i, n;
    int C1, C2, C3;
    unsigned char* p;

    if (slen == 0)
    {
        *olen = 0;
        return (0);
    }

    n = slen / 3 + (slen % 3 != 0);

    if (n > (BASE64_SIZE_T_MAX - 1) / 4)
    {
        *olen = BASE64_SIZE_T_MAX;
        return (MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL);
    }

    n *= 4;

    if (dst == NULL || dlen < n + 1)
    {
        *olen = n + 1; // +1 for the null terminator
        return (MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL);
    }

    n = (slen / 3) * 3;

    for (i = 0, p = dst; i < n; i += 3)
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if (i < slen)
    {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if ((i + 1) < slen)
            *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else
            *p++ = '=';

        *p++ = '=';
    }

    *olen = (size_t)(p - dst + 1); // +1 for the null terminator
    *p = '\0';

    return (0);
}

static int mbedtls_base64_decode(unsigned char** dst, size_t* olen, const unsigned char* src, size_t slen)
{
    size_t i, n, expectedLen = 0;
    uint32_t j, x;
    unsigned char* p;

    i = n = j = 0;
    /* First pass: check for validity and get output length */
    while (i < slen)
    {
        /* Skip spaces before checking for EOL */
        x = 0;
        while (i < slen && src[i] == ' ')
        {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if (i < slen)
        {
            if (((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n') || (src[i] == '\n'))
            {
                /* Do nothing */
            }
            else
            {
                /* Space inside a line is an error */
                if (x != 0)
                    return (MBEDTLS_ERR_BASE64_INVALID_CHARACTER);

                if (src[i] == '=' && ++j > 2)
                    return (MBEDTLS_ERR_BASE64_INVALID_CHARACTER);

                if (src[i] > 127 || base64_dec_map[src[i]] == 127)
                    return (MBEDTLS_ERR_BASE64_INVALID_CHARACTER);

                if (base64_dec_map[src[i]] < 64 && j != 0)
                    return (MBEDTLS_ERR_BASE64_INVALID_CHARACTER);

                n++;
            }
        }
        ++i;
    }

    if (n == 0)
    {
        *olen = 0;
        return (0);
    }

    n = ((n * 6) + 7) >> 3;
    n -= j;
    expectedLen = n + 1; // +1 for the null terminator

    /* Allocate the necessary buffer */
    unsigned char* pBuffer = SOPC_Calloc(expectedLen, sizeof(char));
    if (pBuffer == NULL)
    {
        *olen = 0;
        return (SOPC_STATUS_OUT_OF_MEMORY);
    }

    for (j = 3, n = x = 0, p = pBuffer; i > 0; i--, src++)
    {
        if (*src != '\r' && *src != '\n' && *src != ' ')
        {
            j -= (base64_dec_map[*src] == 64);
            x = (x << 6) | (base64_dec_map[*src] & 0x3F);

            if (++n == 4)
            {
                n = 0;
                if (j > 0)
                    *p++ = (unsigned char) (x >> 16);
                if (j > 1)
                    *p++ = (unsigned char) (x >> 8);
                if (j > 2)
                    *p++ = (unsigned char) (x);
            }
        }
    }
    *p++ = '\0'; // Null-terminate the output buffer
    *olen = (size_t)(p - pBuffer);
    /* check that lenght is the expected one */
    SOPC_ASSERT(*olen == expectedLen);

    *dst = pBuffer; // Assign the allocated buffer to the output pointer

    return (0);
}

SOPC_ReturnStatus SOPC_HelperDecode_Base64(const char* pInput, unsigned char** ppOut, size_t* pOutLen)
{
    if (NULL == pInput || NULL == ppOut || NULL == pOutLen)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    size_t inputLen = sopc_strnlen(pInput, (size_t) SOPC_DEFAULT_MAX_STRING_LENGTH);
    int ret = mbedtls_base64_decode(ppOut, pOutLen, (const unsigned char*) pInput, inputLen);
    if (ret != 0)
    {
        SOPC_Free(*ppOut);
        *ppOut = NULL;
        *pOutLen = 0;
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperEncode_Base64(const SOPC_Byte* pInput, size_t inputLen, char** ppOut, size_t* pOutLen)
{
    if (NULL == pInput || NULL == ppOut || NULL == pOutLen)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t buffer_len = 0;

    // First call to get required output length
    int ret = mbedtls_base64_encode(NULL, 0, &buffer_len, (const unsigned char*) pInput, inputLen);
    if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL)
    {
        return SOPC_STATUS_NOK;
    }

    // Allocate buffer
    *ppOut = SOPC_Calloc(buffer_len, sizeof(char));
    if (NULL == *ppOut)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Actual encoding
    ret = mbedtls_base64_encode((unsigned char*) *ppOut, buffer_len, pOutLen, (const unsigned char*) pInput, inputLen);
    SOPC_ASSERT(*pOutLen == buffer_len);
    if (ret != 0)
    {
        SOPC_Free(*ppOut);
        *ppOut = NULL;
        *pOutLen = 0;
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
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
