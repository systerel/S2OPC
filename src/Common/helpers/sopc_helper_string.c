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

#include "sopc_helper_string.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h> /* strtoul */
#include <string.h>

#include "sopc_mem_alloc.h"

int SOPC_strncmp_ignore_case(const char* s1, const char* s2, size_t size)
{
    int lc1, lc2;
    size_t idx;
    int res = -1000;
    if (NULL == s1 || NULL == s2)
    {
        return res;
    }

    res = 0;
    for (idx = 0; idx < size && res == 0; idx++)
    {
        lc1 = tolower((unsigned char) s1[idx]);
        lc2 = tolower((unsigned char) s2[idx]);
        if (lc1 < lc2)
        {
            res = -1;
        }
        else if (lc1 > lc2)
        {
            res = +1;
        }
        else if (lc1 == '\0')
        {
            // In case we reached end of both strings, stop comparison here
            return res;
        }
    }
    return res;
}

int SOPC_strcmp_ignore_case(const char* s1, const char* s2)
{
    int res = -1000;
    if (NULL == s1 || NULL == s2)
    {
        return res;
    }

    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    if (s1_len == s2_len)
    {
        return SOPC_strncmp_ignore_case(s1, s2, s1_len);
    }
    else
    {
        if (s1_len < s2_len)
        {
            res = -1;
        }
        else
        {
            res = +1;
        }
    }
    return res;
}

int SOPC_strcmp_ignore_case_alt_end(const char* s1, const char* s2, char endCharacter)
{
    int lc1, lc2;
    int endChar = tolower((unsigned char) endCharacter);
    size_t idx;
    int res = -1000;
    bool lc1_is_endchar = false;
    bool lc2_is_endchar = false;

    if (NULL == s1 || NULL == s2)
    {
        return res;
    }

    res = 0;
    for (idx = 0; res == 0; idx++)
    {
        lc1 = tolower((unsigned char) s1[idx]);
        lc2 = tolower((unsigned char) s2[idx]);
        lc1_is_endchar = endChar == lc1 || '\0' == lc1;
        lc2_is_endchar = endChar == lc2 || '\0' == lc2;

        if (!lc1_is_endchar && !lc2_is_endchar)
        {
            if (lc1 < lc2)
            {
                res = -1;
            }
            else if (lc1 > lc2)
            {
                res = +1;
            }
        }
        else
        {
            if (lc1_is_endchar && lc2_is_endchar)
            {
                // In case we reached end of both strings, stop comparison here
                return res;
            }
            else if (lc1_is_endchar)
            {
                res = -1;
            }
            else
            {
                res = +1;
            }
        }
    }
    return res;
}

SOPC_ReturnStatus SOPC_strtouint8_t(const char* sz, uint8_t* n, int base, char cEnd)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    char* pEnd = NULL;
    /* ULONG_MAX is at least 2^32 - 1, so it will always be possible to store an uint8_t inside */
    unsigned long int value = 0;

    if (NULL == sz || NULL == n)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* 10 and 16 are the only supported bases */
    if (10 != base && 16 != base)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* ULONG_MAX is at least 2^32 - 1 (see C99 §5.2.4.2.1 Sizes of integer types)
         *  so it will always be possible to store an uint8_t inside value */
        value = strtoul(sz, &pEnd, base);
        if (NULL == pEnd || pEnd == sz || *pEnd != cEnd || value > UINT8_MAX)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            *n = (uint8_t) value;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_strtouint16_t(const char* sz, uint16_t* n, int base, char cEnd)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    char* pEnd = NULL;
    unsigned long int value = 0;

    if (NULL == sz || NULL == n)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* 10 and 16 are the only supported bases */
    if (10 != base && 16 != base)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* ULONG_MAX is at least 2^32 - 1 (see C99 §5.2.4.2.1 Sizes of integer types)
         *  so it will always be possible to store an uint16_t inside value */
        value = strtoul(sz, &pEnd, base);
        if (NULL == pEnd || pEnd == sz || *pEnd != cEnd || value > UINT16_MAX)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            *n = (uint16_t) value;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_strtouint32_t(const char* sz, uint32_t* n, int base, char cEnd)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    char* pEnd = NULL;
    unsigned long int value = 0;

    if (NULL == sz || NULL == n)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* 10 and 16 are the only supported bases */
    if (10 != base && 16 != base)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* ULONG_MAX is at least 2^32 - 1 (see C99 §5.2.4.2.1 Sizes of integer types)
         *  so it will always be possible to store an uint32_t inside value */
        errno = 0;
        value = strtoul(sz, &pEnd, base);
        if (NULL == pEnd || pEnd == sz || *pEnd != cEnd || (ULONG_MAX == value && ERANGE == errno) ||
            value > UINT32_MAX)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            *n = (uint32_t) value;
        }
    }

    return status;
}

bool SOPC_strtoint(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[21];

    if (NULL == dest || len == 0 || len > (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    errno = 0;

    char* endptr;
    long long int val = strtoll(buf, &endptr, 10);

    if (endptr != (buf + len))
    {
        return false;
    }

    bool res = true;
    if (width == 8 && val >= INT8_MIN && val <= INT8_MAX)
    {
        *((int8_t*) dest) = (int8_t) val;
    }
    else if (width == 16 && val >= INT16_MIN && val <= INT16_MAX)
    {
        *((int16_t*) dest) = (int16_t) val;
    }
    else if (width == 32 && val >= INT32_MIN && val <= INT32_MAX)
    {
        *((int32_t*) dest) = (int32_t) val;
    }
    else if (width == 64 && val >= INT64_MIN && val <= INT64_MAX &&
             !((LLONG_MAX == val || LLONG_MIN == val) && ERANGE == errno))
    {
        *((int64_t*) dest) = (int64_t) val;
    }
    else
    {
        // Invalid width and/or out of bounds value
        res = false;
    }

    return res;
}

bool SOPC_strtouint(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[21];

    if (NULL == dest || len == 0 || len > (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    errno = 0;
    unsigned long long int val = strtoull(buf, &endptr, 10);

    if (endptr != (buf + len))
    {
        return false;
    }

    bool res = true;
    if (width == 8 && val <= UINT8_MAX)
    {
        *((uint8_t*) dest) = (uint8_t) val;
    }
    else if (width == 16 && val <= UINT16_MAX)
    {
        *((uint16_t*) dest) = (uint16_t) val;
    }
    else if (width == 32 && val <= UINT32_MAX)
    {
        *((uint32_t*) dest) = (uint32_t) val;
    }
    else if (width == 64 && val <= UINT64_MAX && !(ULLONG_MAX == val && ERANGE == errno))
    {
        *((uint64_t*) dest) = (uint64_t) val;
    }
    else
    {
        // Invalid width and/or out of bounds value
        res = false;
    }

    return res;
}

bool SOPC_strtodouble(const char* data, size_t len, uint8_t width, void* dest)
{
    char buf[340];

    if (NULL == dest || len <= 0 || len > (sizeof(buf) / sizeof(char) - 1))
    {
        return false;
    }

    memcpy(buf, data, len);
    buf[len] = '\0';

    char* endptr;
    errno = 0;
    double val = strtod(buf, &endptr);

    if (endptr != (buf + len))
    {
        return false;
    }

    bool res = true;
    if (width == 32 && val >= -FLT_MAX && val <= FLT_MAX && ERANGE != errno)
    {
        *((float*) dest) = (float) val;
    }
    else if (width == 64 && val >= -DBL_MAX && val <= DBL_MAX && ERANGE != errno)
    {
        *((double*) dest) = val;
    }
    else
    {
        // Invalid width and/or out of bounds value
        res = false;
    }

    return res;
}

char* SOPC_strdup(const char* s)
{
    if (NULL == s)
    {
        return NULL;
    }

    size_t len = strlen(s);
    char* res = SOPC_Calloc(1 + len, sizeof(char));

    if (res == NULL)
    {
        return NULL;
    }

    memcpy(res, s, len * sizeof(char));
    return res;
}
