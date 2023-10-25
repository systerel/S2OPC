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

/** \file sopc_key_cert_pair.h
 *
 * \brief The ::SOPC_KeyCertPair stores a private key and certificate pair
 *        and allows to update it and triggering an associated external treatment.
 *        ::SOPC_KeyCertPair is thread-safe for accessing / updating the key and certificate pair.
 */

#ifndef SOPC_KEY_CERT_PAIR_H
#define SOPC_KEY_CERT_PAIR_H

#include <stddef.h>
#include <stdint.h>

#include "sopc_enums.h"
#include "sopc_key_manager.h"

/**
 * \brief An abstract structure used to store a pair of private key and certificate.
 */
typedef struct SOPC_KeyCertPair SOPC_KeyCertPair;

/**
 * \brief Type of the callback triggered on key / certificate update
 *
 * \param updateParam a user defined parameter for the callback
 */
typedef void SOPC_KeyCertPairUpdateCb(uintptr_t updateParam);

/**
 * \brief Creates a private key / certificate pair from file paths.
 *        The private key decryption password shall be provided if the key file is encrypted.
 *
 * \param certPath the file path to a X509 certificate in DER format
 * \param privateKeyPath the file path to a private key in PEM or DER format (PEM if encrypted)
 * \param keyPassword (optional) the password to decrypt the private key file if it is encrypted or NULL.
 *                               If not NULL it shall be a NULL terminated C string.
 * \param[out] ppKeyCertPair A pointer to the newly allocated and filled with key/certificate pair in case of success
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case a parameter is NULL
 *         (except \p keyPassword ), SOPC_STATUS_OUT_OF_MEMORY in case of memory issue
 *         or SOPC_STATUS_NOK in other cases.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_CreateFromPaths(const char* certPath,
                                                   const char* privateKeyPath,
                                                   char* keyPassword,
                                                   SOPC_KeyCertPair** ppKeyCertPair);

/**
 * \brief Creates a private key / certificate pair from bytes arrays.
 *
 * \param certificateNbBytes the number of bytes in \p certificate
 * \param certificate the bytes array containing one X509 certificate (DER / PEM)
 * \param keyNbBytes the number of bytes in \p privateKey
 * \param privateKey the bytes array containing the private key associated to \p certificate (DER / PEM)
 * \param[out] ppKeyCertPair A pointer to the newly allocated and filled with key/certificate pair in case of success
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case a parameter is NULL
 *         (except \p keyPassword ) and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_CreateFromBytes(size_t certificateNbBytes,
                                                   const unsigned char* certificate,
                                                   size_t keyNbBytes,
                                                   const unsigned char* privateKey,
                                                   SOPC_KeyCertPair** ppKeyCertPair);

/**
 * \brief Defines the callback to be called when a key/certificate update is done with
 * ::SOPC_KeyCertPair_UpdateFromBytes. It is mandatory to define an associated behavior to allow calls to
 * ::SOPC_KeyCertPair_UpdateFromBytes. This is used to re-evaluate connections using the previous key/certificate.
 *
 * \param keyCertPair the key / certificate pair for which an update callback will be defined
 * \param updateCb the callback to be called when a key/certificate update is done
 * \param updateParam a user defined parameter for the callback
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameter
 *         and SOPC_STATUS_INVALID_STATE in case a callback is already defined.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_SetUpdateCb(SOPC_KeyCertPair* keyCertPair,
                                               SOPC_KeyCertPairUpdateCb* updateCb,
                                               uintptr_t updateParam);

/**
 * \brief Updates the private key and/or certificate of the pair from bytes arrays.
 *        If the certificate uses the same public/private keys pair, the private key might not be updated.
 *
 * \warning A callback shall have been defined using ::SOPC_KeyCertPair_SetUpdateCb otherwise update is not authorized
 *
 * \param keyCertPair the key / certificate pair for which an update will be done
 * \param certificateNbBytes the number of bytes in \p certificate
 * \param certificate the bytes array containing one X509 certificate (DER / PEM)
 * \param keyNbBytes (optional) the number of bytes in \p privateKey or 0
 * \param privateKey (optional) the bytes array containing the private key associated to \p certificate (DER / PEM)
 *                              or NULL if the certificate public key is unchanged
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case a parameter is NULL
 *         (except \p privateKey) or invalid, SOPC_STATUS_INVALID_STATE in case update callback is not set
 *         and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_UpdateFromBytes(SOPC_KeyCertPair* keyCertPair,
                                                   size_t certificateNbBytes,
                                                   const unsigned char* certificate,
                                                   size_t keyNbBytes,
                                                   const unsigned char* privateKey);

/**
 * \brief Gets a copy of the serialized certificate contained in the key /certificate pair
 *
 * \param keyCertPair the key / certificate pair for which a copy of the serialized certificate is requested
 * \param[out] ppCertCopy a pointer to the newly allocated serialized certificate copy in case of success.
 *                        It shall be deallocated by caller using ::SOPC_KeyManager_SerializedCertificate_Delete.
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameter
 *         and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_GetSerializedCertCopy(SOPC_KeyCertPair* keyCertPair,
                                                         SOPC_SerializedCertificate** ppCertCopy);

/**
 * \brief Gets a copy of the certificate contained in the key /certificate pair
 *
 * \param keyCertPair the key / certificate pair for which a copy of the certificate is requested
 * \param[out] ppCertCopy a pointer to the newly allocated certificate copy in case of success.
 *                        It shall be deallocated by caller using ::SOPC_KeyManager_Certificate_Free.
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameter
 *         and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_GetCertCopy(SOPC_KeyCertPair* keyCertPair, SOPC_CertificateList** ppCertCopy);

/**
 * \brief Gets a copy of the private key contained in the key /certificate pair
 *
 * \param keyCertPair the key / certificate pair for which a copy of the key is requested
 * \param[out] ppKeyCopy a pointer to the newly allocated key copy in case of success.
 *                       It shall be deallocated by caller using ::SOPC_KeyManager_AsymmetricKey_Free.
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case of NULL parameter
 *         and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_GetKeyCopy(SOPC_KeyCertPair* keyCertPair, SOPC_AsymmetricKey** ppKeyCopy);

/**
 * \brief Clears and frees the key / certificate pair and set pointer to NULL
 *
 * \param[out] ppKeyCertPair pointer to the key / certificate pair to delete, it is set to NULL in case of success
 */
void SOPC_KeyCertPair_Delete(SOPC_KeyCertPair** ppKeyCertPair);

/**
 * \brief An abstract structure used to store a certificate in a thread-safe context.
 */
typedef SOPC_KeyCertPair SOPC_CertHolder;

/**
 * \brief Creates a certificate holder from file path.
 *
 * \note ::SOPC_KeyCertPair functions should then be used to access/update the stored certificate
 *
 * \param certPath the file path to a X509 certificate in DER format
 * \param[out] ppCertHolder A pointer to the newly allocated holder filled with certificate holder in case of success
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case a parameter is NULL
 *         SOPC_STATUS_OUT_OF_MEMORY in case of memory issue
 *         or SOPC_STATUS_NOK in other cases.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_CreateCertHolderFromPath(const char* certPath, SOPC_CertHolder** ppCertHolder);

/**
 * \brief Creates a certificate holder from bytes arrays.
 *
 * \note ::SOPC_KeyCertPair functions should then be used to access/update the stored certificate
 *
 * \param certificateNbBytes the number of bytes in \p certificate
 * \param certificate the bytes array containing one X509 certificate (DER / PEM)
 * \param[out] ppCertHolder A pointer to the newly allocated holder filled with certificate in case of success
 *
 * \return SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS in case a parameter is NULL
 *         and SOPC_STATUS_OUT_OF_MEMORY in case of memory issue.
 */
SOPC_ReturnStatus SOPC_KeyCertPair_CreateCertHolderFromBytes(size_t certificateNbBytes,
                                                             const unsigned char* certificate,
                                                             SOPC_CertHolder** ppCertHolder);

#endif /* SOPC_KEY_CERT_PAIR_H */
