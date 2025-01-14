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

#include <string.h>

#include "sopc_assert.h"
#include "sopc_buffer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_date_time.h"
#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pub_fixed_buffer.h"
#include "sopc_pubsub_security.h"

struct SOPC_PubFixedBuffer_DataSetField_Position
{
    // Pointer to DataSetField stored in SOPC_Dataset_LL_NetworkMessage structure
    const SOPC_Dataset_LL_DataSetField* dataSetField;
    // Position in preencode buffer
    uint32_t position;
};

typedef struct SOPC_PubFixedBuffer_Dsm_DynamicInfo
{
    // Pointer to dsm_sequence_number stored in SOPC_Dataset_LL_NetworkMessage structure
    const uint16_t* sequence_number;
    uint32_t sequence_number_position;
    // Pointer to dataset_message_timestamp stored in SOPC_DataSet_LL_NetworkMessage structure
    const uint64_t* timestamp;
    uint32_t timestamp_position;
} SOPC_PubFixedBuffer_Dsm_DynamicInfo;

struct SOPC_PubFixedBuffer_Buffer_Ctx
{
    // Array of pointer to sequenceNumber and timestamp stored in SOPC_DataSet_LL_NetworkMessage structure
    SOPC_PubFixedBuffer_Dsm_DynamicInfo* dynamic_dsm_info;
    size_t nb_dynamic_dsm_info;

    // Position of Nonce Message in security header of network message
    uint32_t nm_nonceMessage_position;

    // Position of token Id in security header of network message
    uint32_t nm_tokenId_position;

    // Position of signature and last position used to sign
    uint32_t signPosition;

    // Array of tuple pointer to dataSetField stored in SOPC_Dataset_LL_NetworkMessage structure and position in
    // preencode buffer
    SOPC_PubFixedBuffer_DataSetField_Position* dataSetFields;
    size_t dataSetFields_len;

    // Preencode network message
    SOPC_Buffer* buffer;
};

static SOPC_PubFixedBuffer_Buffer_Ctx* PubFixedBuffer_Create_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode_structure = SOPC_Calloc(1, sizeof(SOPC_PubFixedBuffer_Buffer_Ctx));
    SOPC_ASSERT(NULL != preencode_structure);

    uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);

    preencode_structure->nb_dynamic_dsm_info = (size_t) nbDsm;
    preencode_structure->dynamic_dsm_info =
        SOPC_Calloc(preencode_structure->nb_dynamic_dsm_info, sizeof(*preencode_structure->dynamic_dsm_info));
    SOPC_ASSERT(NULL != preencode_structure->dynamic_dsm_info);

    for (uint8_t iDsm = 0; iDsm < nbDsm; iDsm++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, iDsm);
        if (SOPC_Dataset_LL_DataSetMsg_Get_EnableEmission(dsm))
        {
            preencode_structure->dataSetFields_len += SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        }
    }

    preencode_structure->dataSetFields =
        SOPC_Calloc(preencode_structure->dataSetFields_len, sizeof(SOPC_PubFixedBuffer_DataSetField_Position));
    SOPC_ASSERT(NULL != preencode_structure->dataSetFields);

    preencode_structure->buffer = NULL;

    return preencode_structure;
}

SOPC_PubFixedBuffer_DataSetField_Position* SOPC_PubFixedBuffer_Get_DataSetField_Position_At(
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
    size_t index)
{
    if (NULL == preencode || index >= preencode->dataSetFields_len)
    {
        return NULL;
    }
    return preencode->dataSetFields + index;
}

static void PubFixedBuffer_Initialize_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode_structure = SOPC_DataSet_LL_NetworkMessage_Get_Preencode_Buffer(nm);
    SOPC_ASSERT(NULL != preencode_structure);

    uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    SOPC_ASSERT(preencode_structure->nb_dynamic_dsm_info == nbDsm);
    size_t index_dsf = 0;
    size_t index_dsm = 0;
    for (int i = 0; i < nbDsm; i++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
        if (SOPC_Dataset_LL_DataSetMsg_Get_EnableEmission(dsm))
        {
            const SOPC_DataSet_LL_UadpDataSetMessageContentMask* dsmContentMask =
                SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(dsm);
            if (NULL != dsmContentMask)
            {
                SOPC_PubFixedBuffer_Dsm_DynamicInfo* dynamicDsmInfo = &preencode_structure->dynamic_dsm_info[index_dsm];
                if (dsmContentMask->dataSetMessageSequenceNumberFlag)
                {
                    dynamicDsmInfo->sequence_number = SOPC_Dataset_LL_DataSetMsg_Get_SequenceNumberPointer(dsm);
                }
                if (dsmContentMask->timestampFlag)
                {
                    dynamicDsmInfo->timestamp = SOPC_Dataset_LL_DataSetMsg_Get_TimestampPointer(dsm);
                }
            }
            uint16_t nbDsf = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
            SOPC_ASSERT(preencode_structure->dataSetFields_len >= nbDsf + index_dsf);
            for (uint16_t j = 0; j < nbDsf; j++)
            {
                const SOPC_Dataset_LL_DataSetField* dsf = SOPC_Dataset_LL_DataSetMsg_Get_ConstDataSetField_At(dsm, j);
                SOPC_ASSERT(NULL != dsf);
                SOPC_PubFixedBuffer_DataSetField_Position* dsfPosition =
                    SOPC_PubFixedBuffer_Get_DataSetField_Position_At(preencode_structure, index_dsf);
                SOPC_ASSERT(NULL != dsfPosition);
                dsfPosition->dataSetField = dsf;
                index_dsf++;
            }
            index_dsm++;
        }
    }
}

SOPC_ReturnStatus SOPC_DataSet_LL_NetworkMessage_Create_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                         SOPC_PubSub_SecurityType* security)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode = PubFixedBuffer_Create_Preencode_Buffer(nm);
    SOPC_DataSet_LL_NetworkMessage_Set_Preencode_Buffer(nm, preencode);
    if (NULL == preencode)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        PubFixedBuffer_Initialize_Preencode_Buffer(nm);
        SOPC_Buffer* buffer_payload = NULL;
        SOPC_NetworkMessage_Error_Code code =
            SOPC_UADP_NetworkMessage_Encode_Buffers(nm, security, &preencode->buffer, &buffer_payload);
        if (SOPC_NetworkMessage_Error_Code_None != code || NULL == preencode->buffer || NULL == buffer_payload)
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Failed to preencode PUB message, SOPC_NetworkMessage_Error_Code is : %08X",
                                   (unsigned) code);
        }
        else
        {
            // Don't sign the buffer.
            code = SOPC_UADP_NetworkMessage_BuildFinalMessage(NULL, preencode->buffer, &buffer_payload);
            if (SOPC_NetworkMessage_Error_Code_None != code || NULL == preencode->buffer || NULL != buffer_payload)
            {
                status = SOPC_STATUS_NOK;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                       "Failed to merge preencode buffer, SOPC_NetworkMessage_Error_Code is : %08X",
                                       (unsigned) code);
            }
            if (SOPC_STATUS_OK == status && NULL != security)
            {
                // Signature is set at the end of the networkMessage
                SOPC_PubFixedBuffer_Set_Sign_Position(preencode, preencode->buffer->length);
            }
        }
    }
    return status;
}

void SOPC_PubFixedBuffer_Delete_Preencode_Buffer(SOPC_PubFixedBuffer_Buffer_Ctx** preencode)
{
    SOPC_ASSERT(NULL != preencode);
    if (NULL != *preencode)
    {
        SOPC_PubFixedBuffer_Buffer_Ctx* preencodeBuffer = *preencode;
        SOPC_Free(preencodeBuffer->dataSetFields);
        SOPC_Free(preencodeBuffer->dynamic_dsm_info);
        SOPC_Buffer_Delete(preencodeBuffer->buffer);
        SOPC_Free(*preencode);
        *preencode = NULL;
    }
}

bool SOPC_PubFixedBuffer_Set_DSM_SequenceNumber_Position_At(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                            uint32_t position,
                                                            size_t index)
{
    if (NULL == preencode || index >= preencode->nb_dynamic_dsm_info)
    {
        return false;
    }
    preencode->dynamic_dsm_info[index].sequence_number_position = position;
    return true;
}

bool SOPC_PubFixedBuffer_Set_DSM_Timestamp_Position_At(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                       uint32_t position,
                                                       size_t index)
{
    if (NULL == preencode || index >= preencode->nb_dynamic_dsm_info)
    {
        return false;
    }
    preencode->dynamic_dsm_info[index].timestamp_position = position;
    return true;
}

void SOPC_PubFixedBuffer_DataSetFieldPosition_Set_Position(SOPC_PubFixedBuffer_DataSetField_Position* dsfPos,
                                                           uint32_t position)
{
    SOPC_ASSERT(NULL != dsfPos);
    dsfPos->position = position;
}

void SOPC_PubFixedBuffer_Set_Nonce_Message_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position)
{
    SOPC_ASSERT(NULL != preencode);
    preencode->nm_nonceMessage_position = position;
}

void SOPC_PubFixedBuffer_Set_TokenId_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position)
{
    SOPC_ASSERT(NULL != preencode);
    preencode->nm_tokenId_position = position;
}

void SOPC_PubFixedBuffer_Set_Sign_Position(SOPC_PubFixedBuffer_Buffer_Ctx* preencode, uint32_t position)
{
    SOPC_ASSERT(NULL != preencode);
    preencode->signPosition = position;
}

/** Update value in buffer at position given with data. Length correspond to the number of bytes to update */
static void update_buffer(SOPC_Buffer* buffer, const void* data, uint32_t position, uint32_t length)
{
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(NULL != data);
    SOPC_Buffer_SetPosition(buffer, position);
    SOPC_Buffer_Write(buffer, (const uint8_t*) data, length);
}

SOPC_Buffer* SOPC_PubFixedBuffer_Get_UpdatedBuffer(SOPC_PubFixedBuffer_Buffer_Ctx* preencode,
                                                   const SOPC_PubSub_SecurityType* security)
{
    SOPC_ASSERT(NULL != preencode);
    SOPC_ASSERT(NULL != preencode->dynamic_dsm_info);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* updatedBuffer = preencode->buffer;
    for (size_t i = 0; i < preencode->nb_dynamic_dsm_info; i++)
    {
        SOPC_PubFixedBuffer_Dsm_DynamicInfo* dynamicDsmInfo = &preencode->dynamic_dsm_info[i];
        if (NULL != dynamicDsmInfo->sequence_number)
        {
            update_buffer(preencode->buffer, dynamicDsmInfo->sequence_number, dynamicDsmInfo->sequence_number_position,
                          sizeof(*dynamicDsmInfo->sequence_number));
        }
        if (NULL != dynamicDsmInfo->timestamp)
        {
            // Update Timestamp here
            uint64_t dsmTimestamp = (uint64_t) SOPC_Time_GetCurrentTimeUTC();
            update_buffer(preencode->buffer, &dsmTimestamp, dynamicDsmInfo->timestamp_position,
                          sizeof(*dynamicDsmInfo->timestamp));
        }
    }
    if (SOPC_STATUS_OK == status && 0 != preencode->nm_tokenId_position && NULL != security)
    {
        if (NULL == security->groupKeys)
        {
            updatedBuffer = NULL;
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_Buffer_SetPosition(updatedBuffer, preencode->nm_tokenId_position);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
            status = SOPC_UInt32_Write(&security->groupKeys->tokenId, updatedBuffer, 0);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
    }
    if (SOPC_STATUS_OK == status && 0 != preencode->nm_nonceMessage_position && NULL != security)
    {
        if (NULL == security->groupKeys)
        {
            updatedBuffer = NULL;
            status = SOPC_STATUS_NOK;
        }
        else
        {
            uint32_t nonceRandomLength = 0;
            status = SOPC_Buffer_SetPosition(updatedBuffer, preencode->nm_nonceMessage_position);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
            status = SOPC_CryptoProvider_PubSubGetLength_MessageRandom(security->provider, &nonceRandomLength);
            SOPC_ASSERT(4 == nonceRandomLength && // Size is fixed to 4 bytes. See OPCUA Spec Part 14 - Table 75
                        nonceRandomLength + 4 <= UINT8_MAX && // check before cast to uint8
                        SOPC_STATUS_OK == status);
            status = SOPC_PubSub_Security_Write_Nonce(security, updatedBuffer, (uint8_t) nonceRandomLength);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
    }
    for (size_t i = 0; i < preencode->dataSetFields_len && SOPC_STATUS_OK == status; i++)
    {
        SOPC_PubFixedBuffer_DataSetField_Position* dsfPos =
            SOPC_PubFixedBuffer_Get_DataSetField_Position_At(preencode, i);
        const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetField_Get_Variant(dsfPos->dataSetField);
        status = SOPC_Buffer_SetPosition(updatedBuffer, dsfPos->position);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        status = SOPC_Variant_Write(variant, updatedBuffer, 0);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to update variant");
            updatedBuffer = NULL;
        }
    }
    if (SOPC_STATUS_OK == status && 0 != preencode->signPosition && NULL != security)
    {
        if (NULL == security->groupKeys)
        {
            status = SOPC_STATUS_NOK;
            updatedBuffer = NULL;
        }
        else
        {
            SOPC_Buffer_SetPosition(updatedBuffer, preencode->signPosition);
            status = SOPC_PubSub_Security_Sign(security, updatedBuffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to sign updated buffer");
                updatedBuffer = NULL;
            }
        }
    }
    return updatedBuffer;
}
