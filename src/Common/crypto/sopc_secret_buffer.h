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

/**
 *  \file sopc_secret_buffer.h
 *  \brief SecretBuffer (mangled key) and ExposedBuffer (contiguous deciphered buffered) APIs.
 *
 *  Sensitive information should be stored as SecretBuffer (e.g. crypto keys, nonces, initialisation vectors).
 *  The current implementation of the SecretBuffer is a contiguous buffer, but that could be changed without
 *  impact on the API.
 */

#ifndef SOPC_SECRET_BUFFER_H_
#define SOPC_SECRET_BUFFER_H_

#include <stdint.h>

// Types
typedef struct SOPC_SecretBuffer SOPC_SecretBuffer;
typedef uint8_t SOPC_ExposedBuffer;

/* ------------------------------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates a new SecretBuffer from an ExposedBuffer.
 *
 *   Copies \p buf, so it can be de-allocated after that call.
 *   This function does not clear the exposed secrets from \p buf.
 *
 *   SecretBuffer shall be de-allocated with SOPC_SecretBuffer_DeleteClear().
 *
 * \param buf       The ExposedBuffer.
 * \param len       Number of bytes of the buffer.
 *
 * \return          The SecretBuffer when successful, otherwise a NULL.
 */
SOPC_SecretBuffer* SOPC_SecretBuffer_NewFromExposedBuffer(SOPC_ExposedBuffer* buf, uint32_t len);

/**
 * \brief Creates a new SecretBuffer from a file.
 *
 * \param path  The path to the file
 *
 * \return The created SecretBuffer if successful, or NULL in case of error.
 */
SOPC_SecretBuffer* SOPC_SecretBuffer_NewFromFile(const char* path);

/**
 * \brief           Creates a new empty SecretBuffer.
 *
 * \param len       Number of bytes of the buffer.
 *
 * \return          The SecretBuffer when successful, otherwise a NULL.
 */
SOPC_SecretBuffer* SOPC_SecretBuffer_New(uint32_t len);

/**
 * \brief           Clears the SecretBuffer from its secrets and frees it.
 *
 * \param sec       The SecretBuffer to free.
 */
void SOPC_SecretBuffer_DeleteClear(SOPC_SecretBuffer* sec);

/**
 * \brief           Length of the SecretBuffer.
 */
uint32_t SOPC_SecretBuffer_GetLength(const SOPC_SecretBuffer* sec);

/**
 * \brief           Creates a ExposedBuffer from a SecretBuffer.
 *
 *   Each call to _Expose shoud be followed by a call to SOPC_SecretBuffer_Unexpose().
 *
 * \warning         \p sec shall not be de-allocated before the call to SOPC_SecretBuffer_Unexpose().
 * \warning         The exposed buffer shall not be modified by the caller.
 *
 * \param sec       The SecretBuffer to expose.
 *
 * \return          The ExposedBuffer when successful, otherwise a NULL.
 */
const SOPC_ExposedBuffer* SOPC_SecretBuffer_Expose(const SOPC_SecretBuffer* sec);

/**
 * \brief           Unexposes the buffer.
 *
 * \param buf       The ExposedBuffer
 * \param sec       The SecretBuffer to store the data
 *
 */
void SOPC_SecretBuffer_Unexpose(const SOPC_ExposedBuffer* buf, const SOPC_SecretBuffer* sec);

/**
 * \brief           Creates a ExposedBuffer from a SecretBuffer for modification.
 *
 *   Each call to _Expose shoud be followed by a call to SOPC_SecretBuffer_Unexpose().
 *
 * \warning         \p sec shall not be de-allocated before the call to SOPC_SecretBuffer_Unexpose().
 *
 * \param sec       The SecretBuffer to expose.
 *
 * \return          The ExposedBuffer when successful, otherwise a NULL.
 */
SOPC_ExposedBuffer* SOPC_SecretBuffer_ExposeModify(SOPC_SecretBuffer* sec);

/**
 * \brief           Unexposes the buffer exposed for modification.
 *
 * \param buf       The ExposedBuffer
 * \param sec       The SecretBuffer to store the data
 *
 */
void SOPC_SecretBuffer_UnexposeModify(SOPC_ExposedBuffer* buf, SOPC_SecretBuffer* sec);

#endif // SOPC_SECRET_BUFFER_H_
