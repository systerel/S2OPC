/*
 *  \brief SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 *
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
SecretBuffer *SecretBuffer_New(uint32_t len);
void SecretBuffer_DeleteClear(SecretBuffer *sec);
uint32_t SecretBuffer_GetLength(const SecretBuffer *sec);

ExposedBuffer * SecretBuffer_Expose(const SecretBuffer *sec);
void SecretBuffer_Unexpose(ExposedBuffer *buf);


#endif // INGOPCS_SECRET_BUFFER_H_
