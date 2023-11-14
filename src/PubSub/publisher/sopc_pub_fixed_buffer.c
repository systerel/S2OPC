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
#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pub_fixed_buffer.h"

struct SOPC_PubFixedBuffer_DataSetField_Position
{
    // Pointer to DataSetField stored in SOPC_Dataset_LL_NetworkMessage structure
    const SOPC_Dataset_LL_DataSetField* dataSetField;
    // Position in preencoded buffer
    uint32_t position;
};

struct SOPC_PubFixedBuffer_Buffer_ctx
{
    // Array of pointers to dsm_sequence_number stored in SOPC_Dataset_LL_NetworkMessage structure
    uint16_t** dsm_sequence_numbers;
    uint32_t* dsm_sequence_number_positions;
    size_t dsm_sequence_numbers_len;

    // Array of tuple pointer to dataSetField stored in SOPC_Dataset_LL_NetworkMessage structure and position in
    // preencoded buffer
    SOPC_PubFixedBuffer_DataSetField_Position* dataSetFields;
    size_t dataSetFields_len;

    // Preencoded network message
    SOPC_Buffer* buffer;
};

static SOPC_PubFixedBuffer_Buffer_ctx* PubFixedBuffer_Create_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_PubFixedBuffer_Buffer_ctx* preencode_structure = SOPC_Malloc(sizeof(SOPC_PubFixedBuffer_Buffer_ctx));
    memset(preencode_structure, 0, sizeof(SOPC_PubFixedBuffer_Buffer_ctx));

    SOPC_ASSERT(NULL != preencode_structure);

    uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    preencode_structure->dsm_sequence_numbers_len = (size_t) nbDsm;
    preencode_structure->dsm_sequence_numbers =
        SOPC_Calloc(preencode_structure->dsm_sequence_numbers_len, sizeof(uint16_t*));
    preencode_structure->dsm_sequence_number_positions =
        SOPC_Calloc(preencode_structure->dsm_sequence_numbers_len, sizeof(uint32_t));

    SOPC_ASSERT(NULL != preencode_structure->dsm_sequence_numbers);
    SOPC_ASSERT(NULL != preencode_structure->dsm_sequence_number_positions);

    for (uint8_t iDsm = 0; iDsm < nbDsm; iDsm++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, iDsm);
        preencode_structure->dataSetFields_len += SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
    }

    preencode_structure->dataSetFields =
        SOPC_Calloc(preencode_structure->dataSetFields_len, sizeof(SOPC_PubFixedBuffer_DataSetField_Position));
    SOPC_ASSERT(NULL != preencode_structure->dataSetFields);

    preencode_structure->buffer = NULL;

    return preencode_structure;
}

SOPC_PubFixedBuffer_DataSetField_Position* SOPC_PubFixedBuffer_Get_DataSetField_Position_At(
    SOPC_PubFixedBuffer_Buffer_ctx* preencode,
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
    SOPC_PubFixedBuffer_Buffer_ctx* preencode_structure = SOPC_DataSet_LL_NetworkMessage_Get_Preencode_Buffer(nm);

    uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    SOPC_ASSERT(preencode_structure->dsm_sequence_numbers_len == nbDsm);
    size_t index_dsf = 0;
    for (int i = 0; i < nbDsm; i++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *(preencode_structure->dsm_sequence_numbers + i) =
            (uint16_t*) SOPC_Dataset_LL_DataSetMsg_Get_PointerSequenceNumber(dsm);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        uint16_t nbDsf = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        SOPC_ASSERT(preencode_structure->dataSetFields_len >= nbDsf + index_dsf);
        for (uint16_t j = 0; j < nbDsf; j++)
        {
            const SOPC_Dataset_LL_DataSetField* dsf = SOPC_Dataset_LL_DataSetMsg_Get_DataSetField_At(dsm, j);
            SOPC_ASSERT(NULL != dsf);
            SOPC_PubFixedBuffer_DataSetField_Position* dsfPosition =
                SOPC_PubFixedBuffer_Get_DataSetField_Position_At(preencode_structure, index_dsf);
            SOPC_ASSERT(NULL != dsfPosition);
            dsfPosition->dataSetField = dsf;
            index_dsf++;
        }
    }
}

SOPC_ReturnStatus SOPC_DataSet_LL_NetworkMessage_Create_Preencode_Buffer_ctx(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PubFixedBuffer_Buffer_ctx* preencode = PubFixedBuffer_Create_Preencode_Buffer(nm);
    SOPC_DataSet_LL_NetworkMessage_Set_Preencode_Buffer(nm, preencode);
    if (NULL == preencode)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        PubFixedBuffer_Initialize_Preencode_Buffer(nm);
        SOPC_Buffer* buffer_payload = NULL;
        SOPC_UADP_NetworkMessage_Error_Code code =
            SOPC_UADP_NetworkMessage_Encode_Buffers(nm, NULL, &preencode->buffer, &buffer_payload);
        if (SOPC_UADP_NetworkMessage_Error_Code_None != code || NULL == preencode->buffer || NULL == buffer_payload)
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Failed to preencode PUB message, SOPC_UADP_NetworkMessage_Error_Code is : %08X",
                                   (unsigned) code);
        }
        else
        {
            code = SOPC_UADP_NetworkMessage_SignAndEncrypt(NULL, preencode->buffer, &buffer_payload);
            if (SOPC_UADP_NetworkMessage_Error_Code_None != code || NULL == preencode->buffer || NULL != buffer_payload)
            {
                status = SOPC_STATUS_NOK;
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_PUBSUB,
                    "Failed to merge preencode buffer, SOPC_UADP_NetworkMessage_Error_Code is : %08X", (unsigned) code);
            }
        }
    }
    return status;
}

void SOPC_PubFixedBuffer_Delete_Preencode_Buffer_ctx(SOPC_PubFixedBuffer_Buffer_ctx* preencode)
{
    if (NULL != preencode)
    {
        SOPC_Free(preencode->dataSetFields);
        preencode->dataSetFields = NULL;
        SOPC_Free(preencode->dsm_sequence_numbers);
        preencode->dsm_sequence_numbers = NULL;
        SOPC_Free(preencode->dsm_sequence_number_positions);
        preencode->dsm_sequence_number_positions = NULL;
        SOPC_Buffer_Delete(preencode->buffer);
        preencode->buffer = NULL;
        SOPC_Free(preencode);
        preencode = NULL;
    }
}

bool SOPC_PubFixedBuffer_Set_DSM_SequenceNumber_Position_At(SOPC_PubFixedBuffer_Buffer_ctx* preencode,
                                                            uint32_t position,
                                                            size_t index)
{
    if (NULL == preencode || index >= preencode->dsm_sequence_numbers_len)
    {
        return false;
    }
    memcpy(preencode->dsm_sequence_number_positions + index, &position, sizeof(position));
    return true;
}

void SOPC_PubFixedBuffer_DataSetFieldPosition_Set_Position(SOPC_PubFixedBuffer_DataSetField_Position* dsfPos,
                                                           uint32_t position)
{
    SOPC_ASSERT(NULL != dsfPos);
    dsfPos->position = position;
}

/** Update value in buffer at position gived with data. Length correspond to the number of bytes to update */
static void update_buffer(SOPC_Buffer* buffer, void* data, uint32_t position, uint32_t length)
{
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(NULL != data);
    SOPC_Buffer_SetPosition(buffer, position);
    SOPC_Buffer_Write(buffer, (uint8_t*) data, length);
}

SOPC_Buffer* SOPC_PubFixedBuffer_Get_UpdatedBuffers(SOPC_PubFixedBuffer_Buffer_ctx* preencode)
{
    SOPC_ASSERT(NULL != preencode);
    for (size_t i = 0; i < preencode->dsm_sequence_numbers_len; i++)
    {
        SOPC_ASSERT(NULL != (preencode->dsm_sequence_numbers + i));
        SOPC_ASSERT(NULL != (preencode->dsm_sequence_number_positions + i));
        update_buffer(preencode->buffer, *(preencode->dsm_sequence_numbers + i),
                      *(preencode->dsm_sequence_number_positions + i), sizeof(**preencode->dsm_sequence_numbers));
    }
    for (size_t i = 0; i < preencode->dataSetFields_len; i++)
    {
        SOPC_PubFixedBuffer_DataSetField_Position* dsfPos =
            SOPC_PubFixedBuffer_Get_DataSetField_Position_At(preencode, i);
        const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetField_Get_Variant(dsfPos->dataSetField);
        SOPC_Buffer_SetPosition(preencode->buffer, dsfPos->position);
        SOPC_Variant_Write(variant, preencode->buffer, 0);
    }
    return preencode->buffer;
}
