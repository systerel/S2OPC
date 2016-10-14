/*
 * SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
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

