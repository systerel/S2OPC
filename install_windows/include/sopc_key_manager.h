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

/** \file sopc_key_manager.h
 *
 * \brief The KeyManager provides an API for Asymmetric Key Management such as loading signed public keys (Certificate)
 * and the corresponding private key.
 *
 * KeyManager is different than PKIProvider, which only handles signed public key validation and storage.
 *
 * KeyManager API is context-less. The KeyManager is generic, and is not linked to the current security policy.
 */

#ifndef SOPC_KEY_MANAGER_H_
#define SOPC_KEY_MANAGER_H_

#include <stddef.h>

#include "sopc_buffer.h"
#include "sopc_crypto_decl.h"
#include "sopc_secret_buffer.h"

/**
 * \brief A serialized representation of an asymmetric key.
 *
 * This representation is safe to share across threads.
 */
typedef SOPC_SecretBuffer SOPC_SerializedAsymmetricKey;

/**
 * \brief A serialized representation of a certificate.
 *
 * This representation is safe to share across threads.
 */
typedef SOPC_Buffer SOPC_SerializedCertificate;

/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates an asymmetric key (usually a private key) from in-memory buffer \p buffer.
 *
 *                  \p buffer is \p lenBuf long, and describes the key in the DER of PEM format.
 *
 *                  Public keys are usually extracted from Certificate, see
 * KeyManager_AsymmetricKey_CreateFromCertificate() or KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param buffer     A valid pointer to the buffer containing the DER or PEM description.
 * \param lenBuf     The length in bytes of the DER/PEM description of the key.
 * \param is_public  Whether the buffer holds a public or a private key.
 * \param ppKey      A handle to the created key. This object must be freed with a call to
 * KeyManager_AsymmetricKey_Free().
 *
 * \note            Content of the key is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t* buffer,
                                                                 uint32_t lenBuf,
                                                                 bool is_public,
                                                                 SOPC_AsymmetricKey** ppKey);

/**
 * \brief           Creates an asymmetric key (usually a private key) from a file in the DER or PEM format.
 *
 *                  \p szPath is the path to the file containing the key. It should be zero-terminated.
 *                  The key may be described in the DER of PEM format.
 *
 *                  Public keys are usually extracted from Certificate, see
 * KeyManager_AsymmetricKey_CreateFromCertificate() or KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param szPath    The path to the DER/PEM file.
 * \param ppKey     A handle to the created key. This object must be freed with a call to
 * KeyManager_AsymmetricKey_Free(). \param password  An optional password. The password must be a zero-terminated string
 *                  with at most \p lenPassword non null chars, and at least \p lenPassword + 1 allocated chars.
 * \param lenPassword  The length of the password.
 *
 * \note            Content of the key is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromFile(const char* szPath,
                                                               SOPC_AsymmetricKey** ppKey,
                                                               char* password,
                                                               uint32_t lenPassword);

/**
 * \brief           Returns the public key of the signed public key.
 *
 * \warning         The returned AsymmetricKey must not be used after the Certificate is freed
 *                  by KeyManager_Certificate_Free().
 *
 * \param pCert     A valid pointer to the signed public key.
 * \param pKey      A handle to the created key structure, the AsymmetricKey will then be rewritten to contain the
 * public key. This is not a deep copy, and the key is not valid anymore when the certificate is not valid. This object
 * must be freed with a call to KeyManager_AsymmetricKey_Free() which will only deallocate the
 * structure.
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_Certificate* pCert,
                                                                      SOPC_AsymmetricKey** pKey);

/**
 * \brief           Frees a previously created asymmetric key created with KeyManager_AsymmetricKey_CreateFromBuffer()
 * or KeyManager_AsymmetricKey_CreateFromFile().
 *
 * \note            Do not use this function on a key obtained from KeyManager_Certificate_GetPublicKey().
 *
 * \param pKey      A valid pointer to the key to free.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey);

/**
 * \brief           Encodes the \p pKey as a DER buffer, and writes the result in \p pDest.
 *
 *                  The encoding process is not predictable, and a buffer of sufficient length
 *                  must be provided. A rule of thumb is to provide a buffer which is at least
 *                  8 times longer than the key (8*CryptoProvider_AsymmetricGetLength_KeyBytes()).
 *
 *                  When SOPC_STATUS_NOK is returned, the function may be called again with a larger buffer.
 *
 * \param pKey         A valid pointer to the asymmetric key (public/private) to encode.
 * \param is_public    Whether the key is public or private.
 * \param pDest        A valid pointer to the buffer which will receive the DER encoded key.
 * \param lenDest      The length in bytes of the buffer \p pDest.
 * \param pLenWritten  A valid pointer to the number of bytes written to pDest.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToDER(const SOPC_AsymmetricKey* pKey,
                                                      bool is_public,
                                                      uint8_t* pDest,
                                                      uint32_t lenDest,
                                                      uint32_t* pLenWritten);

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates a new Certificate (signed public key) from a DER encoded buffer.
 *
 *                  \p bufferDER is \p lenDER long, and describes the certificate in the DER format.
 *
 * \param bufferDER A valid pointer to the buffer containing the DER description.
 * \param lenDER    The length in bytes of the DER description of the certificate.
 * \param ppCert    A valid handle to the newly created Certificate.
 *                  This object must be freed with a call to KeyManager_Certificate_Free().
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromDER(const uint8_t* bufferDER,
                                                            uint32_t lenDER,
                                                            SOPC_Certificate** ppCert);

/**
 * \brief           Creates a new Certificate (signed public key) from a file in the DER or PEM format.
 *
 *                  \p szPath is the path to the file containing the key. It should be zero-terminated.
 *                  The key may be described in the DER of PEM format.
 *
 * \param szPath    The path to the DER/PEM file.
 * \param ppCert    A valid handle to the newly created Certificate.
 *                  This object must be freed with a call to KeyManager_Certificate_Free().
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromFile(const char* szPath, SOPC_Certificate** ppCert);

/**
 * \brief           Frees a Certificate created with KeyManager_Certificate_CreateFromFile() or
 * KeyManager_Certificate_CreateFromDER()
 *
 * \warning         You must not free a Certificate for which a key is still being used. See
 * KeyManager_Certificate_GetPublicKey() and KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param pCert     The Certificate to free.
 */
void SOPC_KeyManager_Certificate_Free(SOPC_Certificate* pCert);

/**
 * \brief           Copies a DER description of \p pCert.
 *
 * \param pCert     A valid pointer to the Certificate.
 * \param ppDest    A valid pointer to the newly created buffer that stores the DER description of the signed public
 * key. The allocated buffer must be freed by the caller. \param pLenAllocated  A valid pointer to the length allocated
 * by this operation.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CopyDER(const SOPC_Certificate* pCert,
                                                      uint8_t** ppDest,
                                                      uint32_t* pLenAllocated);

/**
 * \brief           Computes and writes the thumbprint of \p pCert to \p pDest.
 *
 *                  The computation of the thumbprint and its size depends on the current security policy.
 *                  The thumbprint is usually a SHA digest of the DER representation of the certificate.
 *
 *                  The size of the thumbprint is provided by CryptoProvider_CertificateGetLength_Thumbprint().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pCert     A valid pointer to the signed public key to thumbprint.
 * \param pDest     A valid pointer to the buffer that will contain the thumbprint.
 * \param lenDest   The length in bytes of \p pDest.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_Certificate* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest);

/**
 * \brief           Verify the application URI embedded in a certificate.
 *
 * This function does a strict, case sensitive comparison of the URIs and does not respect the URI comparison rules from
 * RFC3986 (the URI scheme comparison for example is case sensitive).
 *
 * \warning         Some limitations apply, see \p SOPC_KeyManager_Certificate_GetMaybeApplicationUri.
 *
 * \param pCert     The certificate.
 * \param applicationUri  The value that should be stored in the URI subject altName of the certificate.
                          This should be a zero-terminated string.
 *
 * \return \c TRUE if the values match, \c FALSE else.
 */
bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_Certificate* pCert, const char* applicationUri);

/**
 * \brief           Copy the application URI embedded in a certificate.
 *
 * \warning         Some limitations apply when using the MbedTLS crypto backend: MbedTLS has no way to extract
 *                  anything else than the DNS altName from the certificate extensions
 *                  (see https://github.com/ARMmbed/mbedtls/pull/731). We have for now a poor man's ASN.1 "parser" that
 *                  tries to find it. It should not be considered as secure, as it can produce false positives (ie.
 *                  extract the application URI from a field that is not the right one).
 *
 * \param pCert     The certificate.
 * \param ppApplicationUri  A pointer to the newly allocated zero-terminated string containing the application URI.
 * \param pStringLength     Optional pointer to the string length (excluding the trailing \0).
 *
 * \return          SOPC_STATUS_OK when successfully copied.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_Certificate* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength);

/**
 * \brief Creates a serialized asymmetric key from a DER or PEM payload.
 *
 * \param data   the key data in DER or PEM format
 * \param len   length of the data
 * \param key  out parameter, the created serialized key
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(const uint8_t* data,
                                                                         uint32_t len,
                                                                         SOPC_SerializedAsymmetricKey** key);

/**
 * \brief Creates a serialized asymmetric key from a file in DER or PEM format.
 *
 * \param path  path to the file
 * \param key   out parameter, the created serialized key
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(const char* path,
                                                                         SOPC_SerializedAsymmetricKey** key);

/**
 * \brief Deserializes a serialized key.
 *
 * \param key        the serialized key
 * \param is_public  whether the serialized key is a public or a private key
 * \param res        out parameter, the decoded key as a newly allocated SOPC_AsymmetricKey
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(const SOPC_SerializedAsymmetricKey* key,
                                                                      bool is_public,
                                                                      SOPC_AsymmetricKey** res);

/**
 * \brief Releases all resources associated to a serialized asymmetric key.
 *
 * \param key  The serialized key
 */
void SOPC_KeyManager_SerializedAsymmetricKey_Delete(SOPC_SerializedAsymmetricKey* key);

/**
 * \brief Creates a serialized certificate from a DER payload.
 *
 * \param der   the certificate data in DER format
 * \param len   length of the DER data
 * \param cert  out parameter, the created serialized certificate
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromDER(const uint8_t* der,
                                                                      uint32_t len,
                                                                      SOPC_SerializedCertificate** cert);

/**
 * \brief Creates a serialized certificate from a file in DER format.
 *
 * \param path  path to the file
 * \param cert  out parameter, the created serialized certificate
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromFile(const char* path,
                                                                       SOPC_SerializedCertificate** cert);

/**
 * \brief Deserializes a serialized certificate.
 *
 * \param cert  the serialized certificate
 * \param res   out parameter, the decoded certificate as a newly allocated SOPC_Certificate
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_Deserialize(const SOPC_SerializedCertificate* cert,
                                                                    SOPC_Certificate** res);

/**
 * \brief Returns the data held in a serialized certificate
 *
 * \param cert  the serialized certificate
 *
 * \return The data held in the serialized certificate. The returned memory is owned
 *         by the serialized certificate, and should not be modified or freed.
 */
const SOPC_Buffer* SOPC_KeyManager_SerializedCertificate_Data(const SOPC_SerializedCertificate* cert);

/**
 * \brief Releases all resources associated to a serialized certificate.
 *
 * \param cert  The serialized certificate
 */
void SOPC_KeyManager_SerializedCertificate_Delete(SOPC_SerializedCertificate* cert);

#endif /* SOPC_KEY_MANAGER_H_ */
