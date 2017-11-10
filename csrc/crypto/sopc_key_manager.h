/** \file sopc_key_manager.h
 *
 * The KeyManager provides an API for Asymmetric Key Management such as loading signed public keys (Certificate)
 * and the corresponding private key.
 *
 * KeyManager is different than PKIProvider, which only handles signed public key validation and storage.
 *
 * KeyManager API is context-less. The KeyManager is generic, and is not linked to the current security policy.
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

#ifndef SOPC_KEY_MANAGER_H_
#define SOPC_KEY_MANAGER_H_

#include "sopc_crypto_decl.h"
#include "sopc_toolkit_constants.h"

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
 * \param buffer    A valid pointer to the buffer containing the DER or PEM description.
 * \param lenBuf    The length in bytes of the DER/PEM description of the key.
 * \param ppKey     A handle to the created key. This object must be freed with a call to
 * KeyManager_AsymmetricKey_Free().
 *
 * \note            Content of the key is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t* buffer,
                                                               uint32_t lenBuf,
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
 * \note            Content of the key is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_AsymmetricKey_CreateFromFile(const char* szPath,
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
 * \note            Content of the certificate is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_Certificate* pCert,
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
 *                  When STATUS_NOK is returned, the function may be called again with a larger buffer.
 *
 * \param pKey      A valid pointer to the asymmetric key (public/private) to encode.
 * \param pDest     A valid pointer to the buffer which will receive the DER encoded key.
 * \param lenDest   The length in bytes of the buffer \p pDest.
 * \param pLenWritten  A valid pointer to the number of bytes written to pDest.
 *
 * \note            Content of the output is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_AsymmetricKey_ToDER(const SOPC_AsymmetricKey* pKey,
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
 * \note            Content of the certificate is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_Certificate_CreateFromDER(const uint8_t* bufferDER,
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
 * \note            Content of the certificate is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_Certificate_CreateFromFile(const char* szPath, SOPC_Certificate** ppCert);

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
 * \note            Content of the output is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_Certificate_CopyDER(const SOPC_Certificate* pCert,
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
 * \note            Content of the output is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                          const SOPC_Certificate* pCert,
                                                          uint8_t* pDest,
                                                          uint32_t lenDest);

#endif /* SOPC_KEY_MANAGER_H_ */
