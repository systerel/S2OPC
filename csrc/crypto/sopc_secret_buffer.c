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


#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sopc_secret_buffer.h"


struct SOPC_SecretBuffer
{
    uint32_t len; // Mandatory
    uint8_t *buf;
};


SOPC_SecretBuffer *SOPC_SecretBuffer_NewFromExposedBuffer(SOPC_ExposedBuffer *buf, uint32_t len)
{
    SOPC_SecretBuffer *sec = NULL;

    if(NULL != buf)
    {
        sec = SOPC_SecretBuffer_New(len);
        if(NULL != sec && NULL != sec->buf)
            memcpy(sec->buf, buf, len);
    }

    return sec;
}

SOPC_SecretBuffer *SOPC_SecretBuffer_New(uint32_t len)
{
    SOPC_SecretBuffer *sec = NULL;

    if(0 != len)
    {
        sec = malloc(sizeof(SOPC_SecretBuffer));
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

void SOPC_SecretBuffer_DeleteClear(SOPC_SecretBuffer *sec)
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

uint32_t SOPC_SecretBuffer_GetLength(const SOPC_SecretBuffer *sec)
{
    if(NULL == sec)
        return 0;
    return sec->len;
}

SOPC_ExposedBuffer *SOPC_SecretBuffer_Expose(SOPC_SecretBuffer *sec)
{
    if(NULL != sec)
        return sec->buf;
    return NULL;
}

void SOPC_SecretBuffer_Unexpose(SOPC_ExposedBuffer *buf, SOPC_SecretBuffer *sec)
{
    (void) buf;
    (void) sec;
}

