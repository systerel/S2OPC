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

#ifndef SOPC_PUB_FIXED_BUFFER_H_
#define SOPC_PUB_FIXED_BUFFER_H_

#include <stdint.h>

#include "sopc_buffer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_enums.h"
#include "sopc_pubsub_security.h"

typedef struct SOPC_PubFixedBuffer_Buffer_Ctx SOPC_PubFixedBuffer_Buffer_Ctx;
typedef struct SOPC_PubFixedBuffer_DataSetField_Position SOPC_PubFixedBuffer_DataSetField_Position;

/**
 * @brief Create and initialize preencode context against nm.
 *
 * @param nm NetworkMessage used to initialize preencoded buffer context.
 * @param security security structure used to initialize preencoded buffer context. Can be NULL if none security policy
 * used
 *
 * @return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus SOPC_DataSet_LL_NetworkMessage_Create_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                         SOPC_PubSub_SecurityType* security);

/**
 * @brief Free memory allocated by ::SOPC_DataSet_LL_NetworkMessage_Create_Preencode_Buffer.
 *
 * @param preencode Pointer to preencode buffer context to be freed
 */
void SOPC_PubFixedBuffer_Delete_Preencode_Buffer(SOPC_PubFixedBuffer_Buffer_Ctx* preencode);

/**
 * @brief Get pointer to updated preencode buffer stored in \p preencode structure.
 *
 * @note This buffer shouldn't be freed nor modified by user.
 *
 * @param preencode Context which saved the buffer and elements to update it.
 * @param security  security structured used to sign the buffer in case of Sign policy security.
 *                  This structure can be NULL if security policy is None.
 *
 * @return The pointer to networkMessage buffer in case of success, NULL otherwise
 */
SOPC_Buffer* SOPC_PubFixedBuffer_Get_UpdatedBuffer(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                   const SOPC_PubSub_SecurityType* security);

/**
 * @brief Store the position of the sequence number in final networkMessage buffer
 *
 * @param preencode Context to store the information
 * @param position position of the sequence number in the buffer
 * @param index index of the dataSetMessage for which sequence number is associated
 *
 * @return true in case of success, false otherwise
 */
bool SOPC_PubFixedBuffer_Set_DSM_SequenceNumber_Position_At(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                            uint32_t position,
                                                            size_t index);

/**
 * @brief Store the position of the timestamp in final networkMessage buffer
 *
 * @param preencode Context to store the information
 * @param position position of the timestamp in the buffer
 * @param index index of the dataSetMessage for which timestamp is associated
 *
 * @return true in case of success, false otherwise
 */
bool SOPC_PubFixedBuffer_Set_DSM_Timestamp_Position_At(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                       uint32_t position,
                                                       size_t index);

/**
 * @brief Store the position of Nonce message of security header in final networkMessage buffer
 *
 * @param preencode Context to store the information
 * @param position position of the Nonce message in the buffer
 */
void SOPC_PubFixedBuffer_Set_Nonce_Message_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position);

/**
 * @brief Store the position of the tokenId of security header in final networkMessage buffer
 *
 * @param preencode Context to store the information
 * @param position position of the tokenId in the buffer
 */
void SOPC_PubFixedBuffer_Set_TokenId_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position);

/**
 * @brief Store the position of the signature in final networkMessage buffer
 *
 * @param preencode Context to store the information
 * @param position position of the signature in the buffer
 */
void SOPC_PubFixedBuffer_Set_Sign_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position);

/**
 * @brief Get the structure which store dataSetField context in preencode context.
 *
 * @param preencode Context which stored the dataSetField information
 * @param index index of the dataSetField in NetworkMessage
 *
 * @return Pointer to the structure storing dataSetField context or NULL in case of failure
 */
SOPC_PubFixedBuffer_DataSetField_Position* SOPC_PubFixedBuffer_Get_DataSetField_Position_At(
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
    size_t index);

/**
 * @brief Store the position of tha DataSetField in final networkMessage buffer
 *
 * @param dsfPos Pointer to the context which store the dataSetField position. Should NOT be NULL.
 * @param position Position of the dataSetField in buffer.
 */
void SOPC_PubFixedBuffer_DataSetFieldPosition_Set_Position(SOPC_PubFixedBuffer_DataSetField_Position* dsfPos,
                                                           uint32_t position);
#endif /* SOPC_PUB_FIXED_BUFFER_H_ */
