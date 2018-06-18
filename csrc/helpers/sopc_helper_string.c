/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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

#include <ctype.h>
#include <errno.h>
#include <limits.h>

int SOPC_strncmp_ignore_case(const char* s1, const char* s2, size_t size)
{
    int lc1, lc2;
    size_t idx;
    int res = -1000;
    if (NULL != s1 && NULL != s2)
    {
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
        if (NULL == pEnd || pEnd == sz || *pEnd != cEnd || value > UINT32_MAX ||
            (ULONG_MAX == value && ERANGE == errno))
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
