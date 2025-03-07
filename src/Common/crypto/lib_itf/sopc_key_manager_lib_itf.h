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
 * \brief Defines the cryptographic abstraction interface for Asymmetric Key Management such as loading signed public
 *        keys (Certificate) and the corresponding private key.
 *        A cryptographic implementation must define all the function declared in this file.
 *        KeyManager is different than PKIProvider, which only handles signed public key validation and storage.
 *        KeyManager API is context-less. The KeyManager is generic, and is not linked to the current security policy.
 */

#ifndef SOPC_KEY_MANAGER_LIB_ITF_H_
#define SOPC_KEY_MANAGER_LIB_ITF_H_

#include <stddef.h>

#include "sopc_crypto_decl.h"
#include "sopc_enums.h"

#include "sopc_crypto_struct_lib_itf.h"

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
 *   ::SOPC_KeyManager_AsymmetricKey_CreateFromCertificate or ::SOPC_KeyManager_AsymmetricKey_CreateFromCertificate .
 *
 * \param buffer    A valid pointer to the buffer containing the DER or PEM description.
 * \param lenBuf    The length in bytes of the DER/PEM description of the key.
 * \param is_public Whether the buffer holds a public or a private key.
 * \param[out] ppKey A handle to the created key.
 *                   This object must be freed with a call to ::SOPC_KeyManager_AsymmetricKey_Free .
 *
 * \note            Content of the key is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  SOPC_STATUS_OUT_OF_MEMORY in case of memory allocation failure,
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
 *   Public keys are usually extracted from Certificate, see ::SOPC_KeyManager_AsymmetricKey_CreateFromCertificate or
 *   ::SOPC_KeyManager_AsymmetricKey_CreateFromCertificate .
 *
 * \param szPath    The path to the DER/PEM file.
 * \param[out] ppKey A handle to the created key. This object must be freed with a call to
 *                   ::SOPC_KeyManager_AsymmetricKey_Free .
 * \param password  An optional password. The password must be a zero-terminated string with at most \p lenPassword
 *                  non null chars, and at least \p lenPassword + 1 allocated chars.
 * \param lenPassword  The length of the password.
 *
 * \note            Content of the key is unspecified when return value is not SOPC_STATUS_OK.
 *                  Supported encryption algorithm: AES-128-CBC, AES-192-CBC and AES-256-CBC
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromFile(const char* szPath,
                                                               SOPC_AsymmetricKey** ppKey,
                                                               char* password,
                                                               uint32_t lenPassword);

/**
 * \brief  Generate an RSA asymmetric key.
 *
 * \param RSAKeySize The RSA key length to generate.
 * \param[out] ppKey A handle to the generated key. This object must be freed with a call to
 *                   ::SOPC_KeyManager_AsymmetricKey_Free
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_GenRSA(uint32_t RSAKeySize, SOPC_AsymmetricKey** ppKey);

/**
 * \brief           Returns the public key of the signed public key.
 *
 * \warning         The returned SOPC_AsymmetricKey must not be used after the Certificate is freed
 *                  by ::SOPC_KeyManager_Certificate_Free .
 *
 * \param pCert     A valid pointer to the signed public key.
 * \param[out] pKey A handle to the created key structure, the SOPC_AsymmetricKey will then be rewritten to contain the
 *                  public key. This is not a deep copy, and the key is not valid anymore when the certificate is not
 *                  valid. This object must be freed with a call to ::SOPC_KeyManager_AsymmetricKey_Free which will
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
 *                  ::SOPC_KeyManager_AsymmetricKey_CreateFromBuffer or
 *                  ::SOPC_KeyManager_AsymmetricKey_CreateFromFile .
 *
 * \param pKey      A valid pointer to the key to free.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey);

/**
 * \brief           Encodes the \p pKey as a DER buffer, and writes the result in \p pDest.
 *
 *   The encoding process is not predictable, and a buffer of sufficient length
 *   must be provided. A rule of thumb is to provide a buffer which is at least
 *   8 times longer than the key (8 * ::SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes ).
 *
 *   When SOPC_STATUS_NOK is returned, the function may be called again with a larger buffer.
 *
 * \param pKey         A valid pointer to the asymmetric key (public/private) to encode.
 * \param is_public    Whether the key is public or private.
 * \param[out] pDest   A valid pointer to the buffer which will receive the DER encoded key.
 * \param lenDest      The length in bytes of the buffer \p pDest.
 * \param[out] pLenWritten  A valid pointer to the number of bytes written to pDest.
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
 * \brief  Write an asymmetric key to a PEM file.
 *
 * \param pKey      A valid pointer to the asymmetric key (public/private).
 * \param bIsPublic Whether the key is public or private.
 * \param filePath  Path to the file.
 * \param pwd       An optional password (!= NULL). The password must be a zero-terminated string with
 *                  at most \p pwdLen non null chars, and at least \p pwdLen + 1 allocated chars.
 * \param pwdLen    The length of the password.
 *
 * \note            The supported encryption algorithm is AES-256-CBC. \p pwd and \p pwdLen are used only
 *                  to encrypt the key when it is private.
 *
 * \warning         Only PKCS#1 format is supported, In other words, the function is limited to RSA keys.
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToPEMFile(SOPC_AsymmetricKey* pKey,
                                                          const bool bIsPublic,
                                                          const char* filePath,
                                                          const char* pwd,
                                                          const uint32_t pwdLen);

/**
 * \brief Creates a serialized asymmetric key from an ::SOPC_AsymmetricKey structure.
 *
 * \param pKey      A valid pointer to the asymmetric key (public/private) to serialize.
 * \param is_public Whether the key is public or private.
 * \param[out] out  the newly allocated serialized key
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out);

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
 * \param[out] ppCert Creation: a valid handle which will point to the newly created Certificate.
 *                    Addition: a pointer to a pointer to a Certificate list to which add the certificate.
 *                    In either cases, this object must be freed with a call to ::SOPC_KeyManager_Certificate_Free .
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
 * \param szPath      The path to the DER/PEM file.
 * \param[out] ppCert Creation: a valid pointer pointing to NULL which will be set to the newly created Certificate.
 *                    Addition: a pointer to a pointer to a Certificate list to which add the certificate.
 *                    In either cases, this object must be freed with a call to ::SOPC_KeyManager_Certificate_Free
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
 * \brief           Frees a Certificate created with ::SOPC_KeyManager_Certificate_CreateOrAddFromFile or
 *                  ::SOPC_KeyManager_Certificate_CreateOrAddFromDER
 *
 * \warning         You must not free a Certificate for which a key is still being used.
 *                  ::SOPC_KeyManager_AsymmetricKey_CreateFromCertificate .
 *
 * \param pCert     The Certificate to free.
 */
void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert);

/**
 * \brief           Encodes a \p pCert as a DER buffer and writes the result in \p ppDest.
 *
 * \param pCert       A valid pointer to the Certificate. There must be only one certificate in the list.
 * \param[out] ppDest A valid pointer pointing to NULL which will be set to the newly created buffer storing
 *                    the DER certificate content.
 *                    The allocated buffer must be freed by the caller using ::SOPC_KeyManager_Certificate_Free.
 * \param[out] pLenAllocated  A valid pointer for which pointed value will be set to the length of the allocated buffer.
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
 * \brief           Write all the certificates of \p pCerts in DER files
 *                  at destination \p directoryPath . File names are defined using the SHA1 of the certificates.
 *
 * \param pCerts         A valid pointer to the certificate list.
 * \param directoryPath  The directory path to write the DER files.
 *
 * \return          SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER_Files(SOPC_CertificateList* pCerts, const char* directoryPath);

/**
 * \brief           Computes and writes the thumbprint of \p pCert to \p pDest.
 *
 *   The computation of the thumbprint and its size depends on the current security policy.
 *   The thumbprint is usually a SHA digest of the DER representation of the certificate.
 *
 *   The size of the thumbprint is provided by ::SOPC_CryptoProvider_CertificateGetLength_Thumbprint .
 *
 * \param pProvider An initialized cryptographic context.
 * \param pCert     A valid pointer to the signed public key to thumbprint.
 * \param[out] ppDest  A valid pointer pointing to NULL which will be set
 *                    to the newly allocated buffer containing the thumbprint.
 * \param[out] lenDest A valid pointer (!= NULL) to store the length in bytes allocated in \p ppDest .
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
                                                            uint8_t** ppDest,
                                                            uint32_t* lenDest);

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
 * \param[out] ppApplicationUri A valid pointer pointing to NULL which will be set
 *                              to the newly allocated zero-terminated string containing the application URI.
 * \param[out] pStringLength    Optional pointer to the string length (excluding the trailing \0).
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
 * \param pCert        The certificate or certificate list.
 * \param[out] pLength A valid pointer to the computed length of the list.
 *
 * \note            Content of the output is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength);

/**
 * \brief          Returns the subject name of certificate \p pCert as a C String.
 *
 * \param pCert                The certificate.
 * \param[out] ppSubjectName   A valid pointer pointing to NULL which will be set to the newly
 *                             subject name of certificate \p pCert (NULL terminated C string)
 * \param[out] pSubjectNameLen The length of \p ppSubjectName .
 *
 * \note            Content of the output is unspecified when the value returned is not SOPC_STATUS_OK.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \return          SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSubjectName(const SOPC_CertificateList* pCert,
                                                             char** ppSubjectName,
                                                             uint32_t* pSubjectNameLen);

/**
 * \brief          Returns all the DNS names of certificate \p pCert as an array of C String.
 *
 * \param pCert                The certificate.
 * \param[out] ppDnsNameArray  A valid pointer pointing to NULL which will be set to the newly
 *                             allocated array of DNS names for the certificate \p pCert
 *                             (each name shall be a NULL terminated C string)
 * \param[out] pArrayLength    The length of \p ppDnsNameArray .
 *
 * \note            Content of the output is unspecified when the value returned is not SOPC_STATUS_OK.
 *
 * \warning         \p pCert must contain a single certificate. No error is returned if no DNS is defined.
 *
 * \return          SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSanDnsNames(const SOPC_CertificateList* pCert,
                                                             char*** ppDnsNameArray,
                                                             uint32_t* pArrayLength);

/**
 * \brief           Finds whether a certificate is in the given certificate list or not.
 *
 * \param pList        An optional pointer to the Certificate list.
 * \param pCert        An optional pointer to a single Certificate to find in the list.
 * \param[out] pbMatch A valid pointer to the result of the find.
 *                     True indicates that the certificate was found in the list.
 *                     Otherwise false.
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
 * \brief                   Remove a single Certificate from its thumbprint.
 *                          If the Certificate is a CA Certificate then all the CRLs for that CA are removed.
 *
 * \warning                 This function will fail if \p pThumbprint does not match the SHA1 length.
 *                          If \p ppCertList becomes empty, the list is freed and its content is set to NULL.
 *                          If \p ppCRLList becomes empty, the list is freed and its content is set to NULL.
 *
 * \param ppCertList        A valid pointer to the Certificate list.
 * \param ppCRLList         A valid pointer to the CRL list.
 * \param pThumbprint       The SHA1 of the certificate formatted as a hexadecimal C string (NULL terminated)
 *                          40 bytes shall be allocated in \p pThumbprint (+ 1 byte for the NULL character)
 * \param[out] pbMatch      A valid pointer indicating whether the certificate has been found and deleted.
 * \param[out] pbIsIssuer   A valid pointer indicating whether the deleted certificate is an issuer.
 *
 * \return                  SOPC_STATUS_OK when successful
 */
SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(SOPC_CertificateList** ppCertList,
                                                                     SOPC_CRLList** ppCRLList,
                                                                     const char* pThumbprint,
                                                                     bool* pbMatch,
                                                                     bool* pbIsIssuer);

/**
 * \brief           Returns the SHA-1 thumbprint of a certificate.
 *
 * \param pCert     A pointer to a single Certificate.
 *
 * \warning         \p pCert must contain a single certificate.
 *
 * \note            The returned SHA-1 Cstring must be freed by the caller.
 *
 * \return          NULL if error otherwise the SHA-1 thumbprint of \p pCert .
 */
char* SOPC_KeyManager_Certificate_GetCstring_SHA1(const SOPC_CertificateList* pCert);

/**
 * \brief                Whether the first item of a certificate list is self signed.
 *
 * \param pCert               A valid pointer to the certificate list.
 * \param[out] pbIsSelfSigned A valid pointer to the result.
 *
 * \note            Only the first certificate of \p pCert is processed.
 *
 * \return          SOPC_STATUS_OK when successful otherwise SOPC_STATUS_NOK.
 *
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_IsSelfSigned(const SOPC_CertificateList* pCert, bool* pbIsSelfSigned);

/**
 * \brief            Makes a copy of a given certificate list.
 *
 * \param pCert           A valid pointer to the certificate list to copy.
 * \param[out] ppCertCopy A valid pointer pointing to NULL which will be set
 *                        to the newly allocated certificate list copy.
 *                        Caller is responsible to call ::SOPC_KeyManager_Certificate_Free if needed.
 *
 * \return           SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_Copy(const SOPC_CertificateList* pCert,
                                                   SOPC_CertificateList** ppCertCopy);

/**
 * \brief Attach a DER certificate list to a serialized certificate array.
 *
 * \warning  The returned ::SOPC_SerializedCertificate array content shall not be modified and shall not be used
 *           after the certificate list \p pCerts is freed by ::SOPC_KeyManager_Certificate_Free.
 *           The array shall be deallocated by caller.
 *
 * \param pCerts                 The DER certificate list to attach
 * \param[out] pSerializedArray  The serialized certificate array
 * \param[out] pLenArray         The length of \p pSerializedArray
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_CertificateList_AttachToSerializedArray(const SOPC_CertificateList* pCerts,
                                                                          SOPC_SerializedCertificate** pSerializedArray,
                                                                          uint32_t* pLenArray);

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
 * \param bufferDER  A valid pointer to the buffer containing the DER description.
 * \param lenDER     The length in bytes of the DER description of the certificate.
 * \param[out] ppCRL Creation: a valid pointer pointing to NULL which will be set to the newly created CRL.
 *                   Addition: a pointer to a pointer to a CRL list to which add the CRL.
 *                   In either cases, this object must be freed with a call to ::SOPC_KeyManager_CRL_Free .
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
 * \param szPath     The path to the DER/PEM file.
 * \param[out] ppCRL Creation: a valid pointer pointing to NULL which will be set to the newly created CRL.
 *                   Addition: a pointer to a pointer to a CRL list to which add the CRL.
 *                   In either cases, this object must be freed with a call to ::SOPC_KeyManager_CRL_Free .
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
 * \brief           Write all the CRL ( \p pCrls ) in DER files.
 *                  at destination \p directoryPath . File names are defined using the SHA1 of the crls.
 *
 * \param pCrls          A valid pointer to the CRL list.
 * \param directoryPath  The directory path to write the DER files.
 *
 * \return          SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_ToDER_Files(SOPC_CRLList* pCrls, const char* directoryPath);

/**
 * \brief            Makes a copy of a given CRL list.
 *
 * \param pCrl            A valid pointer to the CRL list to copy.
 * \param[out] ppCrlCopy  A valid pointer pointing to NULL which will be set to the newly allocated CRL list copy.
 *                        Caller is responsible to call ::SOPC_KeyManager_CRL_Free if needed.
 *
 * \return          SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_Copy(const SOPC_CRLList* pCrl, SOPC_CRLList** ppCrlCopy);

/**
 * \brief           Returns the number of chained CRL in \p pCrl list.
 *
 * \param pCrl           A valid pointer to the CRL list.
 * \param[out] pLength   A valid pointer to the computed length of the list.
 *
 * \note            Content of the output is unspecified when the returned value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_GetListLength(const SOPC_CRLList* pCrl, size_t* pLength);

/**
 * \brief Attach a DER CRL list to a serialized CRL array.
 *
 * \warning  The returned ::SOPC_SerializedCRL array content shall not be modified and shall not be used
 *           after the CRL list \p pCRLs is freed by ::SOPC_KeyManager_CRL_Free.
 *           The array shall be deallocated by caller.
 *
 * \param pCRLs                  The DER CRL list to attach
 * \param[out] pSerializedArray  The serialized CRL array
 * \param[out] pLenArray         The length of \p pSerializedArray
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRLList_AttachToSerializedArray(const SOPC_CRLList* pCRLs,
                                                                  SOPC_SerializedCRL** pSerializedArray,
                                                                  uint32_t* pLenArray);

/**
 * \brief           Frees a Certificate created with ::SOPC_KeyManager_CRL_CreateOrAddFromFile or
 *                  ::SOPC_KeyManager_CRL_CreateOrAddFromDER .
 *
 * \param pCRL      The CRL to free.
 */
void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL);

/* ------------------------------------------------------------------------------------------------
 * Certificate Signing request API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief              Create a certificate signing request signed with \p pKey
 *
 * \param subjectName  The subject name to set. The format is a sequence of name (OID types)
 *                     value pairs separated by a ‘,’.
 * \param bIsServer    Whether this CSR is to request a server or a client certificate.
 * \param mdType       The MD algorithm (terminated by '\0') use for the signature eg SHA1, SHA256...
 * \param uri          The application URI (terminated by '\0'). Shall not be NULL.
 * \param pDnsArray    Array of DSN names of the server (name terminated by '\0'). Shall not be NULL.
 *                     Array is not modified by the function.
 * \param arrayLength  The length of \p pDnsArray.
 * \param[out] ppCSR   A handle to the created CSR. This object must be freed
 *                     with a call to ::SOPC_KeyManager_CSR_Free .
 *
 * \note  The keyUsage is filled with digitalSignature, nonRepudiation, keyEncipherment and dataEncipherment.
 *        The extendedKeyUsage is filled with serverAuth if \p bIsServer is true, otherwise clientAuth.
 *        The subject alternative name is filled with \p uri and/or \p dns .
 *        The basic constraints is set to false for the CA flag.
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_CSR_Create(const char* subjectName,
                                             const bool bIsServer,
                                             const char* mdType,
                                             const char* uri,
                                             char** pDnsArray,
                                             uint32_t arrayLength,
                                             SOPC_CSR** ppCSR);

/**
 * \brief           Encodes CSR \p pCSR as a DER buffer and writes the result in \p ppDest.
 *
 * \param pCSR     A valid pointer to the CSR.
 * \param pKey     A valid pointer to the asymmetric key. The key shall be private.
 *                 The key is attached to the CSR but not freed by ::SOPC_KeyManager_CSR_Free .
 * \param[out] ppDest   A valid pointer pointing to NULL which will be set to the newly created buffer storing the DER.
 *                      The allocated buffer must be freed by the caller.
 * \param[out] pLenAllocated  A valid pointer to the length allocated by this operation.
 *
 * \note           Content of the outputs is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_CSR_ToDER(SOPC_CSR* pCSR,
                                            SOPC_AsymmetricKey* pKey,
                                            uint8_t** ppDest,
                                            uint32_t* pLenAllocated);

/**
 * \brief              Frees a CSR created with ::SOPC_KeyManager_CSR_Create
 *
 * \param pCSR         The CSR to free.
 */
void SOPC_KeyManager_CSR_Free(SOPC_CSR* pCSR);

/**
 * \brief Checks if each CA certificate from \p pCert have a revocation list available in \p pCRL.
 *
 *   This function does not set match to false if there are CRL that do not match any Certificate.
 *   This function skips certificates in \p pCert that are not authorities.
 * *
 * \param pCert       A valid pointer to the crt
 * \param pCRL        A valid pointer to the CRL list.
 * \param[out] bMatch A valid pointer to a boolean value to the set result of the test.
 *                    true value indicates that each CA certificate in \p pCert
 *                    has at least one associated CRL in \p pCRL.
 *                    Otherwise false.
 *
 * \note            Content of \p bMatch is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_KeyManager_CertificateList_CheckCRL(SOPC_CertificateList* pCert,
                                                           const SOPC_CRLList* pCRL,
                                                           bool* bMatch);

#endif /* SOPC_KEY_MANAGER_LIB_ITF_H_ */
