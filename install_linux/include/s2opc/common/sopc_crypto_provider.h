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

/** \file sopc_crypto_provider.h
 *
 * \brief   Defines the cryptographic API. This API mainly relies on the SOPC_CryptoProvider, which is composed of
 *          lib-specific data alongside a read-only SOPC_CryptoProfile or SOPC_CryptoProfile_PubSub.
 */

#ifndef SOPC_CRYPTO_PROVIDER_H_
#define SOPC_CRYPTO_PROVIDER_H_

#include "sopc_crypto_decl.h"
#include "sopc_key_manager.h"
#include "sopc_key_sets.h"
#include "sopc_pki.h"
#include "sopc_secret_buffer.h"

#include "sopc_enums.h"

/**
 * \brief   The SOPC_CryptoProvider context.
 *
 * Contains two pointers to a const SOPC_CryptoProfile and a SOPC_CryptoProfile_PubSub,
 * which should not be modified and contains function pointers to the cryptographic functions associated to a chosen
 * SecurityPolicy. Also contains a SOPC_CryptolibContext, which are library-specific structures defined in
 * crypto_provider_lib.h/c
 */
struct SOPC_CryptoProvider
{
    /**
     * SOPC_CryptoProfile associated to the chosen Security Policy, to use with client-server security policies.
     * You should not attempt to modify the content of this pointer.
     * You should only access this pointer in the SOPC_CryptoProvider implementation using
     * SOPC_CryptoProvider_GetProfileServices().
     */
    const SOPC_CryptoProfile* const pProfile;
    /**
     * SOPC_CryptoProfile_PubSub associated to the chosen Security policy, to use with PubSub security policies.
     * You should not attempt to modify the content of this pointer.
     * You should only access this pointer in the SOPC_CryptoProvider implementation using
     * SOPC_CryptoProvider_GetProfilePubSub().
     */
    const SOPC_CryptoProfile_PubSub* const pProfilePubSub;
    /**
     * A lib-specific context. This should not be accessed directly as its content may change depending on the chosen
     * crypto-lib implementation.
     */
    SOPC_CryptolibContext* pCryptolibContext;
};

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief       Creates an initialized SOPC_CryptoProvider context for a client-server connection
 *              from a string containing the desired security policy URI.
 *
 *   The SOPC_CryptoProvider contains the SOPC_CryptoProfile corresponding to the security policy.
 *   It should never be modified.
 *
 * \param uri   The URI describing the security policy. Should not be NULL.
 *              Should describe only client-server security policies.
 *
 * \note        Use SOPC_CryptoProvider_CreatePubSub() to create a SOPC_CryptoProvider for PubSub exchanges.
 *
 * \return      An initialized SOPC_CryptoProvider* or NULL if the context could not be created.
 */
SOPC_CryptoProvider* SOPC_CryptoProvider_Create(const char* uri);

/**
 * \brief       Creates an initialized SOPC_CryptoProvider context for PubSub exchanges
 *              from a string containing the desired security policy URI.
 *
 *   The SOPC_CryptoProvider contains the SOPC_CryptoProfile_PubSub corresponding to the security policy.
 *   It should never be modified.
 *
 * \param uri   The URI describing the security policy. Should not be NULL.
 *              Should only describe PubSub security policies.
 *
 * \note        Use SOPC_CryptoProvider_Create() to create a SOPC_CryptoProvider for a client-server connection.
 *
 * \return      An initialized SOPC_CryptoProvider* or NULL if the context could not be created.
 */
SOPC_CryptoProvider* SOPC_CryptoProvider_CreatePubSub(const char* uri);

/**
 * \brief       Frees a SOPC_CryptoProvider created with SOPC_CryptoProvider_Create().
 *
 * \param pCryptoProvider  The SOPC_CryptoProvider to free.
 */
void SOPC_CryptoProvider_Free(SOPC_CryptoProvider* pCryptoProvider);

/**
 * \brief       Initializes a SOPC_CryptoProvider context.
 *              Called by SOPC_CryptoProvider_Create() upon context creation.
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Init(SOPC_CryptoProvider* pCryptoProvider);

/**
 * \brief       Deinitializes a SOPC_CryptoProvider context (this process is specific to the chosen cryptographic
 *              library). Called by SOPC_CryptoProvider_Free() upon context destruction.
 *
 * \note        The implementation is specific to the chosen cryptographic library.
 * \note        Internal API.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Deinit(SOPC_CryptoProvider* pCryptoProvider);

/**
 * \brief       Returns the non NULL client-server crypto profile but returns NULL if the PubSub profile is non NULL.
 *
 * \note        Internal API.
 */
const SOPC_CryptoProfile* SOPC_CryptoProvider_GetProfileServices(const SOPC_CryptoProvider* pProvider);

/**
 *\brief        Returns the non NULL PubSub crypto profile but returns NULL if the client-server profile is non NULL.
 *
 * \note        Internal API.
 */
const SOPC_CryptoProfile_PubSub* SOPC_CryptoProvider_GetProfilePubSub(const SOPC_CryptoProvider* pProvider);

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length & uris operations
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Writes the length in bytes in \p pLength of the key used for symmetric encryption/decryption.
 *
 *   The length of the key depends on the security policy associated with \p pProvider.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the key. Its content is unspecified
 *                  when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength);

/**
 * \brief           Writes the length in bytes in \p pLengthOut of an encrypted message of \p lengthIn bytes.
 *
 * \warning         Does not take padding into account.
 *
 * \param pProvider An initialized cryptographic context.
 * \param lengthIn  The length in bytes of the message to encrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the ciphered message. Its content is
 *                  unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when given pointers are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Encryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut);

/**
 * \brief           Writes the length in bytes in \p pLengthOut of a decrypted message of \p lengthIn bytes.
 *
 * \warning         Does not take padding into account.
 *
 * \param pProvider An initialized cryptographic context.
 * \param lengthIn  The length in bytes of the message to decrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the deciphered message. Its content is
 *                  unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when given pointers are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Decryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut);

/**
 * \brief           Writes the length in bytes in \p pLength of the key used for symmetric signature.
 *
 *   The length of the key depends on the security policy associated with \p pProvider.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the key. Its content is unspecified
 *                  when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SignKey(const SOPC_CryptoProvider* pProvider,
                                                                 uint32_t* pLength);

/**
 * \brief           Provides the length in bytes of the symmetric signature message.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the signature message.
 *                  Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Signature(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength);

/**
 * \brief           Provides the lengths in bytes of the blocks used in the symmetric encryption process.
 *
 *   These lengths are useful to predict the padding sizes required by the symmetric
 *   encryption process.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pCipherTextBlockSize  An optional pointer to the length in bytes of the block size used by
 *                              the encryption process.
 * \param pPlainTextBlockSize   An optional pointer to the length in bytes of the block size used by
 *                              the decryption process.
 *
 * \note            The values held by \p pCipherTextBlockSize and \p pPlainTextBlockSize are unspecified
 *                  when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Blocks(const SOPC_CryptoProvider* pProvider,
                                                                uint32_t* pCipherTextBlockSize,
                                                                uint32_t* pPlainTextBlockSize);

/**
 * \brief           Provides the length in bytes of the SecureChannel nonces used in the symmetric encryption process.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLenNonce  A valid pointer to the length in bytes of the nonce used by the encryption process.
 *                   Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                            uint32_t* pLenNonce);

/**
 * \brief           Provides the lengths in bytes of the secrets derived from the nonce exchange.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pSymmCryptoKeyLength   A valid pointer to the length in bytes of the symmetric key used for encryption.
 * \param pSymmSignKeyLength     A valid pointer to the length in bytes of the symmetric key used for signing.
 * \param pSymmInitVectorLength  A valid pointer to the length in bytes of the symmetric initialization vector.
 *
 * \note            The values held by \p pSymmCryptoKeyLength, \p pSymmSignKeyLength and \p pSymmInitVectorLength
 *                  are unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_DeriveGetLengths(const SOPC_CryptoProvider* pProvider,
                                                       uint32_t* pSymmCryptoKeyLength,
                                                       uint32_t* pSymmSignKeyLength,
                                                       uint32_t* pSymmInitVectorLength);

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
 * \brief           Writes the length in bytes in \p pLenKeyBytes of the asymmetric key \p pKey.
 *
 *   The main purpose of this function is to verify the length of the modulus of the
 *   asymmetric key \p pKey with respect to the security policy.
 *
 * \param pProvider     An initialized cryptographic context.
 * \param pKey          A valid pointer to an SOPC_AsymmetricKey.
 * \param pLenKeyBytes  A valid pointer to the output length in bytes. Its content is unspecified when
 *                      return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(const SOPC_CryptoProvider* pProvider,
                                                                   const SOPC_AsymmetricKey* pKey,
                                                                   uint32_t* pLenKeyBytes);

/**
 * \brief           Provides the length of the hash used for OAEP encryption/decryption.
 *
 * \note            Internal API.
 *
 * \note            Specific to client-server security policies.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(const SOPC_CryptoProvider* pProvider,
                                                                         uint32_t* length);

/**
 * \brief           Provides the length of the hash used for PSS signature/verification.
 *
 * \note            Internal API.
 *
 * \note            Specific to client-server security policies.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_PSSHashLength(const SOPC_CryptoProvider* pProvider,
                                                                        uint32_t* length);

/**
 * \brief           Provides the lengths in bytes of the messages used in asymmetric encryption process.
 *
 *   These lengths are useful to predict the padding sizes required by the asymmetric
 *   encryption process.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param pCipherTextBlockSize  An optional pointer to the maximum length in bytes of the plain text message used by
 *                              the encryption process.
 * \param pPlainTextBlockSize   An optional pointer to the length in bytes of the ciphered message used by
 *                              the decryption process.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Msgs(const SOPC_CryptoProvider* pProvider,
                                                               const SOPC_AsymmetricKey* pKey,
                                                               uint32_t* pCipherTextBlockSize,
                                                               uint32_t* pPlainTextBlockSize);

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

/** \brief          Calculates the size of the required output buffer to cipher lengthIn bytes through
 *                  asymmetric encryption.
 *
 *   Hence, the computation takes into account the padding, but it does not include any signature length.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param lengthIn  The length in bytes of the payload to encrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the corresponding encrypted payload.
 *                    Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Encryption(const SOPC_CryptoProvider* pProvider,
                                                                     const SOPC_AsymmetricKey* pKey,
                                                                     uint32_t lengthIn,
                                                                     uint32_t* pLengthOut);

/**
 * \brief           Calculates the size of the required output buffer to decipher lengthIn bytes through
 *                  asymmetric decryption.
 *
 *   Hence, the computation takes into account the padding, but it does not include any signature length.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param lengthIn  The length in bytes of the payload to decrypt.
 * \param pLengthOut  A valid pointer to the length in bytes of the corresponding decrypted payload.
 *                    Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Decryption(const SOPC_CryptoProvider* pProvider,
                                                                     const SOPC_AsymmetricKey* pKey,
                                                                     uint32_t lengthIn,
                                                                     uint32_t* pLengthOut);

/**
 * \brief           Calculates the size of the required output buffer to contain the asymmetric signature.
 *
 *   It is a single ciphered-message long.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey.
 * \param pLength   A valid pointer to the length in bytes of the signature.
 *                  Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Signature(const SOPC_CryptoProvider* pProvider,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint32_t* pLength);

/**
 * \brief           Returns the URI of the AsymetricSignatureAlgorithm.
 *
 * \param pProvider An initialized cryptographic context.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          A zero-terminated string to the URI or NULL.
 */
const char* SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(const SOPC_CryptoProvider* pProvider);

/**
 * \brief           Calculates the size of the signature of the certificates.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the signature.
 *                  Its content is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_CertificateGetLength_Thumbprint(const SOPC_CryptoProvider* pProvider,
                                                                      uint32_t* pLength);

/**
 * \brief           Writes the length in bytes in \p pLength of the key nonce used for PubSub encryption/decryption.
 *
 *   The length of the nonce depends on the security policy associated with \p pProvider.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the key. Its content is unspecified
 *                  when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, and SOPC_STATUS_NOK for an unsupported
 *                  security policy.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_PubSubGetLength_KeyNonce(const SOPC_CryptoProvider* pProvider, uint32_t* pLength);

/**
 * \brief           Writes the length in bytes in \p pLength of the random bytes used for PubSub encryption/decryption.
 *
 *   This length depends on the security policy associated with \p pProvider.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pLength   A valid pointer to the length in bytes of the key. Its content is unspecified
 *                  when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized, and SOPC_STATUS_NOK for an unsupported
 *                  security policy.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_PubSubGetLength_MessageRandom(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t* pLength);

/* ------------------------------------------------------------------------------------------------
 * Symmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Encrypts a padded payload \p pInput of \p lenPlainText bytes.
 *
 *   Writes the ciphered payload in \p pOutput of \p lenOutput bytes.
 *   Does not apply a padding scheme, which must be done before calling this function.
 *   To calculate the padded size, use SOPC_CryptoProvider_SymmetricGetLength_Blocks().
 *
 *   The key and initialization vectors are usually derived from shared secrets
 *   with SOPC_CryptoProvider_DeriveKeySets().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to cipher. The payload must be padded.
 * \param lenPlainText  Length in bytes of the payload to cipher.
 * \param pKey      A valid pointer to a SecretBuffer containing the symmetric encryption key.
 * \param pIV       A valid pointer to a SecretBuffer containing the initialization vector.
 * \param pOutput   A valid pointer to the buffer which will contain the ciphered payload.
 * \param lenOutput The exact length of the ciphered payload. SOPC_CryptoProvider_SymmetricGetLength_Encryption()
 *                  provides the expected size of this buffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricEncrypt(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenPlainText,
                                                       SOPC_SecretBuffer* pKey,
                                                       SOPC_SecretBuffer* pIV,
                                                       uint8_t* pOutput,
                                                       uint32_t lenOutput);

/**
 * \brief           Decrypts a payload \p pInput of \p lenPlainText bytes into a padded deciphered payload \p pOutput.
 *
 *   Writes the deciphered payload in \p pOutput of \p lenOutput bytes.
 *   Does not use a padding scheme, which must be done after calling this function
 *   to obtain the initial message.
 *   To calculate the padded size, use SOPC_CryptoProvider_SymmetricGetLength_Blocks().
 *
 *   The encryption key and initialization vectors are usually derived from shared secrets
 *   with SOPC_CryptoProvider_DeriveKeySets().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to decipher.
 * \param lenCipherText  Length in bytes of the payload to decipher. The payload size must be a multiple of the
 *                       decipher block size, see SOPC_CryptoProvider_SymmetricGetLength_Blocks().
 * \param pKey      A valid pointer to a SecretBuffer containing the symmetric encryption key.
 * \param pIV       A valid pointer to a SecretBuffer containing the initialization vector.
 * \param pOutput   A valid pointer to the buffer which will contain the deciphered payload.
 * \param lenOutput The exact length of the deciphered payload. SOPC_CryptoProvider_SymmetricGetLength_Decryption()
 *                  provides the expected size of this buffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricDecrypt(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenCipherText,
                                                       SOPC_SecretBuffer* pKey,
                                                       SOPC_SecretBuffer* pIV,
                                                       uint8_t* pOutput,
                                                       uint32_t lenOutput);

/**
 * \brief           Encrypts or Decrypts a payload \p pInput of \p lenInput bytes.
 *
 *   Writes the ciphered payload in \p pOutput of \p lenOutput bytes.
 *   The length of the output must be the same as the input.
 *
 *   This mode of operation does not require block alignment or padding.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to cipher/decipher.
 * \param lenInput  Length in bytes of the payload to cipher/decipher.
 * \param pKey      A valid pointer to a SecretBuffer containing the symmetric encryption key.
 * \param pKeyNonce A valid pointer to a SecretBuffer containing the key nonce associated to the key.
 * \param pRandom   A valid pointer to an ExposedBuffer containing the random nonce of the message.
 * \param lenRandom Length in bytes of the message random buffer.
 * \param uSequenceNumber  The sequence number of the message. It forms the block counter alongside the
                           key nonce and random.
 * \param pOutput   A valid pointer to the buffer which will contain the transformed payload.
 * \param lenOutput The output length must be the same as the input length.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            See SOPC_CryptoProvider_SymmetricGetLength_CryptoKey() for the key length,
 *                  SOPC_CryptoProvider_PubSubGetLength_KeyNonce() for the nonce length,
 *                  and SOPC_CryptoProvider_PubSubGetLength_MessageRandom() for \p lenRandom.
 *
 * \note            Specific to PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or \p pProvider is initialized for a client-server
 *                  security policy or sizes are incorrect, and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_PubSubCrypt(const SOPC_CryptoProvider* pProvider,
                                                  const uint8_t* pInput,
                                                  uint32_t lenInput,
                                                  SOPC_SecretBuffer* pKey,
                                                  SOPC_SecretBuffer* pKeyNonce,
                                                  const SOPC_ExposedBuffer* pRandom,
                                                  uint32_t lenRandom,
                                                  uint32_t uSequenceNumber,
                                                  uint8_t* pOutput,
                                                  uint32_t lenOutput);

/**
 * \brief           Signs a payload \p pInput of \p lenInput bytes, writes the signature in \p pOutput of \p lenOutput
 *                  bytes.
 *
 *   The signature is as long as the underlying hash digest, which size is computed with
 *   SOPC_CryptoProvider_SymmetricGetLength_Signature().
 *   Usually, the unpadded plain text message is signed.
 *
 *   The signing key is usually derived from shared secrets with SOPC_CryptoProvider_DeriveKeySets().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to sign.
 * \param lenInput  Length in bytes of the payload to sign.
 * \param pKey      A valid pointer to a SecretBuffer containing the symmetric signing key.
 * \param pOutput   A valid pointer to the buffer which will contain the signature.
 * \param lenOutput The exact length of the signature buffer. SOPC_CryptoProvider_SymmetricGetLength_Signature()
 *                  provides the expected size of this buffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricSign(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenInput,
                                                    SOPC_SecretBuffer* pKey,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput);

/**
 * \brief           Verifies the signature \p pSignature of the payload \p pInput of \p lenInput bytes.
 *
 *   The signature is as long as the underlying hash digest, which size is computed with
 *   SOPC_CryptoProvider_SymmetricGetLength_Signature().
 *   Usually, the unpadded plain text message is signed.
 *   The signature verification process computes the signature from \p pInput and
 *   compares it with the content of \p pSignature.
 *
 *   The signing key is usually derived from shared secrets with SOPC_CryptoProvider_DeriveKeySets().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to sign.
 * \param lenInput  Length in bytes of the payload to sign.
 * \param pKey      A valid pointer to a SecretBuffer containing the symmetric signing key.
 * \param pSignature  A valid pointer to the signature.
 * \param lenOutput The exact length of the signature buffer. SOPC_CryptoProvider_SymmetricGetLength_Signature()
 *                  provides the expected size of this buffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricVerify(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      SOPC_SecretBuffer* pKey,
                                                      const uint8_t* pSignature,
                                                      uint32_t lenOutput);

/* ------------------------------------------------------------------------------------------------
 * Random and pseudo-random functionalities
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Generates truly random data of arbitrary length.
 *
 *   Uses the entropy generator provided by the underlying cryptographic library.
 *   The new ExposedBuffer is to be freed by the caller.
 *
 * \note            Prefer the functions SOPC_CryptoProvider_GenerateSecureChannelNonce()
 *                  and SOPC_CryptoProvider_GenerateRandomID().
 *
 * \param pProvider An initialized cryptographic context.
 * \param nBytes    Number of bytes to generate (and length of the created \p ppBuffer).
 * \param ppBuffer  A valid handle to the newly created ExposedBuffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            For both client-server and PubSub security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error (e.g. no entropy source).
 */
SOPC_ReturnStatus SOPC_CryptoProvider_GenerateRandomBytes(const SOPC_CryptoProvider* pProvider,
                                                          uint32_t nBytes,
                                                          SOPC_ExposedBuffer** ppBuffer);

/**
 * \brief           Generates a single truly random nonce for the SecureChannel creation.
 *
 *   The length of the nonce is defined by the current security policy (see *TBD*).
 *   Uses the entropy generator provided by the underlying cryptographic library.
 *   The new SecretBuffer is to be freed by the caller.
 *
 * \param pProvider An initialized cryptographic context.
 * \param ppNonce   A valid handle to the newly created SecretBuffer.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error (e.g. no entropy source).
 */
SOPC_ReturnStatus SOPC_CryptoProvider_GenerateSecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                 SOPC_SecretBuffer** ppNonce);

/**
 * \brief           Generates 4 bytes of truly random data.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pID       A valid pointer which will contain the random data.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error (e.g. no entropy source).
 */
SOPC_ReturnStatus SOPC_CryptoProvider_GenerateRandomID(const SOPC_CryptoProvider* pProvider, uint32_t* pID);

/**
 * \brief           Derives pseudo-random data from the randomly generated and shared secrets.
 *
 * \note            Internal API, use SOPC_CryptoProvider_DeriveKeySetsClient() or
 *                  SOPC_CryptoProvider_DeriveKeySetsServer() instead.
 *
 * \note            Specific to client-server security policies.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_DerivePseudoRandomData(const SOPC_CryptoProvider* pProvider,
                                                             const SOPC_ExposedBuffer* pSecret,
                                                             uint32_t lenSecret,
                                                             const SOPC_ExposedBuffer* pSeed,
                                                             uint32_t lenSeed,
                                                             SOPC_ExposedBuffer* pOutput,
                                                             uint32_t lenOutput);

/**
 * \brief           Derive pseudo-random key sets from the randomly generated and shared secrets.
 *
 * \sa              SOPC_CryptoProvider_SymmetricGenerateKey(), SOPC_CryptoProvider_DeriveKeySetsClient(),
 *                  and SOPC_CryptoProvider_DeriveKeySetsServer().
 *
 * \param pProvider         An initialized cryptographic context.
 * \param pClientNonce      A valid pointer to the client nonce buffer, the client part of the secret.
 * \param lenClientNonce    Length in bytes of the buffer of the client nonce. Its size should be *TBD*.
 * \param pServerNonce      A valid pointer to the server nonce buffer, the server part of the secret.
 * \param lenServerNonce    Length in bytes of the buffer of the server nonce. Its size should be *TBD*.
 * \param pClientKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          client side derived data.
 * \param pServerKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          server side derived data.
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySets(const SOPC_CryptoProvider* pProvider,
                                                    const SOPC_ExposedBuffer* pClientNonce,
                                                    uint32_t lenClientNonce,
                                                    const SOPC_ExposedBuffer* pServerNonce,
                                                    uint32_t lenServerNonce,
                                                    SOPC_SC_SecurityKeySet* pClientKeySet,
                                                    SOPC_SC_SecurityKeySet* pServerKeySet);

/**
 * \brief           Derive pseudo-random key sets from the randomly generated and shared secrets.
 *
 *   This function is similar to SOPC_CryptoProvider_DeriveKeySets but uses the client nonce as a SecretBuffer.
 *
 * \param pProvider         An initialized cryptographic context.
 * \param pClientNonce      A valid pointer to the client nonce as a SecretBuffer.
 * \param pServerNonce      A valid pointer to the server nonce buffer, the server part of the secret.
 * \param lenServerNonce    Length in bytes of the buffer of the server nonce. Its size should be *TBD*.
 * \param pClientKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          client side derived data.
 * \param pServerKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          server side derived data.
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySetsClient(const SOPC_CryptoProvider* pProvider, // DeriveKeySets
                                                          SOPC_SecretBuffer* pClientNonce,
                                                          const SOPC_ExposedBuffer* pServerNonce,
                                                          uint32_t lenServerNonce,
                                                          SOPC_SC_SecurityKeySet* pClientKeySet,
                                                          SOPC_SC_SecurityKeySet* pServerKeySet);

/**
 * \brief           Derive pseudo-random key sets from the randomly generated and shared secrets.
 *
 *   This function is similar to SOPC_CryptoProvider_DeriveKeySets but uses the server nonce as a SecretBuffer.
 *
 * \param pProvider         An initialized cryptographic context.
 * \param pClientNonce      A valid pointer to the client nonce buffer, the client part of the secret.
 * \param lenClientNonce    Length in bytes of the buffer of the client nonce. Its size should be *TBD*.
 * \param pServerNonce      A valid pointer to the server nonce as a SecretBuffer.
 * \param pClientKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          client side derived data.
 * \param pServerKeySet     A valid pointer to a pre-allocated SC_SecurityKeySet which will contain the
 *                          server side derived data.
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySetsServer(const SOPC_CryptoProvider* pProvider,
                                                          const SOPC_ExposedBuffer* pClientNonce,
                                                          uint32_t lenClientNonce,
                                                          SOPC_SecretBuffer* pServerNonce,
                                                          SOPC_SC_SecurityKeySet* pClientKeySet,
                                                          SOPC_SC_SecurityKeySet* pServerKeySet);

/* ------------------------------------------------------------------------------------------------
 * Asymmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Encrypts a payload \p pInput of \p lenInput bytes.
 *
 *   Writes the ciphered payload in \p pOutput of \p lenOutput bytes.
 *   The message may be padded. Depending on the chosen security policy, optimal padding
 *   is performed if \p lenPlainText is less than the maximum message size (computed with
 *   SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText()).
 *   If the payload is larger than the maximum message size for a single encryption pass,
 *   it is split in several smaller messages of at most that maximum length.
 *
 *   The key is usually taken from a signed public key (Certificate,
 *   SOPC_KeyManager_Certificate_GetPublicKey()) and is the public key of the receiver.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to cipher. The payload may be padded by the function, if necessary.
 * \param lenInput  Length in bytes of the payload to cipher.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey containing the asymmetric encryption key (public key).
 * \param pOutput   A valid pointer to the buffer which will contain the ciphered payload.
 * \param lenOutput The exact length of the ciphered payload. SOPC_CryptoProvider_AsymmetricGetLength_Encryption()
 *                  provides the expected size of this buffer.
 * \param errorReason  Pointer to a C string used to point on error reason string when returned status != SOPC_STATUS_OK
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricEncrypt(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_AsymmetricKey* pKey,
                                                        uint8_t* pOutput,
                                                        uint32_t lenOutput,
                                                        const char** errorReason);

/**
 * \brief           Decrypts a payload \p pInput of \p lenInput bytes.
 *
 *   Writes the deciphered payload in \p pOutput of \p lenOutput bytes.
 *   Depending on the chosen security policy, when the message was padded with
 *   SOPC_CryptoProvider_AsymmetricEncrypt(), the output is unpadded by this function and the initial
 *   payload is written to \p pOutput. If the payload is larger than the maximum message size for a
 *   single decryption pass, it is split in several smaller messages of at most that maximum length
 *   (SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText()).
 *
 *   The key is usually taken from a private key (Certificate,
 *   SOPC_KeyManager_AsymmetricKey_CreateFromFile()) and is the private key of the sender.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to cipher. The payload may be padded by the function, if necessary.
 * \param lenInput  Length in bytes of the payload to cipher.
 * \param pKey      A valid pointer to an SOPC_AsymmetricKey containing the asymmetric decryption key (private key).
 * \param pOutput   A valid pointer to the buffer which will contain the deciphered payload.
 * \param lenOutput The exact length of the deciphered payload. SOPC_CryptoProvider_AsymmetricGetLength_Decryption()
 *                  provides the expected size of this buffer.
 * \param pLenWritten  An optional pointer to the length in bytes that are written to the \p pOutput buffer.
 *                     Useful to determine the actual size of the plain text.
 * \param errorReason  Pointer to a C string used to point on error reason string when returned status != SOPC_STATUS_OK
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricDecrypt(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_AsymmetricKey* pKey,
                                                        uint8_t* pOutput,
                                                        uint32_t lenOutput,
                                                        uint32_t* pLenWritten,
                                                        const char** errorReason);

/**
 * \brief           Signs a payload \p pInput of \p lenInput bytes.
 *
 *   Writes the signature to \p pSignature, which is exactly \p lenSignature bytes long.
 *   The signature is as long as a single ciphered message, which size is computed with
 *   SOPC_CryptoProvider_AsymmetricGetLength_Signature().
 *   Usually, the unpadded plain text message is signed.
 *   The asymmetric signature process first hashes the \p pInput.
 *
 *   The key is usually taken from a private key (SOPC_KeyManager_AsymmetricKey_CreateFromFile())
 *   and is the private key of the sender, which authenticates the sender as the signer.
 *
 *   The signature is already encrypted and does not require to be ciphered again before
 *   being sent to the receiver.
 *
 * \note            The signature process may use the entropy source of the SOPC_CryptoProvider
 *                  (depending on the current security policy).
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the payload to sign.
 * \param lenInput  Length in bytes of the payload to sign.
 * \param pKeyPrivateLocal  A valid pointer to an SOPC_AsymmetricKey containing the asymmetric signing key
                            (private key of the sender).
 * \param pSignature  A valid pointer to the buffer which will contain the signature.
 * \param lenSignature  The exact length of the signature payload.
                        SOPC_CryptoProvider_AsymmetricGetLength_Signature() provides the expected size of this buffer.
 * \param errorReason  Pointer to a C string used to point on error reason string when returned status != SOPC_STATUS_OK
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error (e.g. no entropy source).
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricSign(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_AsymmetricKey* pKeyPrivateLocal,
                                                     uint8_t* pSignature,
                                                     uint32_t lenSignature,
                                                     const char** errorReason);

/**
 * \brief           Verifies the signature \p pSignature of a payload \p pInput of \p lenInput bytes.
 *
 *   The signature \p pSignature is exactly \p lenSignature bytes long.
 *   The signature is as long as a single ciphered message, which size is computed with
 *   SOPC_CryptoProvider_AsymmetricGetLength_Signature().
 *   The asymmetric verify process first deciphers the signature which should provide
 *   the hash of \p pInput.
 *   Usually, the unpadded plain text message is signed.
 *
 *   The key is usually taken from a public key (Certificate,
 *   SOPC_KeyManager_Certificate_GetPublicKey()) and is the public key of the sender,
 *   which authenticates the sender as the signer.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pInput    A valid pointer to the signed payload to verify.
 * \param lenInput  Length in bytes of the signed payload to verify.
 * \param pKeyRemotePublic  A valid pointer to an SOPC_AsymmetricKey containing the asymmetric verification key
                            (public key of the sender).
 * \param pSignature  A valid pointer to the buffer which will contain the signature.
 * \param lenSignature  The exact length of the signature payload.
                        SOPC_CryptoProvider_AsymmetricGetLength_Signature() provides the expected size of this buffer.
 * \param errorReason  Pointer to a C string used to point on error reason string when returned status != SOPC_STATUS_OK
 *
 * \note            Contents of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricVerify(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKeyRemotePublic,
                                                       const uint8_t* pSignature,
                                                       uint32_t lenSignature,
                                                       const char** errorReason);

/* ------------------------------------------------------------------------------------------------
 * Certificate validation
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Validates the given Certificate \p pCert.
 *
 *   This function first verifies that the signed public key respects the current
 *   security policy (asymmetric key type and length, signature hash type, ...),
 *   and then let the PKIProvider handle the signature validation.
 *   The verification of the signature chain up to the certificate authority is
 *   not endorsed by the SOPC_CryptoProvider, but by the PKIProvider, which must be
 *   created and configured outside the stack.
 *
 * \param pProvider An initialized cryptographic context.
 * \param pPKI      An initialized public key infrastructure (PKIProvider).
 * \param pCert     A valid pointer to the Certificate to validate.
 * \param error     Ouput error code set when returned status is not SOPC_STATUS_OK (see sopc_pki.h for values)
 *
 * \note            Contents of the outputs is unspecified when return value is SOPC_STATUS_NOK.
 *
 * \note            Specific to client-server security policies.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or
 *                  \p pProvider not correctly initialized or sizes are incorrect,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Certificate_Validate(const SOPC_CryptoProvider* pProvider,
                                                           const SOPC_PKIProvider* pPKI,
                                                           const SOPC_CertificateList* pCert,
                                                           uint32_t* error);

#endif /* SOPC_CRYPTO_PROVIDER_H_ */
