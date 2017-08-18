/**
 *  \file secret_buffer.h
 *  \brief SecretBuffer (mangled key) and ExposedBuffer (contiguous deciphered buffered) APIs.
 *
 *  Sensitive information should be stored as SecretBuffer (e.g. crypto keys, nonces, initialisation vectors).
 *  The current implementation of the SecretBuffer is a contiguous buffer, but that could be changed without
 *  impact on the API.
 */
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


#ifndef SOPC_SECRET_BUFFER_H_
#define SOPC_SECRET_BUFFER_H_

#include <stdint.h>

// Types
typedef struct SecretBuffer SecretBuffer;
typedef uint8_t ExposedBuffer;


/* ------------------------------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates a new SecretBuffer from an ExposedBuffer.
 *
 *                  Copies \p buf, so it can be de-allocated after that call.
 *                  This function does not clear the exposed secrets from \p buf.
 *
 *                  SecretBuffer shall be de-allocated with SecretBuffer_DeleteClear().
 *
 * \param buf       The ExposedBuffer.
 * \param len       Number of bytes of the buffer.
 *
 * \return          The SecretBuffer when successful, otherwise a NULL.
 */
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len);

/**
 * \brief           Creates a new empty SecretBuffer.
 *
 * \param len       Number of bytes of the buffer.
 *
 * \return          The SecretBuffer when successful, otherwise a NULL.
 */
SecretBuffer *SecretBuffer_New(uint32_t len);

/**
 * \brief           Clears the SecretBuffer from its secrets and frees it.
 *
 * \param sec       The SecretBuffer to free.
 */
void SecretBuffer_DeleteClear(SecretBuffer *sec);

/**
 * \brief           Length of the SecretBuffer.
 */
uint32_t SecretBuffer_GetLength(const SecretBuffer *sec);


/**
 * \brief           Creates a ExposedBuffer from a SecretBuffer.
 *
 *                  Each call to _Expose shoud be followed by a call to SecretBuffer_Unexpose().
 *
 * \warning         \p sec shall not be de-allocated before the call to SecretBuffer_Unexpose().
 * \warning         The exposed buffer shall not be modified by the caller.
 *
 * \param sec       The SecretBuffer to expose.
 *
 * \return          The ExposedBuffer when successful, otherwise a NULL.
 */
ExposedBuffer * SecretBuffer_Expose(const SecretBuffer *sec);

/**
 * \brief           Unexposes the buffer.
 *
 * \param buf       The ExposedBuffer.
 */
void SecretBuffer_Unexpose(ExposedBuffer *buf);


#endif // SOPC_SECRET_BUFFER_H_
