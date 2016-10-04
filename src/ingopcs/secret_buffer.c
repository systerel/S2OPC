/*
 * SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
 */


#include "ua_builtintypes.h" // UA_NULL
#include "secret_buffer.h"


/* Creates a new SecretBuffer from an exposed buffer and a length. Copies the buffer, so ExposedBuffer
 *  can be cleared and de-allocated.
 * You must call SecretBuffer_DeleteClear() for each call to _New
 */
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len)
{
    SecretBuffer *sec = UA_NULL;

    if(UA_NULL != buf && 0 != len)
    {
        sec = malloc(sizeof(SecretBuffer));
        if(UA_NULL != sec)
        {
            sec->len = len;
            sec->buf = malloc(len);
            if(UA_NULL != sec->buf)
                memcpy(sec->buf, buf, len);
            else
            {
                free(sec->buf);
                sec = UA_NULL;
            }
        }
    }

    return sec;
}
void SecretBuffer_DeleteClear(SecretBuffer *sec)
{
    if(UA_NULL != sec)
    {
        if(sec->buf)
        {
            memset(sec->buf, 0, sec->len);
            free(sec->buf);
        }
        free(sec);
    }
}

uint32_t SecretBuffer_GetLength(SecretBuffer *sec)
{
    if(UA_NULL == sec)
        return 0;
    return sec->len;
}

/* Exposes the secret buffer to a contiguous buffer.
 * Each call to Expose should be followed by a call to Unexpose.
 * SecretBuffer should not be destroyed before the call to Unexpose.
 */
ExposedBuffer *SecretBuffer_Expose(SecretBuffer *sec)
{
    if(UA_NULL != sec)
        return sec->buf;
    return UA_NULL;
}

void SecretBuffer_Unexpose(ExposedBuffer *buf)
{
    (void) buf;
}

