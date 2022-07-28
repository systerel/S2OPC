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

#ifndef SOPC_PUBSUB_SECURITY_H_
#define SOPC_PUBSUB_SECURITY_H_

#include "sopc_crypto_provider.h"
#include "sopc_enums.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"

/**
 * Security data related to a WriterGroup or ReaderGroup
 */
typedef struct SOPC_PubSub_SecurityType
{
    SOPC_SecurityMode_Type mode;
    SOPC_CryptoProvider* provider;
    SOPC_LocalSKS_Keys* groupKeys;
    SOPC_ExposedBuffer* msgNonceRandom;
    uint32_t sequenceNumber;
} SOPC_PubSub_SecurityType;

/**
 * \brief Callback function to retrieve a security context from a security tokenId, publisher id and a writer group
 *
 * \param tokenId token id to identify the group keys
 * \param pub_id publisher id
 * \param writerGroupId writer group id
 *
 * \return a SOPC_PubSub_SecurityType object or NULL is not found
 */
typedef SOPC_PubSub_SecurityType* SOPC_UADP_GetSecurity_Func(uint32_t tokenId,
                                                             SOPC_Conf_PublisherId pub_id,
                                                             uint16_t writerGroupId);

/**
 * \brief clear a SOPC_PubSub_SecurityType object
 *
 * \param security SOPC_PubSub_SecurityType object to clear. Should not be NULL.
 */
void SOPC_PubSub_Security_Clear(SOPC_PubSub_SecurityType* security);

/**
 * \brief Encrypt a buffer
 *
 * \warning the payload position shall be 0 in the buffer
 *
 * \param security SOPC_PubSub_SecurityType object containing SecurityProfile and Keys. Should not be NULL.
 * \param payload buffer to encrypt. Data starting from position 0 are encrypted. Should not be NULL.
 * \return If succeed, a buffer which receive the encrypted data. Otherwise return NULL.
 */
SOPC_Buffer* SOPC_PubSub_Security_Encrypt(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* payload);

/**
 * \brief Decrypt the payload of a buffer
 *
 * \param keys object containing crypto function and keys to encrypt.
 * \param src buffer to decrypt. Data to decrypt starts from current position of this buffer.
 * \param size number of byte to decrypt. Remaing byte of the given buffer should be equal or greater.
 * \return If succeed, the returned buffer contains only the decrypted payload. Otherwise return NULL.
 */
SOPC_Buffer* SOPC_PubSub_Security_Decrypt(const SOPC_PubSub_SecurityType* keys, SOPC_Buffer* src, uint32_t size);

/**
 * \brief Get the size of signature.
 *
 * \param security SOPC_PubSub_SecurityType object containing SecurityProfile and Keys. Should not be NULL.
 * \param enabled indicates if the signature is enabled. If it is not, size is 0.
 * \param length a valid pointer which received the size of signature. Size is given in byte.
 * \return SOPC_STATUS_OK only if succeed.
 */
SOPC_ReturnStatus SOPC_PubSub_Security_GetSignSize(const SOPC_PubSub_SecurityType* security,
                                                   bool enabled,
                                                   uint32_t* length);

/**
 * \brief Sign from 0 to current position and add signature after current position
 *
 * \warning the signature is done for buffer position 0 until current position and added at current position
 *
 * \param security SOPC_PubSub_SecurityType object containing SecurityProfile and Keys. Should not be NULL.
 * \param src buffer to sign.
 */
SOPC_ReturnStatus SOPC_PubSub_Security_Sign(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* src);

/**
 * \brief Verify the signature of a buffer.
 *
 * Signature's Byte should be at end of the buffer.
 *
 *
 * \param security SOPC_PubSub_SecurityType object containing SecurityProfile and Keys. Should not be NULL.
 * \param src buffer to verify.
 * \param payloadPosition position of the UADP payload message in buffer.
 *        It is necessary since not always 0 (e.g.: Ethernet buffer contains the Ethernet header).
 *
 * \return
 *  - True if the buffer is signed and the signature are equal
 *  - False if security or src are NULL or the signature are not equal
 */
bool SOPC_PubSub_Security_Verify(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* src, uint32_t payloadPosition);

SOPC_ExposedBuffer* SOPC_PubSub_Security_Random(const SOPC_CryptoProvider* provider);

#endif /* SOPC_PUBSUB_SECURITY_H_ */
