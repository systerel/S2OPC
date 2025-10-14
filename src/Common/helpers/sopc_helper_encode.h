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

#ifndef SOPC_HELPER_ENCODE_H_
#define SOPC_HELPER_ENCODE_H_

#include <stddef.h>
#include "sopc_builtintypes.h"
#include "sopc_enums.h"

/**
 * \brief  This function decodes a null-terminated base64 C string.
 *
 * \param pInput     A valid pointer to the null-terminated base64 C string.
 *
 * \param[out] ppOut       A valid pointer poiting to NULL. It will be set to the buffer containing the newly decoded
 * data. The allocated buffer must be freed by the caller.
 *
 * \param pOutLen    A valid pointer. On success, it is set to the exact size
 *                    of the decoded binary data stored in \p ppOut.
 *
 * \return  SOPC_STATUS_OK when successful otherwise SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_HelperDecode_Base64(const char* pInput, unsigned char** ppOut, size_t* pOutLen);

/**
 * \brief  This function encodes a data into a null-terminated base64 C string.
 *
 * \param pInput     A valid pointer to the data.
 * \param inputLen     The size of data in \p pInput
 *
 * \param[out] ppOut       A valid pointer. It will be set to the buffer containing the newly encoded
 * null-terminated base64 C string. The allocated buffer must be freed by the caller.
 *
 * \param[out] pOutLen    A valid pointer. In case of success, the content is set to the size of
 *                        \p ppOut computed during the function execution (including NULL terminating char).
 *
 * \return  SOPC_STATUS_OK when successful otherwise SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_HelperEncode_Base64(const SOPC_Byte* pInput, size_t inputLen, char** ppOut, size_t* pOutLen);

/**
 * \brief  Decodes a hexadecimal ByteString.
 *
 * \param pInput     A valid pointer to the hexadecimal ByteString.
 *
 * \param[out] pOut       A valid pointer to the output (you should allocate strlen( \p pInput )/2 in \p pOut .
 *
 * \param outputLen    The size of the \p pInput divided by two.
 *
 * \return  SOPC_STATUS_OK when successful otherwise SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_HelperDecode_Hex(const char* pInput, unsigned char* pOut, size_t outputLen);

/**
 * \brief  Encodes a ByteString to a hexadecimal ByteString.
 *
 * \param pInput     A valid pointer to a ByteString.
 *
 * \param[out] pOut       A valid pointer to the output (you should allocate strlen( \p pInput )*2 in \p pOut .
 *
 * \param inputLen    The size of the \p pInput .
 *
 * \return  SOPC_STATUS_OK when successful otherwise SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_HelperEncode_Hex(const unsigned char* pInput, char* pOut, size_t inputLen);

#endif /* SOPC_HELPER_ENCODE_H_ */
