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


#include "secret_buffer.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* Creates a new SecretBuffer from an exposed buffer and a length. Copies the buffer, so ExposedBuffer
 *  can be cleared and de-allocated.
 * You must call SecretBuffer_DeleteClear() for each call to _New
 */
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len)
{
    SecretBuffer *sec = NULL;

    if(NULL != buf)
    {
        sec = SecretBuffer_New(len);
        if(NULL != sec && NULL != sec->buf)
            memcpy(sec->buf, buf, len);
    }

    return sec;
}
SecretBuffer *SecretBuffer_New(uint32_t len)
{
    SecretBuffer *sec = NULL;

    if(0 != len)
    {
        sec = malloc(sizeof(SecretBuffer));
        if(NULL != sec)
        {
            sec->len = len;
            sec->buf = malloc(len);
            if(NULL == sec->buf)
            {
                free(sec->buf);
                sec = NULL;
            }
        }
    }

    return sec;
}

void SecretBuffer_DeleteClear(SecretBuffer *sec)
{
    if(NULL != sec)
    {
        if(sec->buf)
        {
            memset(sec->buf, 0, sec->len);
            free(sec->buf);
        }
        free(sec);
    }
}

uint32_t SecretBuffer_GetLength(const SecretBuffer *sec)
{
    if(NULL == sec)
        return 0;
    return sec->len;
}

/* Exposes the secret buffer to a contiguous buffer.
 * Each call to Expose should be followed by a call to Unexpose.
 * SecretBuffer should not be destroyed before the call to Unexpose.
 */
ExposedBuffer *SecretBuffer_Expose(const SecretBuffer *sec)
{
    if(NULL != sec)
        return sec->buf;
    return NULL;
}

void SecretBuffer_Unexpose(ExposedBuffer *buf)
{
    (void) buf;
}

