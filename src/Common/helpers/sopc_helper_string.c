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

static bool parseTwoDigitsUint8(const char* startPointer, size_t len, const char endChar, uint8_t* pOut)
{
    assert(NULL != startPointer);
    assert(NULL != pOut);

    if ((len > 2 && startPointer[2] != endChar) || len < 2)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_strtouint8_t(startPointer, pOut, 10, endChar);
    if (SOPC_STATUS_OK != status)
    {
        return false;
    }

    return true;
}

bool SOPC_stringToDateTime(const char* datetime,
                           size_t len,
                           int16_t* pYear,
                           uint8_t* pMonth,
                           uint8_t* pDay,
                           uint8_t* pHour,
                           uint8_t* pMinute,
                           uint8_t* pSecond,
                           double* pSecondAndFrac,
                           bool* p_UTC,
                           bool* pUTC_neg_off,
                           uint8_t* pUTC_hour_off,
                           uint8_t* pUTC_min_off)
{
    int16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    double secondAndFraction = 0.0;
    bool utc = true;
    bool negativeOffset = false;
    uint8_t utc_hour_off = 0;
    uint8_t utc_min_off = 0;

    // Check input: minimum length '<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>'
    if (NULL == datetime || len < 19)
    {
        return false;
    }

    const char* currentPointer = datetime;
    size_t remainingLength = len;

    /*
     * Parse year:
     * -?([1-9][0-9]{3,}|0[0-9]{3})-
     */
    // Check if year is prefixed by '-'
    const char* endPointer = NULL;

    // Manage the case of negative year by searching from next character
    // since at least 4 digits are expected in both cases
    endPointer = memchr(currentPointer + 1, '-', remainingLength - 1);
    if (NULL == endPointer || endPointer - currentPointer < (*currentPointer == '-' ? 5 : 4))
    {
        // End character not found or year < 4 digits year
        return false;
    }

    bool res = SOPC_strtoint(currentPointer, (size_t)(endPointer - currentPointer), 16, &year);
    if (!res)
    {
        return false;
    }
    endPointer++; // remove '-' end separator
    assert(endPointer > currentPointer);
    remainingLength -= (size_t)(endPointer - currentPointer);
    currentPointer = endPointer;

    if (NULL != pYear)
    {
        *pYear = year;
    }

    /*
     * Parse Month:
     * (0[1-9]|1[0-2])-
     */
    res = parseTwoDigitsUint8(currentPointer, remainingLength, '-', &month);
    if (!res || month < 1 || month > 12)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    if (NULL != pMonth)
    {
        *pMonth = month;
    }

    /*
     * Parse Day:
     * (0[1-9]|[12][0-9]|3[01])T
     */
    res = parseTwoDigitsUint8(currentPointer, remainingLength, 'T', &day);
    if (!res || day < 1 || day > 31)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    if (NULL != pDay)
    {
        *pDay = day;
    }

    /*
     * Parse Hour:
     * ([01][0-9]|2[0-4]):
     */
    res = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &hour);
    // Note: accept hour = 24 for case 24:00:00.0 to be check after parsing minutes and seconds
    if (!res || hour > 24)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    if (NULL != pHour)
    {
        *pHour = hour;
    }

    /*
     * Parse Minutes:
     * ([0-5][0-9]):
     */
    res = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &minute);

    if (!res || minute > 59)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    if (NULL != pMinute)
    {
        *pMinute = minute;
    }

    /*
     * Parse Seconds (whole number part):
     * ([0-5][0-9]):
     */
    if (remainingLength < 2)
    {
        return false;
    }
    // Use SOPC_strtouint to allow string ending without '\0' and avoid access out of string memory bounds
    res = SOPC_strtouint(currentPointer, 2, 8, &second);
    if (!res || second > 59)
    {
        return false;
    }

    if (NULL != pSecond)
    {
        *pSecond = second;
    }

    // Initialize seconds without fraction first (in case of no fraction present)
    secondAndFraction = (double) second;

    // Check 24:00:00 special case
    if (24 == hour && (0 != minute || 0 != second))
    {
        // Minutes and seconds shall be 0 when hour is 24
        if (NULL != pMinute)
        {
            *pMinute = 0;
        }

        if (NULL != pSecond)
        {
            *pSecond = 0;
        }

        return false;
    }

    if (2 == remainingLength)
    {
        // Whole datetime string consume without any error

        // Set the seconds with fraction since we will not parse a fraction later
        if (NULL != pSecondAndFrac)
        {
            *pSecondAndFrac = secondAndFraction;
        }

        return true;
    }

    /*
     * Parse Seconds with fraction (if applicable)
     * [0-5][0-9](\.[0-9]+)?
     */

    // Check if there is a fraction of second
    if ('.' == currentPointer[2])
    {
        // Search for end character starting from character after the '.'
        endPointer = &currentPointer[3];
        size_t localRemLength = remainingLength - 3;

        while (localRemLength > 0 && *endPointer >= '0' && *endPointer <= '9')
        {
            // Check all digits are 0 if hour was 24
            if (24 == hour && '0' != *endPointer)
            {
                return false;
            }
            endPointer++;
            localRemLength--;
        }

        // Parse the seconds with fraction as a double value
        res = SOPC_strtodouble(currentPointer, (size_t)(endPointer - currentPointer), 64, &secondAndFraction);
        // Note: we do not need to check for actual value since we already controlled each digit individually
        // If something went wrong it is either due to SOPC_strtodouble or due to double representation
        if (!res)
        {
            return false;
        }
        remainingLength -= (size_t)(endPointer - currentPointer);
        currentPointer = endPointer;
    }
    else
    {
        remainingLength -= 2;
        currentPointer += 2;
    }

    if (NULL != pSecondAndFrac)
    {
        *pSecondAndFrac = secondAndFraction;
    }

    if (0 != remainingLength && 'Z' != *currentPointer)
    {
        // Parse offset sign
        if ('-' == *currentPointer)
        {
            negativeOffset = true;
        }
        else if ('+' != *currentPointer)
        {
            return false;
        }

        remainingLength--;
        currentPointer++;

        /*
         * Parse Hour Offset:
         * (0[0-9]|1[0-4]):
         */
        res = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &utc_hour_off);
        // Note: accept hour = 14 for case 14:00 to be check after parsing minutes and seconds
        if (!res || utc_hour_off > 14)
        {
            return false;
        }
        remainingLength -= 3;
        currentPointer += 3;

        /*
         * Parse Minute Offset
         * ([0-5][0-9]):
         */
        if (remainingLength < 2)
        {
            return false;
        }
        // Use SOPC_strtouint to allow string ending without '\0' and avoid access out of string memory bounds
        res = SOPC_strtouint(currentPointer, 2, 8, &utc_min_off);
        // Check for special case of 14:00 which is maximum offset value
        if (!res || utc_min_off > 59 || (14 == utc_hour_off && 0 != utc_min_off))
        {
            return false;
        }

        remainingLength -= 2;
        currentPointer += 2;

        // Set UTC flag regarding offset
        utc = (0 == utc_hour_off && 0 == utc_min_off);
    }
    else if ('Z' == *currentPointer)
    {
        remainingLength--;
        currentPointer++;
    }

    if (NULL != p_UTC)
    {
        *p_UTC = utc;
    }
    if (NULL != pUTC_neg_off)
    {
        *pUTC_neg_off = negativeOffset;
    }
    if (NULL != pUTC_hour_off)
    {
        *pUTC_hour_off = utc_hour_off;
    }
    if (NULL != pUTC_min_off)
    {
        *pUTC_min_off = utc_min_off;
    }

    return 0 == remainingLength;
}
