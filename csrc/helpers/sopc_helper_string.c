/*
 *  Copyright (C) 2018 Systerel and others.
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
