/*
 * SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
 */


#include "secret_buffer.h"
#include <stdlib.h>
#include <string.h>

/* Creates a new SecretBuffer from an exposed buffer and a length. Copies the buffer, so ExposedBuffer
 *  can be cleared and de-allocated.
 * You must call SecretBuffer_DeleteClear() for each call to _New
 */
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len)
{
    SecretBuffer *sec = 0;

    if(0 != buf && 0 != len)
    {
        sec = malloc(sizeof(SecretBuffer));
        if(0 != sec)
        {
            sec->len = len;
            sec->buf = malloc(len);
            if(0 != sec->buf)
                memcpy(sec->buf, buf, len);
            else
            {
                free(sec->buf);
                sec = 0;
            }
        }
    }

    return sec;
}
void SecretBuffer_DeleteClear(SecretBuffer *sec)
{
    if(0 != sec)
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
    if(0 == sec)
        return 0;
    return sec->len;
}

/* Exposes the secret buffer to a contiguous buffer.
 * Each call to Expose should be followed by a call to Unexpose.
 * SecretBuffer should not be destroyed before the call to Unexpose.
 */
ExposedBuffer *SecretBuffer_Expose(const SecretBuffer *sec)
{
    if(0 != sec)
        return sec->buf;
    return 0;
}

void SecretBuffer_Unexpose(ExposedBuffer *buf)
{
    (void) buf;
}

