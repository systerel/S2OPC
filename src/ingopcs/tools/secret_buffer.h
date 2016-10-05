/*
 * SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_SECRET_BUFFER_H_
#define INGOPCS_SECRET_BUFFER_H_

#include <stdint.h>

// Types
typedef struct SecretBuffer
{
    uint32_t len; // Mandatory
    uint8_t *buf;
} SecretBuffer;
typedef uint8_t ExposedBuffer;


// API
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len);
void SecretBuffer_DeleteClear(SecretBuffer *sec);
uint32_t SecretBuffer_GetLength(const SecretBuffer *sec);

ExposedBuffer * SecretBuffer_Expose(const SecretBuffer *sec);
void SecretBuffer_Unexpose(ExposedBuffer *buf);


#endif // INGOPCS_SECRET_BUFFER_H_
