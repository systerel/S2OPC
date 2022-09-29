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

#ifndef SOPC_HELPER_DECODE_H_
#define SOPC_HELPER_DECODE_H_

#include <stddef.h>
#include "sopc_enums.h"

/**
 * \brief  Get the number of padding characters for base64 ('=')
 *
 * \param input  A valid pointer to the input.
 *
 * \return  The number of padding characters.
 */
int SOPC_HelperDecode_Base64_GetPaddingLength(const char* input);

/**
 * \brief  This function decodes a base64 ByteString. Base64 ByteString shall be null terminated.
 *         Otherwise, the result is undefined.
 *
 * \param input     A valid pointer to the input.
 *
 * \param out       A valid pointer to the output (must be allocated by the caller thank
 *                  SOPC_HelperDecode_Base64_GetPaddingLength)
 *
 * \param outLen    The size of the \p out comptuted during the function execution.
 *
 * \return  true when successful otherwise false.
 */
SOPC_ReturnStatus SOPC_HelperDecode_Base64(const char* input, unsigned char* out, size_t* outLen);

/**
 * \brief  Unhexlify a ByteString.
 *
 * \param input     A valid pointer to the input.
 *
 * \param out       A valid pointer to the output (you should allocate strlen( \p input )/2 in \p out . \p inputLen is
 *                  strlen( \p input ))
 *
 * \param inputLen    The size of the \p input .
 *
 * \return  Returns n the number of translated couples (< 0 for errors)
 */
SOPC_ReturnStatus SOPC_HelperDecode_Hex(const char* input, unsigned char* out, size_t inputLen);

#endif /* SOPC_HELPER_DECODE_H_ */
