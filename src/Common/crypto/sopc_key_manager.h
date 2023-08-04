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
 * (Certificate) and the corresponding private key. Also see sopc_key_manager_lib_itf.h for the complete API.
 *
 * KeyManager is different than PKIProvider, which only handles signed public key validation and storage.
 *
 * KeyManager API is context-less. The KeyManager is generic, and is not linked to the current security policy.
 */

#ifndef SOPC_KEY_MANAGER_H_
#define SOPC_KEY_MANAGER_H_

#include "sopc_crypto_decl.h"

// Must be included last
#include "sopc_key_manager_lib_itf.h"

/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief Creates a serialized asymmetric key from a DER or PEM payload.
 *
 * \param data     the key data in DER or PEM format
 * \param len      length of the data
 * \param[out] key the created serialized key
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(const uint8_t* data,
                                                                         uint32_t len,
                                                                         SOPC_SerializedAsymmetricKey** key);

/**
 * \brief Creates a serialized asymmetric key from a file in DER or PEM format.
 *
 * \param path     path to the file
 * \param[out] key the created serialized key
 *
 * \deprecated  Use ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd instead
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(const char* path,
                                                                         SOPC_SerializedAsymmetricKey** key);

/**
 * \brief               Creates a serialized asymmetric key from a file in DER or PEM format
 *                      with an optional \p password for the encrypted private key (PEM format).
 *
 * \param keyPath       The path to the DER/PEM file.
 * \param[out] key      A valid pointer pointing to NULL which will be set to the newly allocated serialized key.
 * \param password      An optional password (!= NULL). The password must be a zero-terminated string with
 *                      at most \p lenPassword non null chars, and at least \p lenPassword + 1 allocated chars.
 * \param lenPassword   The length of the password.
 *
 * \note                Supported encryption algorithm: AES-128-CBC, AES-192-CBC and AES-256-CBC
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(const char* keyPath,
                                                                                 SOPC_SerializedAsymmetricKey** key,
                                                                                 char* password,
                                                                                 uint32_t lenPassword);

/**
 * \brief Creates a serialized asymmetric key from an ::SOPC_AsymmetricKey structure.
 *
 * \param pKey      A valid pointer to the asymmetric key (public/private) to serialize.
 * \param is_public Whether the key is public or private.
 * \param[out] out  A valid pointer pointing to NULL which will be set to the newly allocated serialized key
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out);

/**
 * \brief Deserializes a serialized key.
 *
 * \param key        the serialized key
 * \param is_public  whether the serialized key is a public or a private key
 * \param[out] res   A valid pointer pointing to NULL which will be set to the newly allocated SOPC_AsymmetricKey
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
 * \brief Creates a serialized certificate from a DER payload.
 *
 * \param der        the certificate data in DER format
 * \param len        length of the DER data
 * \param[out] cert  A valid pointer pointing to NULL which will be set to the newly allocated serialized certificate
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromDER(const uint8_t* der,
                                                                      uint32_t len,
                                                                      SOPC_SerializedCertificate** cert);

/**
 * \brief Creates a serialized certificate from a file in DER or PEM format.
 *
 * \param path       path to the file
 * \param[out] cert  A valid pointer pointing to NULL which will be set to the newly allocated serialized certificate
 *
 * \return \c SOPC_STATUS_OK on success, or an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromFile(const char* path,
                                                                       SOPC_SerializedCertificate** cert);

/**
 * \brief Releases all resources associated to a serialized certificate.
 *
 * \param cert  The serialized certificate
 */
void SOPC_KeyManager_SerializedCertificate_Delete(SOPC_SerializedCertificate* cert);

/**
 * \brief Deserializes a serialized certificate.
 *
 * \param cert     the serialized certificate
 * \param[out] res A valid pointer pointing to NULL which will be set to the newly allocated SOPC_CertificateList
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
 * \brief Returns the data held in a serialized CRL
 *
 * \param crl  the serialized CRL
 *
 * \return The data held in the serialized CRL. The returned memory is owned
 *         by the serialized CRL, and should not be modified or freed.
 */
const SOPC_Buffer* SOPC_KeyManager_SerializedCRL_Data(const SOPC_SerializedCRL* crl);

#endif /* SOPC_KEY_MANAGER_H_ */
