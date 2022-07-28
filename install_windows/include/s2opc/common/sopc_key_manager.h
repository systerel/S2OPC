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
 * \brief The SOPC_KeyManager provides an API for Asymmetric Key Management such as loading signed public keys
 * (Certificate) and the corresponding private key.
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
 *   \p buffer is \p lenBuf long, and describes the key in the DER of PEM format.
 *
 *   Public keys are usually extracted from Certificate, see
 *   SOPC_KeyManager_AsymmetricKey_CreateFromCertificate() or SOPC_KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param buffer    A valid pointer to the buffer containing the DER or PEM description.
 * \param lenBuf    The length in bytes of the DER/PEM description of the key.
 * \param is_public Whether the buffer holds a public or a private key.
 * \param ppKey     A handle to the created key.
 *                  This object must be freed with a call to SOPC_KeyManager_AsymmetricKey_Free().
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
 *   \p szPath is the path to the file containing the key. It should be zero-terminated.
 *   The key may be described in the DER of PEM format.
 *
 *   Public keys are usually extracted from Certificate, see SOPC_KeyManager_AsymmetricKey_CreateFromCertificate() or
 *   SOPC_KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param szPath    The path to the DER/PEM file.
 * \param ppKey     A handle to the created key. This object must be freed with a call to
 *                  SOPC_KeyManager_AsymmetricKey_Free().
 * \param password  An optional password. The password must be a zero-terminated string with at most \p lenPassword
 *                  non null chars, and at least \p lenPassword + 1 allocated chars.
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
 * \warning         The returned SOPC_AsymmetricKey must not be used after the Certificate is freed
 *                  by SOPC_KeyManager_Certificate_Free().
 *
 * \param pCert     A valid pointer to the signed public key.
 * \param pKey      A handle to the created key structure, the SOPC_AsymmetricKey will then be rewritten to contain the
 *                  public key. This is not a deep copy, and the key is not valid anymore when the certificate is not
 *                  valid. This object must be freed with a call to SOPC_KeyManager_AsymmetricKey_Free() which will
 *                  only deallocate the structure.
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_CertificateList* pCert,
                                                                      SOPC_AsymmetricKey** pKey);

/**
 * \brief           Frees a previously created asymmetric key created with
 *                  SOPC_KeyManager_AsymmetricKey_CreateFromBuffer() or
 *                  SOPC_KeyManager_AsymmetricKey_CreateFromFile().
 *
 * \note            Do not use this function on a key obtained from SOPC_KeyManager_Certificate_GetPublicKey().
 *
 * \param pKey      A valid pointer to the key to free.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey);

/**
 * \brief           Encodes the \p pKey as a DER buffer, and writes the result in \p pDest.
 *
 *   The encoding process is not predictable, and a buffer of sufficient length
 *   must be provided. A rule of thumb is to provide a buffer which is at least
 *   8 times longer than the key (8*SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes()).
 *
 *   When SOPC_STATUS_NOK is returned, the function may be called again with a larger buffer.
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

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates a new Certificate (signed public key) from a DER encoded buffer,
 *                  or add it to an existing certificate list.
 *
 *   \p bufferDER is \p lenDER long, and describes the certificate in the DER format.
 *
 * \param bufferDER A valid pointer to the buffer containing the DER description.
 * \param lenDER    The length in bytes of the DER description of the certificate.
 * \param ppCert    Creation: a valid handle which will point to the newly created Certificate.
 *                  Addition: a pointer to a pointer to a Certificate list to which add the certificate.
 *                  In either cases, this object must be freed with a call to SOPC_KeyManager_Certificate_Free().
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *                  However, in case of a failed addition, the whole certificate list is freed,
 *                  and \p ppCert set to NULL to avoid double frees.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                                 uint32_t lenDER,
                                                                 SOPC_CertificateList** ppCert);

/**
 * \brief           Creates a new Certificate (signed public key) from a file in the DER or PEM format,
 *                  or add it to an existing certificate list.
 *
 *   \p szPath is the path to the file containing the key. It should be zero-terminated.
 *   The key may be described in the DER of PEM format.
 *
 * \param szPath    The path to the DER/PEM file.
 * \param ppCert    Creation: a valid handle which will point to the newly created Certificate.
 *                  Addition: a pointer to a pointer to a Certificate list to which add the certificate.
 *                  In either cases, this object must be freed with a call to SOPC_KeyManager_Certificate_Free().
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *                  However, in case of a failed addition, the whole certificate list is freed,
 *                  and \p ppCert set to NULL to avoid double frees.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert);

/**
 * \brief           Frees a Certificate created with SOPC_KeyManager_Certificate_CreateOrAddFromFile() or
 *                  SOPC_KeyManager_Certificate_CreateOrAddFromDER()
 *
 * \warning         You must not free a Certificate for which a key is still being used.
 *                  See SOPC_KeyManager_Certificate_GetPublicKey() and
 *                  SOPC_KeyManager_AsymmetricKey_CreateFromCertificate().
 *
 * \param pCert     The Certificate to free.
 */
void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert);

/**
 * \brief           Encodes a \p pCert as a DER buffer and writes the result in \p ppDest.
 *
 * \param pCert     A valid pointer to the Certificate. There must be only one certificate in the list.
 * \param ppDest    A valid pointer to the newly created buffer that stores the DER description of the signed public
 *                  key. The allocated buffer must be freed by the caller.
 * \param pLenAllocated  A valid pointer to the length allocated by this operation.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or the certificate list contains more than one certificate,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER(const SOPC_CertificateList* pCert,
                                                    uint8_t** ppDest,
                                                    uint32_t* pLenAllocated);

/**
 * \brief           Computes and writes the thumbprint of \p pCert to \p pDest.
 *
 *   The computation of the thumbprint and its size depends on the current security policy.
 *   The thumbprint is usually a SHA digest of the DER representation of the certificate.
 *
 *   The size of the thumbprint is provided by SOPC_CryptoProvider_CertificateGetLength_Thumbprint().
 *
 * \param pProvider An initialized cryptographic context.
 * \param pCert     A valid pointer to the signed public key to thumbprint.
 * \param pDest     A valid pointer to the buffer that will contain the thumbprint.
 * \param lenDest   The length in bytes of \p pDest.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or the certificate list contains more than one certificate,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_CertificateList* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest);

/**
 * \brief           Verify the application URI embedded in a certificate.
 *
 *   This function does a strict, case sensitive comparison of the URIs and does not respect the URI comparison rules
 *   from RFC3986 (the URI scheme comparison for example is case sensitive).
 *
 * \warning         Some limitations apply, see \p SOPC_KeyManager_Certificate_GetMaybeApplicationUri.
 *
 * \param pCert     The certificate.
 * \param applicationUri  The value that should be stored in the URI subject altName of the certificate.
                          This should be a zero-terminated string.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or the certificate list contains more than one certificate,
 * \return \c TRUE if the values match, return \c FALSE else.
 */
bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_CertificateList* pCert, const char* applicationUri);

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
 * \warning         \p pCert must contain a single certificate.
 *
 * \return          SOPC_STATUS_OK when successfully copied.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength);

/**
 * \brief           Return the number of chained certificates in the certificate list \p pCert.
 *
 * \param pCert     The certificate or certificate list.
 * \param pLength   A valid pointer to the computed length of the list.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength);

/**
 * \brief           Removes (and frees) certificates from \p pCert that do not have exactly one revocation list
 *                  in \p pCRL.
 *
 *   This function does not set match to false if there are CRL that do not match any Certificate.
 *   This function skips certificates in /p pCert that are not authorities.
 *   Warning: this function fails with SOPC_STATUS_NOK if there are only CA without CRL.
 *
 * \param pCert     A valid pointer to the Certificate list.
 * \param pCRL      A valid pointer to the CRL list.
 * \param pbMatch   An optional pointer to the result of the test.
 *                  True value indicates that each certificate in \p pCert has exactly one associated CRL in \p pCRL,
 *                  and no certificate has been freed.
 *                  Otherwise false.
 *
 * \note            Content of \p pbMatch is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when all certificate are CA without CRL.
 */
SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(SOPC_CertificateList* pCert,
                                                                     const SOPC_CRLList* pCRL,
                                                                     bool* pbMatch);

/**
 * \brief           Finds whether a certificate is in the given certificate list or not.
 *
 * \param pList     An optional pointer to the Certificate list.
 * \param pCert     An optional pointer to a single Certificate to find in the list.
 * \param pbMatch   A valid pointer to the result of the find.
 *                  True indicates that the certificate was found in the list.
 *                  Otherwise false.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or \p pCert has more than one certificate.
 */
SOPC_ReturnStatus SOPC_KeyManager_CertificateList_FindCertInList(const SOPC_CertificateList* pList,
                                                                 const SOPC_CertificateList* pCert,
                                                                 bool* pbMatch);

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
 * \param res   out parameter, the decoded certificate as a newly allocated SOPC_CertificateList
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_Deserialize(const SOPC_SerializedCertificate* cert,
                                                                    SOPC_CertificateList** res);

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

/* ------------------------------------------------------------------------------------------------
 * Certificate Revocation List API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief           Creates a new Certificate Revocation List (CRL) from a DER encoded buffer,
 *                  or add it to an existing CRL list.
 *
 *   \p bufferDER is \p lenDER long, and describes one CRL in the DER format.
 *
 * \param bufferDER A valid pointer to the buffer containing the DER description.
 * \param lenDER    The length in bytes of the DER description of the certificate.
 * \param ppCRL     Creation: a valid handle which will point to the newly created CRL.
 *                  Addition: a pointer to a pointer to a CRL list to which add the CRL.
 *                  In either cases, this object must be freed with a call to SOPC_KeyManager_CRL_Free().
 *
 * \note            Content of the CRL is unspecified when return value is not SOPC_STATUS_OK.
 *                  However, in case of a failed addition, the whole CRL list is freed,
 *                  and \p ppCRL set to NULL to avoid double frees.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                         uint32_t lenDER,
                                                         SOPC_CRLList** ppCRL);

/**
 * \brief           Creates a new Certificate Revocation List (CRL) from a file in the DER or PEM format,
 *                  or add it to an existing CRL list.
 *
 *   \p szPath is the path to the file containing the key. It should be zero-terminated.
 *   The key may be described in the DER of PEM format.
 *
 * \param szPath    The path to the DER/PEM file.
 * \param ppCRL     Creation: a valid handle which will point to the newly created CRL.
 *                  Addition: a pointer to a pointer to a CRL list to which add the CRL.
 *                  In either cases, this object must be freed with a call to SOPC_KeyManager_CRL_Free().
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *                  However, in case of a failed addition, the whole CRL list is freed,
 *                  and \p ppCRL set to NULL to avoid double frees.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL);

/**
 * \brief           Frees a Certificate created with SOPC_KeyManager_CRL_CreateOrAddFromFile() or
 *                  SOPC_KeyManager_CRL_CreateOrAddFromDER()
 *
 * \param pCRL      The CRL to free.
 */
void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL);

#endif /* SOPC_KEY_MANAGER_H_ */
