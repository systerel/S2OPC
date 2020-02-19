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

#include <assert.h>
#include <stdio.h>

#include "sopc_encoder.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pubsub_constants.h"

/**
 * For next versions:
 *  - Replaced constantes by variables from configuration,
 *  - Add code to manage disabled part of the Network Message
 *  - Re- allocate memory of the returned buffer if needed
 *  - Manage other type of publisher id type. Only uint32_t is managed
 */

/**
 * Constantes definition for Hard-Coded value.
 * It defines the part of the Network Message which are not managed.
 * Warning: these constant cannot be changed without added new code
 */

const bool DATASET_LL_PUBLISHER_ID_ENABLED = true;
const bool DATASET_LL_GROUP_HEADER_ENABLED = true;
const bool DATASET_LL_PAYLOAD_HEADER_ENABLED = true;
const bool DATASET_LL_DATASET_CLASSID_ENABLED = false;
const bool DATASET_LL_SECURITY_ENABLED = false;
const bool DATASET_LL_TIMESTAMP_ENABLED = false;
const bool DATASET_LL_PICOSECONDS_ENABLED = false;
const bool DATASET_LL_EXTENDED_FLAGS2_ENABLED = false;

const bool DATASET_LL_WRITER_GROUP_ID_ENABLED = true;
const bool DATASET_LL_WRITER_GROUP_VERSION_ENABLED = true;
const bool DATASET_LL_NETWORK_MESSAGE_NUMBER_ENABLED = false;
const bool DATASET_LL_SEQUENCE_NUMBER_ENABLED = false;
// Indicates that only one DataSetMessage is sent in the NetworkMessage
// the size of DataSetMessage in the NetworkMessage is not managed
const bool DATASET_LL_MANAGE_ONLY_ONE_DSM = true;

const uint8_t DATASET_LL_DSM_ENCODING_TYPE = 0;
const bool DATASET_LL_DSM_IS_VALID = true;
const bool DATASET_LL_DSM_SEQ_NUMBER_ENABLED = false;
const bool DATASET_LL_DSM_STATUS_ENABLED = false;
const bool DATASET_LL_DSM_MAJOR_VERSION_ENABLED = false;
const bool DATASET_LL_DSM_MINOR_VERSION_ENABLED = false;
// DataSet Flags 2 is not managed => UADP DataSetMessage type is Data Key Frame
const bool DATASET_LL_DSM_FLAGS2_ENABLED = false;

// END Constantes definition for Hard-Coded value

/**
 * Mask to set value at bit position
 */
const uint8_t C_NETWORK_MESSAGE_BIT_0 = 1;
const uint8_t C_NETWORK_MESSAGE_BIT_1 = 2;
const uint8_t C_NETWORK_MESSAGE_BIT_2 = 4;
const uint8_t C_NETWORK_MESSAGE_BIT_3 = 8;
const uint8_t C_NETWORK_MESSAGE_BIT_4 = 16;
const uint8_t C_NETWORK_MESSAGE_BIT_5 = 32;
const uint8_t C_NETWORK_MESSAGE_BIT_6 = 64;
const uint8_t C_NETWORK_MESSAGE_BIT_7 = 128;

const uint8_t C_NETWORK_MESSAGE_COMP_BIT_0 = 255 - 1;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_1 = 255 - 2;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_2 = 255 - 4;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_3 = 255 - 8;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_4 = 255 - 16;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_5 = 255 - 32;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_6 = 255 - 64;
const uint8_t C_NETWORK_MESSAGE_COMP_BIT_7 = 255 - 128;

/**
 * Set the value of a bit
 * byte is the variable to set. Type is uint8_t*
 * bit is the position of the bit to set. Type is uint8_t
 * b is the new value of the given bit. Type is bool
 */
#define Network_Message_Set_Bool_Bit(byte, bit, b)     \
    {                                                  \
        if (b)                                         \
        {                                              \
            *byte |= C_NETWORK_MESSAGE_BIT_##bit;      \
        }                                              \
        else                                           \
        {                                              \
            *byte &= C_NETWORK_MESSAGE_COMP_BIT_##bit; \
        }                                              \
    }

/**
 * Get the value of a bit
 * byte is the variable to read. Type is SOPC_Byte
 * bit is the position of the bit. Type is SOPC_Byte
 */
#define Network_Message_Get_Bool_Bit(byte, bit) ((byte & (C_NETWORK_MESSAGE_BIT_##bit)) ? true : false)

/**
 * Private
 * Network Message are initialised. DataMessage Array of the Network Message is not allocated.
 * Configuration fields are initialised to false.
 */
static SOPC_UADP_NetworkMessage* SOPC_Network_Message_Create(void);

/**
 * Private
 * precondition: dsm is not null neither its dataset fields
 */
static SOPC_ReturnStatus Network_DataSetFields_To_UADP(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm);

/**
 * Private
 * Read Length and Data in buffer and fill DataSetFields array of given DataSetMessage
 */
static SOPC_ReturnStatus UADP_To_DataSetFields(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm);

/**
 * Private
 * Write a publisher id.
 */
static SOPC_ReturnStatus Network_Layer_PublisherId_Write(SOPC_Buffer* buffer, SOPC_Dataset_LL_PublisherId* pub_id);

/**
 * Private
 * Read a publisher id in the given buffer.
 * The size of byte to read depends of pub_id_type.
 * The readed data is set in the given NetworkMessage
 */
static SOPC_ReturnStatus Network_Layer_PublisherId_Read(SOPC_Buffer* buffer,
                                                        SOPC_Byte pub_id_type,
                                                        SOPC_Dataset_LL_NetworkMessage* nm);

/**
 * Private
 */

static bool Network_Layer_Is_Flags1_Enabled(SOPC_Dataset_LL_NetworkMessage* nm);

static SOPC_UADP_NetworkMessage* SOPC_Network_Message_Create(void)
{
    SOPC_UADP_NetworkMessage* result = SOPC_Calloc(1, sizeof(SOPC_UADP_NetworkMessage));
    result->nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    return result;
}

static SOPC_ReturnStatus Network_DataSetFields_To_UADP(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm)
{
    uint16_t length = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

    SOPC_ReturnStatus status = SOPC_UInt16_Write(&length, buffer);
    for (uint16_t i = 0; i < length && SOPC_STATUS_OK == status; i++)
    {
        const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, i);
        status = SOPC_Variant_Write(variant, buffer);
    }
    return status;
}

static SOPC_ReturnStatus UADP_To_DataSetFields(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm)
{
    uint16_t length = 0;
    SOPC_ReturnStatus status;
    bool allocStatus;

    status = SOPC_UInt16_Read(&length, buffer);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    allocStatus = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(dsm, length);
    if (!allocStatus)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (uint16_t i = 0; i < length && SOPC_STATUS_OK == status; i++)
    {
        SOPC_Variant* variant = SOPC_Variant_Create();
        if (NULL == variant)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, i);
            assert(res); // valid index
            status = SOPC_Variant_Read(variant, buffer);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Dataset_LL_DataSetMsg_Delete_DataSetField_Array(dsm);
    }

    return status;
}

static SOPC_ReturnStatus Network_Layer_PublisherId_Write(SOPC_Buffer* buffer, SOPC_Dataset_LL_PublisherId* pub_id)
{
    assert(NULL != buffer && NULL != pub_id);
    // String Publisher Id is not managed
    assert(DataSet_LL_PubId_String_Id != pub_id->type);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    switch (pub_id->type)
    {
    case DataSet_LL_PubId_Byte_Id:
        status = SOPC_Byte_Write(&(pub_id->data.byte), buffer);
        break;
    case DataSet_LL_PubId_UInt16_Id:
        status = SOPC_UInt16_Write(&(pub_id->data.uint16), buffer);
        break;
    case DataSet_LL_PubId_UInt32_Id:
        status = SOPC_UInt32_Write(&(pub_id->data.uint32), buffer);
        break;
    case DataSet_LL_PubId_UInt64_Id:
        status = SOPC_UInt64_Write(&(pub_id->data.uint64), buffer);
        break;
    case DataSet_LL_PubId_String_Id:
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

static SOPC_ReturnStatus Network_Layer_PublisherId_Read(SOPC_Buffer* buffer,
                                                        SOPC_Byte pub_id_type,
                                                        SOPC_Dataset_LL_NetworkMessage* nm)
{
    assert(NULL != buffer && NULL != nm);
    // String Publisher Id is not managed
    assert(DataSet_LL_PubId_String_Id != pub_id_type);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    switch (pub_id_type)
    {
    case DataSet_LL_PubId_Byte_Id:
    {
        SOPC_Byte id;
        status = SOPC_Byte_Read(&id, buffer);
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nm, id);
        break;
    }
    case DataSet_LL_PubId_UInt16_Id:
    {
        uint16_t id;
        status = SOPC_UInt16_Read(&id, buffer);
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(nm, id);
        break;
    }
    case DataSet_LL_PubId_UInt32_Id:
    {
        uint32_t id;
        status = SOPC_UInt32_Read(&id, buffer);
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(nm, id);
        break;
    }
    case DataSet_LL_PubId_UInt64_Id:
    {
        uint64_t id;
        status = SOPC_UInt64_Read(&id, buffer);
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(nm, id);
        break;
    }
    case DataSet_LL_PubId_String_Id:
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

static bool Network_Layer_Is_Flags1_Enabled(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_Dataset_LL_PublisherId* pub_id = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm);
    return (DataSet_LL_PubId_Byte_Id != pub_id->type || DATASET_LL_DATASET_CLASSID_ENABLED ||
            DATASET_LL_SECURITY_ENABLED || DATASET_LL_TIMESTAMP_ENABLED || DATASET_LL_PICOSECONDS_ENABLED);
}

SOPC_Buffer* SOPC_UADP_NetworkMessage_Encode(SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
    uint8_t byte = 0;
    bool flags1_enabled;

    // UADP version bit 0-3
    byte = SOPC_Dataset_LL_NetworkMessage_Get_Version(nm);
    // UADP flags bit 4-7
    //  - PublisherId enabled
    Network_Message_Set_Bool_Bit(&byte, 4, DATASET_LL_PUBLISHER_ID_ENABLED);
    //  - GroupHeader enabled
    Network_Message_Set_Bool_Bit(&byte, 5, DATASET_LL_GROUP_HEADER_ENABLED);
    //  - PayloadHeader enabled
    Network_Message_Set_Bool_Bit(&byte, 6, DATASET_LL_PAYLOAD_HEADER_ENABLED);
    //  - ExtendedFlags1 enabled
    flags1_enabled = Network_Layer_Is_Flags1_Enabled(nm);
    Network_Message_Set_Bool_Bit(&byte, 7, flags1_enabled);
    status = SOPC_Buffer_Write(buffer, &byte, 1);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(buffer);
        return NULL;
    }

    if (flags1_enabled)
    {
        // Bit range 0-2: PublisherId Type
        byte = (uint8_t) SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm)->type;
        Network_Message_Set_Bool_Bit(&byte, 3, DATASET_LL_DATASET_CLASSID_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 4, DATASET_LL_SECURITY_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 5, DATASET_LL_TIMESTAMP_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 6, DATASET_LL_PICOSECONDS_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 7, DATASET_LL_EXTENDED_FLAGS2_ENABLED);

        status = SOPC_Buffer_Write(buffer, &byte, 1);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }
    }

    if (DATASET_LL_PUBLISHER_ID_ENABLED)
    {
        status = Network_Layer_PublisherId_Write(buffer, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm));
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }
    }

    // GroupHeader
    //  - set reserved bits to 0
    byte = 0;
    //  - WriterGroupId enabled
    Network_Message_Set_Bool_Bit(&byte, 0, DATASET_LL_WRITER_GROUP_ID_ENABLED);
    //  - WriterGroupVersion enabled
    Network_Message_Set_Bool_Bit(&byte, 1, DATASET_LL_WRITER_GROUP_VERSION_ENABLED);
    //  - NetworkMessageNumber enabled
    Network_Message_Set_Bool_Bit(&byte, 2, DATASET_LL_NETWORK_MESSAGE_NUMBER_ENABLED);
    //  - SequenceNumber enabled
    Network_Message_Set_Bool_Bit(&byte, 3, DATASET_LL_SEQUENCE_NUMBER_ENABLED);
    status = SOPC_Buffer_Write(buffer, &byte, 1);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(buffer);
        return NULL;
    }

    if (DATASET_LL_WRITER_GROUP_ID_ENABLED)
    {
        uint16_t byte_2 = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm);
        status = SOPC_UInt16_Write(&byte_2, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }
    }

    if (DATASET_LL_WRITER_GROUP_VERSION_ENABLED)
    {
        uint32_t version = SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm);
        status = SOPC_UInt32_Write(&version, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }
    }

    // payload header
    uint8_t msg_count = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    if (DATASET_LL_MANAGE_ONLY_ONE_DSM && msg_count > 1)
    {
        SOPC_Buffer_Delete(buffer);
        return NULL;
    }

    status = SOPC_Buffer_Write(buffer, (uint8_t*) &msg_count, 1);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(buffer);
        return NULL;
    }

    for (int i = 0; i < msg_count; i++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
        // - writer id
        uint16_t byte_2 = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm);
        status = SOPC_UInt16_Write(&byte_2, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }

        if (DATASET_LL_MANAGE_ONLY_ONE_DSM)
        {
            break;
        }
    }

    // payload
    for (int i = 0; i < msg_count; i++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);

        // DataSetMessage (1 byte)

        // - DataSet Flags 1
        //   - FieldEncoding is variant
        byte = DATASET_LL_DSM_ENCODING_TYPE;
        //   - DataSetMessage isValid = true
        Network_Message_Set_Bool_Bit(&byte, 0, DATASET_LL_DSM_IS_VALID);
        //   - sequence number is disabled
        Network_Message_Set_Bool_Bit(&byte, 3, DATASET_LL_DSM_SEQ_NUMBER_ENABLED);
        //   - status is disabled
        Network_Message_Set_Bool_Bit(&byte, 4, DATASET_LL_DSM_STATUS_ENABLED);
        //   - major version is disabled
        Network_Message_Set_Bool_Bit(&byte, 5, DATASET_LL_DSM_MAJOR_VERSION_ENABLED);
        //   - minor version is disabled
        Network_Message_Set_Bool_Bit(&byte, 6, DATASET_LL_DSM_MINOR_VERSION_ENABLED);
        //   - extended flags 2 is disabled
        Network_Message_Set_Bool_Bit(&byte, 7, DATASET_LL_DSM_FLAGS2_ENABLED);
        status = SOPC_Buffer_Write(buffer, (uint8_t*) &byte, 1);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }

        status = Network_DataSetFields_To_UADP(buffer, dsm);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Buffer_Delete(buffer);
            return NULL;
        }

        if (DATASET_LL_MANAGE_ONLY_ONE_DSM)
        {
            break;
        }
    }

    status = SOPC_Buffer_SetPosition(buffer, 0);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(buffer);
        return NULL;
    }

    return buffer;
}

SOPC_UADP_NetworkMessage* SOPC_UADP_NetworkMessage_Decode(SOPC_Buffer* buffer)
{
    SOPC_ReturnStatus status;
    SOPC_UADP_NetworkMessage* uadp_nm = SOPC_Network_Message_Create();
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_UADP_Configuration* conf = &uadp_nm->conf;
    SOPC_Boolean flags1_enabled = false;
    SOPC_Boolean flags2_enabled = false;
    // Publisher Id Type. Read in Extended flags1 or Byte if no flags1
    SOPC_Byte pub_id_type = DataSet_LL_PubId_Byte_Id;
    // number of DataSetMessage. Should be one
    SOPC_Byte msg_count = 0;

    // Version and Flags
    //  Bit range 0-3: Version of the UADP NetworkMessage
    //  Bit 4: PublisherId enabled
    //  Bit 5: GroupHeader enabled
    //  Bit 6: PayloadHeader enabled
    //  Bit 7: ExtendedFlags1 enabled
    {
        SOPC_Byte data;
        SOPC_Byte version;
        status = SOPC_Byte_Read(&data, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        version = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_4 - 1);
        SOPC_Dataset_LL_NetworkMessage_SetVersion(nm, version);
        conf->PublisherIdFlag = Network_Message_Get_Bool_Bit(data, 4);
        conf->GroupHeaderFlag = Network_Message_Get_Bool_Bit(data, 5);
        conf->PayloadHeaderFlag = Network_Message_Get_Bool_Bit(data, 6);
        flags1_enabled = Network_Message_Get_Bool_Bit(data, 7);
    }

    // Bit range 0-2: PublisherId Type
    // Bit 3: DataSetClassId enabled
    // Bit 4: Security enabled
    // Bit 5: Timestamp enabled
    // Bit 6: PicoSeconds enabled
    // Bit 7: ExtendedFlags2 enabled
    if (flags1_enabled)
    {
        SOPC_Byte data;
        status = SOPC_Byte_Read(&data, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        pub_id_type = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_3 - 1);
        // other flags are not managed
        if (data - pub_id_type)
        {
            // not managed yet
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        conf->DataSetClassIdFlag = Network_Message_Get_Bool_Bit(data, 3);
        conf->SecurityFlag = Network_Message_Get_Bool_Bit(data, 4);
        conf->TimestampFlag = Network_Message_Get_Bool_Bit(data, 5);
        conf->PicoSecondsFlag = Network_Message_Get_Bool_Bit(data, 6);
        flags2_enabled = Network_Message_Get_Bool_Bit(data, 7);
    }
    else
    {
        conf->DataSetClassIdFlag = false;
        conf->SecurityFlag = false;
        conf->TimestampFlag = false;
        conf->PicoSecondsFlag = false;
        flags2_enabled = false;
    }

    // Bit 0: Chunk message
    // Bit 1: PromotedFields enabled
    // Bit range 2-4: UADP NetworkMessage type
    // Others: not used
    if (flags2_enabled)
    {
        // not managed yet
        // starting here, we assume that flags2_enabled is false.
        // removing this code implies to change payload and payload header management
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }
    else
    {
        conf->PromotedFieldsFlag = false;
    }

    if (conf->PublisherIdFlag)
    {
        status = Network_Layer_PublisherId_Read(buffer, pub_id_type, nm);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
    }

    if (conf->DataSetClassIdFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    // Group Header
    if (conf->GroupHeaderFlag)
    {
        // Bit 0: WriterGroupId enabled
        // Bit 1: GroupVersion enabled
        // Bit 2: NetworkMessageNumber enabled
        // Bit 3: SequenceNumber enabled
        // Others: not used
        {
            SOPC_Byte data;
            status = SOPC_Byte_Read(&data, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            conf->GroupIdFlag = Network_Message_Get_Bool_Bit(data, 0);
            conf->GroupVersionFlag = Network_Message_Get_Bool_Bit(data, 1);
            conf->NetworkMessageNumberFlag = Network_Message_Get_Bool_Bit(data, 2);
            conf->SequenceNumberFlag = Network_Message_Get_Bool_Bit(data, 3);
        }

        if (conf->GroupIdFlag)
        {
            uint16_t group_id;
            status = SOPC_UInt16_Read(&group_id, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, group_id);
        }

        if (conf->GroupVersionFlag)
        {
            uint32_t group_version;
            status = SOPC_UInt32_Read(&group_version, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, group_version);
        }
        if (conf->NetworkMessageNumberFlag)
        {
            // not managed yet
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        if (conf->SequenceNumberFlag)
        {
            // not managed yet
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
    }

    // Payload Header
    // Only DataSetMessage is managed
    if (conf->PayloadHeaderFlag)
    {
        uint16_t writer_id;
        status = SOPC_Byte_Read(&msg_count, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }

        // Only one DataSetMessage is managed
        if (1 != msg_count)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }

        bool allocStatus = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, msg_count);
        if (!allocStatus)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

        status = SOPC_UInt16_Read(&writer_id, buffer);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
        SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, writer_id);
    }
    else
    {
        // If there is no Payload header, NetworkMessage contains one DataSetMessage

        msg_count = 1;

        bool allocStatus = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, 1);
        if (!allocStatus)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }
    }

    // Timestamp
    if (conf->TimestampFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    // Picoseconds
    if (conf->PicoSecondsFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    // Picoseconds
    if (conf->PromotedFieldsFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    // Security Header
    if (conf->SecurityFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    // Payload
    // Only DataSetMessage is managed and only one
    assert(1 == msg_count);
    // No size if there is only one DataSetMessage

    // Bit 0: DataSetMessage is valid.
    // Bit range 1-2: Field Encoding
    // Bit 3: DataSetMessageSequenceNumber enabled
    // Bit 4: Status enabled
    // Bit 5: ConfigurationVersionMajorVersion enabled
    // Bit 6: ConfigurationVersionMinorVersion enable
    // Bit 7: DataSetFlags2 enabled
    for (int i = 0; i < msg_count; i++)
    {
        SOPC_Byte data;
        SOPC_Boolean seq_nb_enabled = false;
        SOPC_Boolean status_enabled = false;
        SOPC_Boolean maj_version_enabled = false;
        SOPC_Boolean min_version_enabled = false;
        SOPC_Boolean dsm_flags2 = false;
        SOPC_Boolean timestamp_enabled = false;
        SOPC_Boolean picoseconds_enabled = false;
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);

        /* DataSetMessages Header */

        /** DataSetFlags1 **/
        {
            status = SOPC_Byte_Read(&data, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            SOPC_Boolean isValid = Network_Message_Get_Bool_Bit(data, 0);
            if (!isValid)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }

            uint8_t field_encoding = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_1 + C_NETWORK_MESSAGE_BIT_2);
            if (DATASET_LL_DSM_ENCODING_TYPE != field_encoding)
            {
                // not managed yet
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            seq_nb_enabled = Network_Message_Get_Bool_Bit(data, 3);
            status_enabled = Network_Message_Get_Bool_Bit(data, 4);
            maj_version_enabled = Network_Message_Get_Bool_Bit(data, 5);
            min_version_enabled = Network_Message_Get_Bool_Bit(data, 6);
            dsm_flags2 = Network_Message_Get_Bool_Bit(data, 7);
            timestamp_enabled = false;
            picoseconds_enabled = false;
        }

        /** DataSetFlags2 **/
        if (dsm_flags2)
        {
            // Bit range 0-3: UADP DataSetMessage type
            // Bit 4: Timestamp enabled
            // Bit 5: PicoSeconds enabled
            status = SOPC_Byte_Read(&data, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            uint8_t dataSmessage_type = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_4 - 1);
            if (0 != dataSmessage_type)
            {
                // not managed yet
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
            timestamp_enabled = Network_Message_Get_Bool_Bit(data, 4);
            picoseconds_enabled = Network_Message_Get_Bool_Bit(data, 5);
        }

        /** DataSetMessage SequenceNumber **/
        if (seq_nb_enabled)
        {
            // not managed yet
            uint16_t notUsed;
            status = SOPC_UInt16_Read(&notUsed, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /** Timestamp **/
        if (timestamp_enabled)
        {
            // not managed yet
            uint64_t timestamp;
            status = SOPC_UInt64_Read(&timestamp, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /** PicoSeconds **/
        if (picoseconds_enabled)
        {
            // not managed yet
            uint16_t notUsed;
            status = SOPC_UInt16_Read(&notUsed, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /** Status **/
        if (status_enabled)
        {
            // not managed yet
            uint16_t notUsed;
            status = SOPC_UInt16_Read(&notUsed, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /** ConfigurationVersion MajorVersion **/
        if (maj_version_enabled)
        {
            // not managed yet
            uint32_t not_used;
            status = SOPC_UInt32_Read(&not_used, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /** ConfigurationVersion MinorVersion **/
        if (min_version_enabled)
        {
            // not managed yet
            uint32_t not_used;
            status = SOPC_UInt32_Read(&not_used, buffer);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
                SOPC_Free(uadp_nm);
                return NULL;
            }
        }

        /* Data Key Frame DataSetMessage Data */
        status = UADP_To_DataSetFields(buffer, dsm);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
            SOPC_Free(uadp_nm);
            return NULL;
        }

        if (DATASET_LL_MANAGE_ONLY_ONE_DSM)
        {
            break;
        }
    }

    // Security Footer
    if (conf->SecurityFlag)
    {
        // not managed yet
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
        return NULL;
    }

    return uadp_nm;
}

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm)
{
    if (NULL != uadp_nm)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
    }
}
