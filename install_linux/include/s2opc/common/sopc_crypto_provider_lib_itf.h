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

/** \file
 *
 * \brief Defines the cryptographic abstraction interface for the SOPC_CryptoProvider.
 *        A cryptographic implementation must define all the functions declared in this file.
 */

#ifndef SOPC_CRYPTO_PROVIDER_LIB_ITF_H_
#define SOPC_CRYPTO_PROVIDER_LIB_ITF_H_

#include "sopc_crypto_decl.h"
#include "sopc_enums.h"

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider Creation
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief       Initializes a SOPC_CryptoProvider context.
 *              Called by SOPC_CryptoProvider_Create() upon context creation.
 *
 * \param pCryptoProvider A Cryptographic context to be initialized. It should uninitialized by
 *                        SOPC_CryptoProvider_Deinit().
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 *
 * \return       SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when \p pCryptoProvider is NULL,
 *               SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Init(SOPC_CryptoProvider* pCryptoProvider);

/**
 * \brief       Deinitialize a SOPC_CryptoProvider context (this process is specific to the chosen cryptographic
 *              library). Called by SOPC_CryptoProvider_Free() upon context destruction.
 *
 * \param pCryptoProvider A Cryptographic context to be uninitialized.
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 *
 * \return      SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when \p pCryptoProvider is NULL.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Deinit(SOPC_CryptoProvider* pCryptoProvider);

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length & uris operations
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Writes the length in bits in \p pLenKeyBits of the asymmetric key \p pKey.
 *
 *   The main purpose of this function is to verify the length of the modulus of the
 *   asymmetric key \p pKey with respect to the security policy.
 *
 * \param pProvider    An initialized cryptographic context.
 * \param pKey         A valid pointer to an SOPC_AsymmetricKey.
 * \param pLenKeyBits  A valid pointer to the output length in bits. Its content is unspecified when
 *                     return value is not SOPC_STATUS_OK.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(const SOPC_CryptoProvider* pProvider,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint32_t* pLenKeyBits);

/**
 * \brief           Provides the maximum length in bytes of a message to be encrypted with a single asymmetric
 *                  encryption operation.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param pLenMsg   A valid pointer to the length in bytes of the maximum length in bytes of the plain text message
 *                  used by the encryption process.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(const SOPC_CryptoProvider* pProvider,
                                                                       const SOPC_AsymmetricKey* pKey,
                                                                       uint32_t* pLenMsg);

/**
 * \brief           Provides the length in bytes of a ciphered message to be decrypted with a single asymmetric
 *                  decryption operation.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param pLenMsg   A valid pointer to the length in bytes of the ciphered message used by the decryption process.
 *
 * \note            The implementation is specific to the chosen cryptographic library.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(const SOPC_CryptoProvider* pProvider,
                                                                        const SOPC_AsymmetricKey* pKey,
                                                                        uint32_t* pLenMsg);

#endif /* SOPC_CRYPTO_PROVIDER_LIB_ITF_H_ */
