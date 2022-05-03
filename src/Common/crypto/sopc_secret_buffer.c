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

#include <stddef.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

struct SOPC_SecretBuffer
{
    uint32_t len; // Mandatory
    uint8_t* buf;
};

SOPC_SecretBuffer* SOPC_SecretBuffer_NewFromExposedBuffer(SOPC_ExposedBuffer* buf, uint32_t len)
{
    SOPC_SecretBuffer* sec = NULL;

    if (NULL != buf)
    {
        sec = SOPC_SecretBuffer_New(len);
        if (NULL != sec && NULL != sec->buf)
            memcpy(sec->buf, buf, len);
    }

    return sec;
}

SOPC_SecretBuffer* SOPC_SecretBuffer_NewFromFile(const char* path)
{
    SOPC_Buffer* contents = NULL;

    if (SOPC_Buffer_ReadFile(path, &contents) != SOPC_STATUS_OK)
    {
        return NULL;
    }

    SOPC_SecretBuffer* sec = SOPC_Calloc(1, sizeof(SOPC_SecretBuffer));

    if (sec == NULL)
    {
        SOPC_Buffer_Delete(contents);
        return NULL;
    }

    // "Steal" the data from the buffer. This is a bit on the hacky side, but
    // it allows reusing existing code.
    sec->len = contents->length;
    sec->buf = contents->data;
    contents->data = NULL;
    SOPC_Buffer_Delete(contents);

    return sec;
}

SOPC_SecretBuffer* SOPC_SecretBuffer_New(uint32_t len)
{
    SOPC_SecretBuffer* sec = NULL;

    if (0 != len)
    {
        sec = SOPC_Malloc(sizeof(SOPC_SecretBuffer));
        if (NULL != sec)
        {
            sec->len = len;
            sec->buf = SOPC_Malloc((size_t) len);
            if (NULL == sec->buf)
            {
                SOPC_Free(sec);
                sec = NULL;
            }
        }
    }

    return sec;
}

void SOPC_SecretBuffer_DeleteClear(SOPC_SecretBuffer* sec)
{
    if (NULL != sec)
    {
        if (sec->buf)
        {
            memset(sec->buf, 0, sec->len);
            SOPC_Free(sec->buf);
        }
        SOPC_Free(sec);
    }
}

uint32_t SOPC_SecretBuffer_GetLength(const SOPC_SecretBuffer* sec)
{
    if (NULL == sec)
        return 0;
    return sec->len;
}

const SOPC_ExposedBuffer* SOPC_SecretBuffer_Expose(const SOPC_SecretBuffer* sec)
{
    if (NULL != sec)
        return sec->buf;
    return NULL;
}

void SOPC_SecretBuffer_Unexpose(const SOPC_ExposedBuffer* buf, const SOPC_SecretBuffer* sec)
{
    SOPC_UNUSED_ARG(buf);
    SOPC_UNUSED_ARG(sec);
}

SOPC_ExposedBuffer* SOPC_SecretBuffer_ExposeModify(SOPC_SecretBuffer* sec)
{
    if (NULL != sec)
        return sec->buf;
    return NULL;
}

void SOPC_SecretBuffer_UnexposeModify(SOPC_ExposedBuffer* buf, SOPC_SecretBuffer* sec)
{
    SOPC_UNUSED_ARG(buf);
    SOPC_UNUSED_ARG(sec);
}
