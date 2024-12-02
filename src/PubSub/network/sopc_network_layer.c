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

// TODO : use conf rather than constants to encode message

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pub_fixed_buffer.h"
#include "sopc_version.h"

/**
 * For next versions:
 *  - Replace constants by variables from configuration,
 *  - Add code to manage disabled part of the Network Message
 *  - Re- allocate memory of the returned buffer if needed
 *  - Manage other type of publisher id type. Only uint32_t is managed
 */

/* Convert a PublisherId from dataset module to one of Configuration module */
static SOPC_Conf_PublisherId Network_Layer_Convert_PublisherId(const SOPC_Dataset_LL_PublisherId* src);

/*
 * Check if security mode as enum is equals to security mode as boolean
 */
static bool Network_Check_ReceivedSecurityMode(SOPC_SecurityMode_Type mode, bool ssigned, bool encrypted);

/** Extract the message header
 * \param buffer The Buffer to decode
 * \param buffer The header to header to fill with decoded buffer
 * \return SOPC_NetworkMessage_Error_Code_None if header is correct.*/
static inline SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessageHeader_Decode(
    SOPC_Buffer* buffer,
    SOPC_Dataset_LL_NetworkMessage_Header* header);

/**
 * Decode a network message (V1 format)
 */
static inline SOPC_NetworkMessage_Error_Code Decode_Message_V1(
    SOPC_Buffer* buffer,
    uint32_t payload_sign_position,
    SOPC_Dataset_LL_NetworkMessage* nm,
    SOPC_Dataset_LL_NetworkMessage_Header* header,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* readerConf,
    const SOPC_ReaderGroup* group);

/**
 * Decodes the group header
 */
static inline SOPC_NetworkMessage_Error_Code Decode_GroupHeader(SOPC_Buffer* buffer,
                                                                SOPC_Dataset_LL_NetworkMessage* nm,
                                                                SOPC_Dataset_LL_NetworkMessage_Header* header,
                                                                SOPC_UADP_Configuration* conf);

/**
 * Decodes the security header
 */
static inline SOPC_NetworkMessage_Error_Code Decode_SecurityHeader(SOPC_Buffer* buffer,
                                                                   SOPC_Buffer** buffer_payload,
                                                                   uint32_t payload_position,
                                                                   uint16_t group_id,
                                                                   SOPC_UADP_GetSecurity_Func getSecurity_Func,
                                                                   SOPC_Dataset_LL_NetworkMessage_Header* header,
                                                                   SOPC_UADP_Configuration* conf);

/**
 * Constants definition for Hard-Coded value.
 * It defines the part of the Network Message which are not managed.
 * Warning: these constant cannot be changed without added new code
 */

const bool DATASET_LL_FLAGS1_ENABLED = true;
const bool DATASET_LL_PUBLISHER_ID_ENABLED = true;
const bool DATASET_LL_GROUP_FLAGS_ENABLED = true;
const bool DATASET_LL_GROUP_HEADER_ENABLED = true;
const bool DATASET_LL_PAYLOAD_HEADER_ENABLED = true;
const bool DATASET_LL_DATASET_CLASSID_ENABLED = false;
const bool DATASET_LL_TIMESTAMP_ENABLED = false;
const bool DATASET_LL_PICOSECONDS_ENABLED = false;
const bool DATASET_LL_EXTENDED_FLAGS2_ENABLED = false;

const bool DATASET_LL_WRITER_GROUP_ID_ENABLED = true;
const bool DATASET_LL_WRITER_GROUP_VERSION_ENABLED = true;
const bool DATASET_LL_NETWORK_MESSAGE_NUMBER_ENABLED = false;
const bool DATASET_LL_SEQUENCE_NUMBER_ENABLED = false;

const bool DATASET_LL_DSM_IS_VALID = true;
const bool DATASET_LL_DSM_STATUS_ENABLED = false;
const bool DATASET_LL_DSM_MAJOR_VERSION_ENABLED = false;
const bool DATASET_LL_DSM_MINOR_VERSION_ENABLED = false;
const bool DATASET_LL_DSM_PICOSECONDS_ENABLED = false;

// Security Header Flag
const bool DATASET_LL_SECURITY_SIGNED_ENABLED = true;
const bool DATASET_LL_SECURITY_ENCRYPTED_ENABLED = true;
const bool DATASET_LL_SECURITY_FOOTER_ENABLED = false;
const bool DATASET_LL_SECURITY_KEY_RESET_ENABLED = false;

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
 * If status it's OK
 * Write value into buffer(SOPC_Buffer) with SOPC_Buffer_Write function
 */
#define BUFFER_PRINT_STR(value, buffer, status)                                                   \
    do                                                                                            \
    {                                                                                             \
        if (SOPC_STATUS_OK == status)                                                             \
        {                                                                                         \
            status = SOPC_Buffer_Write(buffer, (const uint8_t*) value, (uint32_t) strlen(value)); \
        }                                                                                         \
    } while (0)

/*
 * If status is not equal to SOPC_STATUS_OK return code otherwise return value
 * SOPC_NetworkMessage_Error_Code_None
 */
static inline SOPC_NetworkMessage_Error_Code checkAndGetErrorCode(const SOPC_ReturnStatus status,
                                                                  const SOPC_NetworkMessage_Error_Code code)
{
    return ((SOPC_STATUS_OK == status) ? SOPC_NetworkMessage_Error_Code_None : code);
}

static inline SOPC_ReturnStatus valid_bool_to_status(const bool b)
{
    return (b ? SOPC_STATUS_OK : SOPC_STATUS_ENCODING_ERROR);
}

static inline void set_status_default(SOPC_ReturnStatus* const status,
                                      SOPC_NetworkMessage_Error_Code* const res,
                                      const SOPC_NetworkMessage_Error_Code code)
{
    *status = SOPC_STATUS_ENCODING_ERROR;
    *res = code;
}

/**
 * Private
 * Network Message are initialised. DataMessage Array of the Network Message is not allocated.
 * Configuration fields are initialised to false.
 */
static SOPC_UADP_NetworkMessage* SOPC_Network_Message_Create(void);

/**
 * Private
 * precondition: dsm is not null neither its dataset fields
 * dataSetField_position give a list of all positions in the buffer of each dataSetField if not NULL
 */
static SOPC_ReturnStatus Network_DataSetFields_To_UADP(SOPC_Buffer* buffer,
                                                       SOPC_Dataset_LL_DataSetMessage* dsm,
                                                       uint32_t* dataSetField_position);

/**
 * Private
 * Read Length and Data in buffer and fill DataSetFields array of given DataSetMessage
 */
static SOPC_ReturnStatus UADP_To_DataSetFields(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm);

/**
 * Private
 * Write a publisher id.
 */
static SOPC_ReturnStatus Network_Layer_PublisherId_Write(SOPC_Buffer* buffer,
                                                         const SOPC_Dataset_LL_PublisherId* pub_id);

/**
 * Private
 * Read a publisher id in the given buffer.
 * The size of byte to read depends of pub_id_type.
 * The readed data is set in the given NetworkMessage
 */
static SOPC_ReturnStatus Network_Layer_PublisherId_Read(SOPC_Buffer* buffer,
                                                        SOPC_Byte pub_id_type,
                                                        SOPC_Dataset_LL_NetworkMessage_Header* header);

/**
 * Private
 * Return true if ExtendedFlags1 block should be enabled.
 */
static bool Network_Layer_Is_Flags1_Enabled(SOPC_Dataset_LL_NetworkMessage_Header* nmh, bool security);

/**
 * Private
 * Check if the received NetworkMessage is newer than the last processed.
 * To do this, compare the sequence number of the NetworkMessage which should be strictly monotonically increasing.
 * This function manages sequence numbers roll over (change from 4294967295 to 0).
 * See OPCUA Spec Part 14 - Table 75
 *
 * received is the sequence number in the current message
 * processed is the last processed sequence number
 */
static bool Network_Layer_Is_Sequence_Number_Newer(uint32_t received, uint32_t processed);

static SOPC_UADP_NetworkMessage* SOPC_Network_Message_Create(void)
{
    SOPC_UADP_NetworkMessage* result = SOPC_Calloc(1, sizeof(SOPC_UADP_NetworkMessage));
    SOPC_ASSERT(NULL != result);
    result->nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    return result;
}

static SOPC_ReturnStatus Network_DataSetFields_To_UADP(SOPC_Buffer* buffer,
                                                       SOPC_Dataset_LL_DataSetMessage* dsm,
                                                       uint32_t* dataSetField_position)
{
    uint16_t length = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

    SOPC_ReturnStatus status = SOPC_UInt16_Write(&length, buffer, 0);

    for (uint16_t i = 0; i < length && SOPC_STATUS_OK == status; i++)
    {
        const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(dsm, i);
        if (NULL != dataSetField_position)
        {
            status = SOPC_Buffer_GetPosition(buffer, &dataSetField_position[i]);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Write(variant, buffer, 0);
        }
    }
    return status;
}

static SOPC_ReturnStatus UADP_To_DataSetFields(SOPC_Buffer* buffer, SOPC_Dataset_LL_DataSetMessage* dsm)
{
    uint16_t length = 0;
    SOPC_ReturnStatus status;
    bool allocStatus;

    status = SOPC_UInt16_Read(&length, buffer, 0);
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
        SOPC_Variant variant;
        SOPC_Variant_Initialize(&variant);
        status = SOPC_Variant_Read(&variant, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, i);
            SOPC_ASSERT(res); // valid index
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Dataset_LL_DataSetMsg_Delete_DataSetField_Array(dsm);
    }

    return status;
}

static SOPC_ReturnStatus Network_Layer_PublisherId_Write(SOPC_Buffer* buffer, const SOPC_Dataset_LL_PublisherId* pub_id)
{
    SOPC_ASSERT(NULL != buffer && NULL != pub_id);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    switch (pub_id->type)
    {
    case DataSet_LL_PubId_Byte_Id:
        status = SOPC_Byte_Write(&(pub_id->data.byte), buffer, 0);
        break;
    case DataSet_LL_PubId_UInt16_Id:
        status = SOPC_UInt16_Write(&(pub_id->data.uint16), buffer, 0);
        break;
    case DataSet_LL_PubId_UInt32_Id:
        status = SOPC_UInt32_Write(&(pub_id->data.uint32), buffer, 0);
        break;
    case DataSet_LL_PubId_UInt64_Id:
        status = SOPC_UInt64_Write(&(pub_id->data.uint64), buffer, 0);
        break;
    case DataSet_LL_PubId_String_Id:
        status = SOPC_String_Write(&(pub_id->data.string), buffer, 0);
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

static SOPC_ReturnStatus Network_Layer_PublisherId_Read(SOPC_Buffer* buffer,
                                                        SOPC_Byte pub_id_type,
                                                        SOPC_Dataset_LL_NetworkMessage_Header* header)
{
    SOPC_ASSERT(NULL != buffer && NULL != header);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    switch (pub_id_type)
    {
    case DataSet_LL_PubId_Byte_Id:
    {
        SOPC_Byte id;
        status = SOPC_Byte_Read(&id, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, id);
        }
        break;
    }
    case DataSet_LL_PubId_UInt16_Id:
    {
        uint16_t id;
        status = SOPC_UInt16_Read(&id, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(header, id);
        }
        break;
    }
    case DataSet_LL_PubId_UInt32_Id:
    {
        uint32_t id;
        status = SOPC_UInt32_Read(&id, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(header, id);
        }
        break;
    }
    case DataSet_LL_PubId_UInt64_Id:
    {
        uint64_t id;
        status = SOPC_UInt64_Read(&id, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(header, id);
        }
        break;
    }
    case DataSet_LL_PubId_String_Id:
    {
        SOPC_String id;
        SOPC_String_Initialize(&id);
        status = SOPC_String_Read(&id, buffer, 0);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_String(header, id);
        }
        SOPC_String_Clear(&id);
        break;
    }
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

static bool Network_Layer_Is_Flags1_Enabled(SOPC_Dataset_LL_NetworkMessage_Header* nmh, bool security)
{
    const SOPC_Dataset_LL_PublisherId* pub_id = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nmh);
    return (NULL != pub_id && DataSet_LL_PubId_Byte_Id != pub_id->type) || DATASET_LL_DATASET_CLASSID_ENABLED ||
           security || DATASET_LL_TIMESTAMP_ENABLED || DATASET_LL_PICOSECONDS_ENABLED;
}

static bool Network_Layer_DataSetMessage_Is_Flags2_Enabled(SOPC_DataSet_LL_UadpDataSetMessageContentMask conf)
{
    return (DataSet_LL_MessageType_KeyFrame != conf.dataSetMessageType || conf.timestampFlag || conf.picoSecondsFlag);
}

static bool Network_Layer_Is_Sequence_Number_Newer(uint32_t received, uint32_t processed)
{
    // See Spec OPC UA Part 14 - Table 75
    // NetworkMessages the following formula shall be used:
    // (4294967295 + received sequence number – last processed sequence number) modulo 4294967296.
    // Results below 1073741824 indicate that the received NetworkMessages is newer than
    // the last processed NetworkMessages...
    // Results above 3221225472 indicate that the received message is older (or same) than
    // the last processed NetworkMessages...
    // Other results are invalid...
    uint64_t max_uint32 = UINT32_MAX;
    uint64_t diff = max_uint32 + received - processed;
    uint64_t res = diff % (max_uint32 + 1);
    if (1073741824 > res)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SOPC_Is_UInt16_Sequence_Number_Newer(uint16_t received, uint16_t processed)
{
    // See Spec OPC UA Part 14 - Table 133 - rev 1.05
    // NetworkMessages the following formula shall be used:
    // (received sequence number - 1 - last processed sequence number) modulo 65536.
    // Results below 16384 indicate that the received NetworkMessages is newer than
    // the last processed NetworkMessages...
    // Results above 49152 indicate that the received message is older (or same) than
    // the last processed NetworkMessages...
    // Other results are invalid...
    const uint16_t diff = (uint16_t)(received - 1 - processed);

    /* We actually don't make difference between results above upper bound and between lower and upper bound
     * because we don't handle reordering message */
    return diff < 16384;
}

/**
 * \brief Print Variant value in buf according to its type.
 *
 * \param variant   A non-NULL pointer to a SOPC_Variant.
 * \param buf       A non-NULL pointer to a SOPC_Buffer.
 *
 * \return  SOPC_STATUS_NOT_SUPPORTED, if the type to print isn't supported
 *          SOPC_STATUS_OK, if data is printed successfully
 */
static SOPC_ReturnStatus print_variant_into_sopc_buffer(const SOPC_Variant* variant, SOPC_Buffer* buf)
{
    SOPC_ASSERT(NULL != buf && NULL != variant);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    static const uint8_t quote = '"';
    static const char* bool2str[2] = {"false", "true"};

    switch (variant->BuiltInTypeId)
    {
    case SOPC_Boolean_Id:
        status = SOPC_Buffer_Write(buf, (const uint8_t*) bool2str[variant->Value.Boolean],
                                   (uint32_t) strlen(bool2str[variant->Value.Boolean]));
        break;
    case SOPC_UInt16_Id:
        status = SOPC_Buffer_PrintU32(buf, variant->Value.Uint16);
        break;
    case SOPC_UInt32_Id:
        status = SOPC_Buffer_PrintU32(buf, variant->Value.Uint32);
        break;
    case SOPC_Int16_Id:
        status = SOPC_Buffer_PrintI32(buf, variant->Value.Int16);
        break;
    case SOPC_Int32_Id:
        status = SOPC_Buffer_PrintI32(buf, variant->Value.Int32);
        break;
    case SOPC_UInt64_Id:
        // TODO : Understand the OPC UA specification,
        // %llu not supported on some embedded target
        status = SOPC_STATUS_NOT_SUPPORTED;
        break;
    case SOPC_Int64_Id:
        // TODO
        status = SOPC_STATUS_NOT_SUPPORTED;
        break;
    case SOPC_Float_Id:
        status = SOPC_Buffer_PrintFloatDouble(buf, variant->Value.Floatv);
        break;
    case SOPC_Double_Id:
        status = SOPC_Buffer_PrintFloatDouble(buf, variant->Value.Doublev);
        break;
    case SOPC_String_Id:
        status = SOPC_Buffer_Write(buf, &quote, 1);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(buf, (const uint8_t*) variant->Value.String.Data,
                                       (uint32_t) variant->Value.String.Length);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(buf, &quote, 1);
        }
        break;
    default:
        return SOPC_STATUS_NOT_SUPPORTED;
        break;
    }
    return status;
}

/**
 * \brief Print PublisherId in buffer according to its type.
 *
 * \param pubId   A non-NULL pointer to a SOPC_Dataset_LL_PublisherId to print.
 * \param buffer  A non-NULL pointer to a SOPC_Buffer.
 *
 * \return  SOPC_STATUS_NOT_SUPPORTED, if the type to print isn't supported
 *          SOPC_STATUS_OK, if data is printed successfully
 */
static SOPC_ReturnStatus print_publisherId_into_sopc_buffer(const SOPC_Dataset_LL_PublisherId* pubId,
                                                            SOPC_Buffer* buffer)
{
    SOPC_ASSERT(NULL != pubId && NULL != buffer);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    switch (pubId->type)
    {
    case DataSet_LL_PubId_Byte_Id:
        status = SOPC_Buffer_PrintU32(buffer, pubId->data.byte);
        break;
    case DataSet_LL_PubId_UInt16_Id:
        status = SOPC_Buffer_PrintU32(buffer, pubId->data.uint16);
        break;
    case DataSet_LL_PubId_UInt32_Id:
        status = SOPC_Buffer_PrintU32(buffer, pubId->data.uint32);
        break;
    case DataSet_LL_PubId_UInt64_Id:
        status = SOPC_STATUS_NOT_SUPPORTED;
        break;
    case DataSet_LL_PubId_String_Id:
        status = SOPC_Buffer_Write(buffer, pubId->data.string.Data, (uint32_t) pubId->data.string.Length);
        break;
    default:
        break;
    }
    return status;
}

/**
 * \brief Encode NetworkMessage start into JSON format
 *        Currently with 4 elements : "MessageId", "MessageType", "PublisherId", "Messages"
 *
 * \param pPayloadJSON       A non-NULL pointer to a SOPC_Buffer.
 * \param MessageId          A non-NULL pointer to a string representing a unique MessageId.
 * \param PublisherId        A non-NULL pointer to a SOPC_Dataset_LL_PublisherId.
 *
 * \return  A SOPC_ReturnStatus
 */
static SOPC_ReturnStatus SOPC_JSON_Encode_NetworkMessage_Start(SOPC_Buffer* pPayloadJSON,
                                                               const char* MessageId,
                                                               const SOPC_Dataset_LL_PublisherId* PublisherId)
{
    SOPC_ASSERT(NULL != pPayloadJSON && NULL != MessageId && NULL != PublisherId);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    BUFFER_PRINT_STR("{\"MessageId\":", pPayloadJSON, status);
    BUFFER_PRINT_STR(MessageId, pPayloadJSON, status);
    BUFFER_PRINT_STR(",\"MessageType\":\"ua-data\",\"PublisherId\":\"", pPayloadJSON, status);
    if (SOPC_STATUS_OK == status)
    {
        status = print_publisherId_into_sopc_buffer(PublisherId, pPayloadJSON);
    }
    BUFFER_PRINT_STR("\",\"Messages\":[", pPayloadJSON, status);

    return status;
}

/**
 * \brief Encode DataSetMessage start into JSON format
 *        Currently with 3 elements : "DataSetWriterId", "MessageType", "Payload"
 *
 * \param pPayloadJSON       A non-NULL pointer to a SOPC_Buffer.
 * \param WriterId           A uint16_t.
 *
 * \return  A SOPC_ReturnStatus
 */
static SOPC_ReturnStatus SOPC_JSON_Encode_DataSetMessage_Start(SOPC_Buffer* pPayloadJSON, const uint16_t WriterId)
{
    SOPC_ASSERT(NULL != pPayloadJSON);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    BUFFER_PRINT_STR("{\"DataSetWriterId\":", pPayloadJSON, status);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_PrintU32(pPayloadJSON, (const uint32_t) WriterId);
    }
    BUFFER_PRINT_STR(",\"MessageType\":\"ua-keyframe\",\"Payload\":{", pPayloadJSON, status);

    return status;
}

/**
 * \brief Encode Variant into JSON format
 *        Currently with 2 elements : "Type", "Body"
 *
 * \param pPayloadJSON       A non-NULL pointer to a SOPC_Buffer.
 * \param variant            A non-NULL pointer to a SOPC_Variant.
 * \param varName            A non-NULL pointer to a string representing a unique VariantName.
 *
 * \return  A SOPC_ReturnStatus
 */
static SOPC_ReturnStatus SOPC_JSON_Encode_Variant(SOPC_Buffer* pPayloadJSON,
                                                  const SOPC_Variant* variant,
                                                  const char* varName)
{
    SOPC_ASSERT(NULL != pPayloadJSON && NULL != variant && NULL != varName);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    BUFFER_PRINT_STR(varName, pPayloadJSON, status);
    BUFFER_PRINT_STR(":{\"Type\":", pPayloadJSON, status);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_PrintU32(pPayloadJSON, variant->BuiltInTypeId);
    }
    BUFFER_PRINT_STR(",\"Body\":", pPayloadJSON, status);
    if (SOPC_STATUS_OK == status)
    {
        status = print_variant_into_sopc_buffer(variant, pPayloadJSON);
    }
    BUFFER_PRINT_STR("}", pPayloadJSON, status);

    return status;
}

/**
 * Function which generate unique MessageId with groupeId and DatasetMessageSequenceNumber(index) (JSON format)
 *
 * Return SOPC_STATUS_NOK if there is a problem in writing the buffer,
 * Return SOPC_STATUS_OK otherwise
 */
static SOPC_ReturnStatus generate_MessageId_JSON(char* buffer,
                                                 uint16_t sizeBuffer,
                                                 const uint16_t groupeId,
                                                 const uint16_t index)
{
    SOPC_ASSERT(NULL != buffer);
    int use = snprintf(buffer, sizeBuffer, "\"%" PRIu16 "-%" PRIu16 "\"", groupeId, index);
    if (use <= 0 || (int) sizeBuffer <= use)
    {
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

SOPC_NetworkMessage_Error_Code SOPC_JSON_NetworkMessage_Encode(SOPC_Dataset_LL_NetworkMessage* message,
                                                               SOPC_PubSub_SecurityType* security,
                                                               SOPC_Buffer** buffer)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    static const uint8_t comma = ',';

    if (NULL == buffer || NULL != *buffer || NULL == message)
    {
        return SOPC_NetworkMessage_Error_Code_InvalidParameters;
    }
    if (NULL != security) // security not supported
    {
        return SOPC_JSON_NetworkMessage_Error_Security_Unsupported;
    }

    // DatasetMessageSequenceNumber
    const SOPC_Dataset_LL_DataSetMessage* dsm0 = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, 0);
    const uint16_t datasetMessageSequenceNumber = SOPC_Dataset_LL_DataSetMsg_Get_SequenceNumber(dsm0);
    const uint16_t groupeId = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(message);
    char messageId[(2 * SOPC_MAX_LENGTH_UINT16_TO_STRING) + 1] = {0};
    status = generate_MessageId_JSON(messageId, 2 * SOPC_MAX_LENGTH_UINT16_TO_STRING, groupeId,
                                     datasetMessageSequenceNumber);
    code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_Generate_Unique_MessageId);

    // PublisherId
    const SOPC_Dataset_LL_NetworkMessage_Header* Header = SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(message);
    const SOPC_Dataset_LL_PublisherId* publisherId = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(Header);

    // ~200 oct for header with only 1 dataSetMessage and 1 simple Variant (without dimension parameter)
    // NetworkMessage elements : "MessageId", "MessageType", "PublisherId", "Messages"
    // DataSetMessage elements : "DataSetWriterId", "MessageType", "Payload"
    // Variant elements : "Type", "Body"
    *buffer = SOPC_Buffer_CreateResizable(512, SOPC_PUBSUB_BUFFER_SIZE);
    if (NULL == *buffer)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        code = SOPC_NetworkMessage_Error_Write_Alloc_Failed;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_JSON_Encode_NetworkMessage_Start(*buffer, messageId, publisherId);
        code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_Encode);
    }

    uint8_t nDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message);
    for (uint8_t iDsm = 0; iDsm < nDsm && SOPC_STATUS_OK == status; iDsm++)
    {
        const SOPC_Dataset_LL_DataSetMessage* dsm =
            SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, (int) iDsm);
        uint16_t WriterId = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm);

        status = SOPC_JSON_Encode_DataSetMessage_Start(*buffer, WriterId);
        code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_DataSetMessage_Encode);

        uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        for (uint16_t iField = 0; iField < nbFields && SOPC_STATUS_OK == status; ++iField)
        {
            // here, this function is used to create a unique variant name! (iDsm-iField)
            status = generate_MessageId_JSON(messageId, 2 * SOPC_MAX_LENGTH_UINT16_TO_STRING, iDsm, iField);
            code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_Generate_Unique_VariantName);
            if (SOPC_STATUS_OK == status)
            {
                const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(dsm, iField);
                status = SOPC_JSON_Encode_Variant(*buffer, var, messageId);
                code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_Variant_Encode);
            }
            // add comma if it's not the last Variant
            if ((iField + 1) < nbFields && SOPC_STATUS_OK == status)
            {
                status = SOPC_Buffer_Write(*buffer, &comma, 1);
            }
        }
        // close Payload & DataSetMessage on JSON file
        BUFFER_PRINT_STR("}}", *buffer, status);

        // add comma if it's not the last DatasetMessage
        if ((iDsm + 1) < nDsm && SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(*buffer, &comma, 1);
        }
    }
    // closing NetworkMessage on JSON file
    BUFFER_PRINT_STR("]}", *buffer, status);
    // If this wasn't the case before, the only problem is an error closing the JSON structure.
    if (SOPC_STATUS_OK != status && SOPC_NetworkMessage_Error_Code_None == code)
    {
        code = checkAndGetErrorCode(status, SOPC_JSON_NetworkMessage_Error_Write_Closing_Structure);
    }

    // Set buffer position to 0 for reading
    if (NULL != *buffer)
    {
        SOPC_Buffer_SetPosition(*buffer, 0);
    }
    // If a problem occurs during encoding, the buffer is deleted and NULL is returned
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Buffer_Delete(*buffer);
        *buffer = NULL;
    }

    if (SOPC_NetworkMessage_Error_Code_None != code)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "JSON NetworkMessage encode failed with error = %" PRIu32,
                                 (uint32_t) code);
    }

    return code;
}

SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Encode_Buffers(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                       SOPC_PubSub_SecurityType* security,
                                                                       SOPC_Buffer** buffer_header,
                                                                       SOPC_Buffer** buffer_payload)
{
    SOPC_NetworkMessage_Error_Code res = SOPC_NetworkMessage_Error_Code_None;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t* dsmSizeBufferPos = NULL;
    uint8_t byte = 0;
    bool flags1_enabled = false;
    // security flags is enabled
    bool securityEnabled = (NULL != security);
    bool signedEnabled = false;
    bool encryptedEnabled = false;

    bool preencodedEnabled = SOPC_DataSet_LL_NetworkMessage_is_Preencode_Buffer_Enabled(nm);
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode = SOPC_DataSet_LL_NetworkMessage_Get_Preencode_Buffer(nm);
    SOPC_Dataset_LL_NetworkMessage_Header* header = NULL;
    uint8_t dsm_count = 0;
    uint32_t bufferPosition = 0;

    if (NULL == buffer_header || NULL == buffer_payload || NULL != *buffer_header || NULL != *buffer_payload ||
        NULL == nm || (securityEnabled && NULL == security->groupKeys))
    {
        return SOPC_NetworkMessage_Error_Code_InvalidParameters;
    }
    if (SOPC_STATUS_OK == status)
    {
        *buffer_header = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
        *buffer_payload = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
        if (NULL == *buffer_payload || NULL == *buffer_header)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            res = SOPC_NetworkMessage_Error_Write_Alloc_Failed;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
        SOPC_ASSERT(NULL != header);
        dsm_count = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    }

    if (SOPC_STATUS_OK == status && NULL != security)
    {
        signedEnabled =
            (SOPC_SecurityMode_Sign == security->mode || SOPC_SecurityMode_SignAndEncrypt == security->mode);
        encryptedEnabled = (SOPC_SecurityMode_SignAndEncrypt == security->mode);
    }

    if (SOPC_STATUS_OK == status)
    {
        // UADP version bit 0-3
        byte = SOPC_Dataset_LL_NetworkMessage_GetVersion(header);
        // UADP flags bit 4-7
        //  - PublisherId enabled
        Network_Message_Set_Bool_Bit(&byte, 4, DATASET_LL_PUBLISHER_ID_ENABLED);
        //  - GroupHeader enabled
        Network_Message_Set_Bool_Bit(&byte, 5, DATASET_LL_GROUP_HEADER_ENABLED);
        //  - PayloadHeader enabled
        Network_Message_Set_Bool_Bit(&byte, 6, DATASET_LL_PAYLOAD_HEADER_ENABLED);
        //  - ExtendedFlags1 enabled
        flags1_enabled = Network_Layer_Is_Flags1_Enabled(header, securityEnabled);
        Network_Message_Set_Bool_Bit(&byte, 7, flags1_enabled);
        status = SOPC_Buffer_Write(*buffer_header, &byte, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
    }

    if (flags1_enabled && SOPC_STATUS_OK == status)
    {
        // Bit range 0-2: PublisherId Type
        byte = (uint8_t) SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->type;
        Network_Message_Set_Bool_Bit(&byte, 3, DATASET_LL_DATASET_CLASSID_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 4, securityEnabled);
        Network_Message_Set_Bool_Bit(&byte, 5, DATASET_LL_TIMESTAMP_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 6, DATASET_LL_PICOSECONDS_ENABLED);
        Network_Message_Set_Bool_Bit(&byte, 7, DATASET_LL_EXTENDED_FLAGS2_ENABLED);

        status = SOPC_Buffer_Write(*buffer_header, &byte, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
    }

    if (DATASET_LL_PUBLISHER_ID_ENABLED && SOPC_STATUS_OK == status)
    {
        status =
            Network_Layer_PublisherId_Write(*buffer_header, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header));
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_PubId_Failed);
    }

    if (DATASET_LL_DATASET_CLASSID_ENABLED && SOPC_STATUS_OK == status)
    {
        // Note :  DATASET_LL_DATASET_CLASSID_ENABLED is disabled in this version.
        // Otherwise, DataSetClassId should be encoded here
        SOPC_ASSERT(false);
    }

    // GroupHeader
    if (SOPC_STATUS_OK == status)
    {
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
        status = SOPC_Buffer_Write(*buffer_header, &byte, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
    }

    if (DATASET_LL_WRITER_GROUP_ID_ENABLED && SOPC_STATUS_OK == status)
    {
        uint16_t byte_2 = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm);
        status = SOPC_UInt16_Write(&byte_2, *buffer_header, 0);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_GroupId_Failed);
    }

    if (DATASET_LL_WRITER_GROUP_VERSION_ENABLED && SOPC_STATUS_OK == status)
    {
        uint32_t version = SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm);
        status = SOPC_UInt32_Write(&version, *buffer_header, 0);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_GroupVersion_Failed);
    }

    // payload header
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(*buffer_header, &dsm_count, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);

        for (int i = 0; i < dsm_count && SOPC_STATUS_OK == status; i++)
        {
            SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
            // - writer id
            uint16_t byte_2 = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm);
            status = SOPC_UInt16_Write(&byte_2, *buffer_header, 0);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_WriterId_Failed);
        }
    }

    // security header
    if (securityEnabled && SOPC_STATUS_OK == status)
    {
        uint32_t nonceRandomLength;
        byte = 0;
        // - NetworkMessage Signed
        Network_Message_Set_Bool_Bit(&byte, 0, signedEnabled);
        // - NetworkMessage Encrypted
        Network_Message_Set_Bool_Bit(&byte, 1, encryptedEnabled);
        // - Security Footer enabled
        Network_Message_Set_Bool_Bit(&byte, 2, DATASET_LL_SECURITY_FOOTER_ENABLED);
        // - Force key reset
        Network_Message_Set_Bool_Bit(&byte, 3, DATASET_LL_SECURITY_KEY_RESET_ENABLED);
        status = SOPC_Buffer_Write(*buffer_header, &byte, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&security->groupKeys->tokenId, *buffer_header, 0);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_TokenId_Failed);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_PubSubGetLength_MessageRandom(security->provider, &nonceRandomLength);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_SecuHdr_Failed);

            SOPC_ASSERT(4 == nonceRandomLength && // Size is fixed to 4 bytes. See OPCUA Spec Part 14 - Table 75
                        nonceRandomLength + 4 <= UINT8_MAX); // check before cast to uint8


            // Random bytes + SequenceNumber 32 bits
            if (SOPC_STATUS_OK == status)
            {
                uint8_t nonceLength = (uint8_t)(nonceRandomLength + 4);
                status = SOPC_Byte_Write(&nonceLength, *buffer_header, 0);
                res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PubSub_Security_Write_Nonce(security, *buffer_header, (uint8_t) nonceRandomLength);
            checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_SecuHdr_Failed);
        }



        if (DATASET_LL_SECURITY_FOOTER_ENABLED && SOPC_STATUS_OK == status)
        {
            // Security Footer is not used with AES-CTR
            status = SOPC_STATUS_NOK;
            res = SOPC_UADP_NetworkMessage_Error_Write_SecuFooter_Failed;
        }
    }

    // payload: write Payload buffer.

    // Information used when prencoding buffer
    size_t indexDataSetField = 0;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_GetPosition(*buffer_header, &bufferPosition);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    if (DATASET_LL_PAYLOAD_HEADER_ENABLED && dsm_count > 1 && SOPC_STATUS_OK == status)
    {
        dsmSizeBufferPos = SOPC_Calloc(dsm_count, sizeof(*dsmSizeBufferPos));
        if (NULL == dsmSizeBufferPos)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            res = SOPC_NetworkMessage_Error_Write_Alloc_Failed;
        }

        const uint16_t zero = 0;

        // DataSet Message size(2 bytes)
        // Sizes are unknown yet. Write Zeros, and store current position to write it later
        for (int i = 0; SOPC_STATUS_OK == status && i < dsm_count; i++)
        {
            status = SOPC_Buffer_GetPosition(*buffer_payload, &dsmSizeBufferPos[i]);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_DsmPreSize_Failed);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_UInt16_Write(&zero, *buffer_payload, 0);
                res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_DsmPreSize_Failed);
            }
        }
    }

    for (int i = 0; i < dsm_count && SOPC_STATUS_OK == status; i++)
    {
        // dsmStartBufferPos is set with buffer position before DSM content
        uint32_t dsmStartBufferPos;
        bool dsmFlags2Enable = false;
        status = SOPC_Buffer_GetPosition(*buffer_payload, &dsmStartBufferPos);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
        SOPC_ASSERT(NULL != dsm);
        const SOPC_DataSet_LL_UadpDataSetMessageContentMask* conf = SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(dsm);
        SOPC_ASSERT(NULL != conf);

        dsmFlags2Enable = Network_Layer_DataSetMessage_Is_Flags2_Enabled(*conf);

        // DataSetMessage (1 byte)

        // - DataSet Flags 1
        //   - fieldEncoding is variant
        SOPC_ASSERT(DataSet_LL_FieldEncoding_Variant == conf->fieldEncoding && "Only variant encoding supported");
        SOPC_ASSERT(conf->fieldEncoding <= 2);
        byte = (uint8_t)(conf->fieldEncoding << 1); // field encoding starts at bit 1

        //   - DataSetMessage isValid
        Network_Message_Set_Bool_Bit(&byte, 0, conf->validFlag);
        SOPC_ASSERT(DATASET_LL_DSM_IS_VALID == conf->validFlag && "Only valid DSM allowed");

        //   - sequence number is enabled
        Network_Message_Set_Bool_Bit(&byte, 3, conf->dataSetMessageSequenceNumberFlag);

        //   - status
        Network_Message_Set_Bool_Bit(&byte, 4, conf->statusFlag);
        SOPC_ASSERT(DATASET_LL_DSM_STATUS_ENABLED == conf->statusFlag && "Status not supported");

        //   - major version
        Network_Message_Set_Bool_Bit(&byte, 5, conf->configurationVersionMajorVersionFlag);
        SOPC_ASSERT(DATASET_LL_DSM_MAJOR_VERSION_ENABLED == conf->configurationVersionMajorVersionFlag &&
                    "Major version not supported");

        //   - minor version
        Network_Message_Set_Bool_Bit(&byte, 6, conf->configurationVersionMinorFlag);
        SOPC_ASSERT(DATASET_LL_DSM_MINOR_VERSION_ENABLED == conf->configurationVersionMinorFlag &&
                    "Minor version not supported");

        //   - DataSet Flags 2
        Network_Message_Set_Bool_Bit(&byte, 7, dsmFlags2Enable);

        status = SOPC_Buffer_Write(*buffer_payload, (uint8_t*) &byte, 1);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);

        // - DataSet Flags 2 (1 byte)
        if (dsmFlags2Enable && SOPC_STATUS_OK == status)
        {
            //   - dataSetMessageType (bits 0-3)
            byte = (uint8_t) conf->dataSetMessageType;
            SOPC_ASSERT(DataSet_LL_MessageType_DeltaFrame != conf->dataSetMessageType && "Unsupported message type");
            SOPC_ASSERT(conf->dataSetMessageType <= DataSet_LL_MessageType_KeepAlive && "Unsupported Message type");
            // - Timestamp enabled
            Network_Message_Set_Bool_Bit(&byte, 4, conf->timestampFlag);

            Network_Message_Set_Bool_Bit(&byte, 5, conf->picoSecondsFlag);
            SOPC_ASSERT(DATASET_LL_DSM_PICOSECONDS_ENABLED == conf->picoSecondsFlag && "Picoseconds not supported");
            //   - status is disabled

            status = SOPC_Buffer_Write(*buffer_payload, (uint8_t*) &byte, 1);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Buffer_Failed);
        }

        // DataSetMessage Sequence Number
        if (SOPC_STATUS_OK == status && conf->dataSetMessageSequenceNumberFlag)
        {
            if (preencodedEnabled)
            {
                uint32_t bufferPayloadPosition = 0;
                status = SOPC_Buffer_GetPosition(*buffer_payload, &bufferPayloadPosition);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
                res = SOPC_PubFixedBuffer_Set_DSM_SequenceNumber_Position_At(
                    preencode, bufferPayloadPosition + bufferPosition, (size_t) i);
                SOPC_ASSERT(res);
            }
            uint16_t dsmSN = SOPC_Dataset_LL_DataSetMsg_Get_SequenceNumber(dsm);
            status = SOPC_UInt16_Write(&dsmSN, *buffer_payload, 0);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_DsmSeqNum_Failed);
        }

        // DataSetMessage Timestamp
        if (SOPC_STATUS_OK == status && conf->timestampFlag)
        {
            if (preencodedEnabled)
            {
                uint32_t bufferPayloadPosition = 0;
                status = SOPC_Buffer_GetPosition(*buffer_payload, &bufferPayloadPosition);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
                res = SOPC_PubFixedBuffer_Set_DSM_Timestamp_Position_At(
                    preencode, bufferPayloadPosition + bufferPosition, (size_t) i);
                SOPC_ASSERT(res);
            }
            uint64_t dsmTimestamp = (uint64_t) SOPC_Time_GetCurrentTimeUTC();
            SOPC_Dataset_LL_DataSetMsg_Set_Timestamp(dsm, dsmTimestamp);
            status = SOPC_UInt64_Write(&dsmTimestamp, *buffer_payload, 0);
            res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_DsmTimestamp_Failed);
        }

        // If message is not a keep alive type, encode data fields
        if (SOPC_STATUS_OK == status && DataSet_LL_MessageType_KeepAlive != conf->dataSetMessageType)
        {
            uint32_t* bufferPayload_dsfPositions = NULL;
            if (preencodedEnabled)
            {
                uint16_t nbVariant = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
                bufferPayload_dsfPositions = SOPC_Calloc(nbVariant, sizeof(uint32_t));
                if (NULL == bufferPayload_dsfPositions)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                status = Network_DataSetFields_To_UADP(*buffer_payload, dsm, bufferPayload_dsfPositions);
                res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_DsmField_Failed);
            }
            if (preencodedEnabled && SOPC_STATUS_OK == status)
            {
                uint16_t nbVariant = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
                for (int j = 0; j < nbVariant; j++)
                {
                    SOPC_PubFixedBuffer_DataSetField_Position* dsfPosition =
                        SOPC_PubFixedBuffer_Get_DataSetField_Position_At(preencode, indexDataSetField);
                    SOPC_ASSERT(NULL != dsfPosition);
                    SOPC_PubFixedBuffer_DataSetFieldPosition_Set_Position(
                        dsfPosition, bufferPayload_dsfPositions[j] + bufferPosition);
                    indexDataSetField++;
                }
            }
            if (NULL != bufferPayload_dsfPositions)
            {
                SOPC_Free(bufferPayload_dsfPositions);
            }
        }
        if (NULL != dsmSizeBufferPos && SOPC_STATUS_OK == status)
        {
            // Write the DSM size at the payload start
            uint32_t dsmEndBufferPos;
            status = SOPC_Buffer_GetPosition(*buffer_payload, &dsmEndBufferPos);
            SOPC_ASSERT(SOPC_STATUS_OK == status);

            const uint16_t dsmSize = (uint16_t)(dsmEndBufferPos - dsmStartBufferPos);
            bool writeOk = true;
            writeOk &= (SOPC_STATUS_OK == SOPC_Buffer_SetPosition(*buffer_payload, dsmSizeBufferPos[i]));
            writeOk &= (SOPC_STATUS_OK == SOPC_UInt16_Write(&dsmSize, *buffer_payload, 0));
            writeOk &= (SOPC_STATUS_OK == SOPC_Buffer_SetPosition(*buffer_payload, dsmEndBufferPos));

            if (!writeOk)
            {
                set_status_default(&status, &res, SOPC_UADP_NetworkMessage_Error_Write_DsmSize_Failed);
            }
        }
    }
    if (NULL != dsmSizeBufferPos)
    {
        SOPC_Free(dsmSizeBufferPos);
        dsmSizeBufferPos = NULL;
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != *buffer_header)
        {
            SOPC_Buffer_Delete(*buffer_header);
            *buffer_header = NULL;
        }
        if (NULL != *buffer_payload)
        {
            SOPC_Buffer_Delete(*buffer_payload);
            *buffer_payload = NULL;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_ASSERT(SOPC_NetworkMessage_Error_Code_None != res);
    }
    return res;
}

SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_BuildFinalMessage(SOPC_PubSub_SecurityType* security,
                                                                          SOPC_Buffer* buffer_header,
                                                                          SOPC_Buffer** buffer_payload)
{
    SOPC_NetworkMessage_Error_Code res = SOPC_NetworkMessage_Error_Code_None;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == buffer_header || NULL == buffer_payload || NULL == *buffer_payload)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
        res = SOPC_NetworkMessage_Error_Code_InvalidParameters;
    }

    bool signedEnabled = false;
    bool encryptedEnabled = false;

    if (NULL != security && SOPC_STATUS_OK == status)
    {
        if (NULL == security->groupKeys)
        {
            // Keys needed when security is enabled
            status = SOPC_STATUS_INVALID_PARAMETERS;
            res = SOPC_NetworkMessage_Error_Code_InvalidParameters;
        }
        else
        {
            signedEnabled =
                (SOPC_SecurityMode_Sign == security->mode || SOPC_SecurityMode_SignAndEncrypt == security->mode);
            encryptedEnabled = (SOPC_SecurityMode_SignAndEncrypt == security->mode);
        }
    }

    // Encrypt the Payload if encrypt is enabled
    if (encryptedEnabled && SOPC_STATUS_OK == status && (*buffer_payload)->length > 0)
    {
        SOPC_Buffer* payload_encrypted = SOPC_PubSub_Security_Encrypt(security, *buffer_payload);
        if (NULL == payload_encrypted)
        {
            status = SOPC_STATUS_NOK;
            res = SOPC_UADP_NetworkMessage_Error_Write_EncryptPaylod_Failed;
        }
        else
        {
            // replace payload by the encrypted buffer
            SOPC_Buffer_Delete(*buffer_payload);
            *buffer_payload = payload_encrypted;
        }
    }
    // Write the Payload in the NetworkMessage Buffer
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Buffer_SetPosition(*buffer_payload, 0);
        int64_t nbread = SOPC_Buffer_ReadFrom(buffer_header, *buffer_payload, (*buffer_payload)->length);
        status = SOPC_Buffer_SetPosition(buffer_header, buffer_header->length);

        if ((*buffer_payload)->length != nbread || SOPC_STATUS_OK != status)
        {
            status = SOPC_STATUS_NOK;
            res = SOPC_UADP_NetworkMessage_Error_Write_PayloadFlush_Failed;
        }
    }

    if (NULL != buffer_payload && NULL != *buffer_payload)
    {
        SOPC_Buffer_Delete(*buffer_payload);
        *buffer_payload = NULL;
    }

    // Signature
    if (signedEnabled && SOPC_STATUS_OK == status)
    {
        status = SOPC_PubSub_Security_Sign(security, buffer_header);
        res = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Write_Sign_Failed);
    }

    if (NULL != buffer_header)
    {
        SOPC_Buffer_SetPosition(buffer_header, 0);
    }

    return res;
}

SOPC_Buffer* SOPC_UADP_NetworkMessage_Get_PreencodedBuffer(SOPC_Dataset_LL_NetworkMessage* nm,
                                                           SOPC_PubSub_SecurityType* security)
{
    if (NULL == nm || NULL != security)
    {
        return NULL;
    }
    SOPC_PubFixedBuffer_Buffer_Ctx* preencode = SOPC_DataSet_LL_NetworkMessage_Get_Preencode_Buffer(nm);
    SOPC_Buffer* buffer = SOPC_PubFixedBuffer_Get_UpdatedBuffer(preencode);
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(buffer, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    return buffer;
}

/**
 * /brief Decode the payload of a datasetmessage
 * /param dsm The DataSetMessage to encode
 * /param buffer_payload The Payload
 * /param dsmSize The expected size (bytes) of the decoded payload from the DataSetMessage.
 *  Set to 0 if unknown
 * /return SOPC_NetworkMessage_Error_Code_None in case of success:
 *    - dsmSize matches actual decoded size, or dsmSize = 0
 *    - decoding stream is consistent (and payload large enough).
 *    Otherwise, a specific error code is returned.
 */
static SOPC_NetworkMessage_Error_Code decode_dataSetMessage(
    SOPC_Dataset_LL_DataSetMessage* dsm,
    SOPC_Buffer* buffer_payload,
    SOPC_Conf_PublisherId pubId,
    const uint16_t groupId,
    uint16_t dsmSize,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* readerConf)
{
    SOPC_ASSERT(NULL != buffer_payload);
    SOPC_ASSERT(NULL != dsm);
    // Note :  dsmSizes may be NULL

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    SOPC_Byte data;

    /* Retrieve Content mask partially filled */
    SOPC_DataSet_LL_UadpDataSetMessageContentMask dsm_conf = *SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(dsm);

    uint32_t dsmStartBufferPos;
    status = SOPC_Buffer_GetPosition(buffer_payload, &dsmStartBufferPos);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    bool dsmFlags2Enable = false;
    /* DataSetMessages Header */

    /** DataSetFlags1 **/
    if (DATASET_LL_FLAGS1_ENABLED)
    {
        status = SOPC_Byte_Read(&data, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

        if (SOPC_STATUS_OK == status)
        {
            status = valid_bool_to_status(Network_Message_Get_Bool_Bit(data, 0));
            code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_InvalidBit);
        }

        if (SOPC_STATUS_OK == status)
        {
            dsm_conf.fieldEncoding = (data & (uint8_t)(C_NETWORK_MESSAGE_BIT_1 + C_NETWORK_MESSAGE_BIT_2)) >> 1;
            if (DataSet_LL_FieldEncoding_Variant != dsm_conf.fieldEncoding)
            {
                // not managed yet
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_EncodingType);
            }
            else
            {
                dsm_conf.dataSetMessageSequenceNumberFlag = Network_Message_Get_Bool_Bit(data, 3);
                dsm_conf.statusFlag = Network_Message_Get_Bool_Bit(data, 4);
                dsm_conf.configurationVersionMajorVersionFlag = Network_Message_Get_Bool_Bit(data, 5);
                dsm_conf.configurationVersionMinorFlag = Network_Message_Get_Bool_Bit(data, 6);
                dsmFlags2Enable = Network_Message_Get_Bool_Bit(data, 7);
            }
        }
    }

    /** dataSetFlags2 **/
    if (dsmFlags2Enable && SOPC_STATUS_OK == status)
    {
        // Bit range 0-3: UADP DataSetMessage type
        // Bit 4: Timestamp enabled
        // Bit 5: PicoSeconds enabled
        status = SOPC_Byte_Read(&data, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

        if (SOPC_STATUS_OK == status)
        {
            dsm_conf.dataSetMessageType = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_4 - 1);
            if (DataSet_LL_MessageType_DeltaFrame == dsm_conf.dataSetMessageType)
            {
                // Not managed yet
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_DsmType);
            }
            else
            {
                dsm_conf.timestampFlag = Network_Message_Get_Bool_Bit(data, 4);
                dsm_conf.picoSecondsFlag = Network_Message_Get_Bool_Bit(data, 5);
            }
        }
    }
    /** DataSetMessage SequenceNumber **/
    if (dsm_conf.dataSetMessageSequenceNumberFlag && SOPC_STATUS_OK == status)
    {
        uint16_t dsmSN = 0;
        status = SOPC_UInt16_Read(&dsmSN, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_DsmSeqNum_Failed);
        if (SOPC_STATUS_OK == status)
        {
            if (NULL != readerConf->checkDataSetMessageSN_Func)
            {
                /* If tuple [PublisherId, DataSetWriterId] is not defined don't check the dataSetMessage sequence number
                 */
                const uint16_t writerId = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm);
                if (0 != writerId)
                {
                    /* If subscriber doesn't meet configuration or is not newer still decode dataSetMessage */
                    if (readerConf->checkDataSetMessageSN_Func(&pubId, groupId, writerId, dsmSN))
                    {
                        SOPC_Dataset_LL_DataSetMsg_Set_SequenceNumber(dsm, dsmSN);
                    }
                }
            }
        }
    }

    /** Timestamp **/
    if (dsm_conf.timestampFlag && SOPC_STATUS_OK == status)
    {
        uint64_t timestamp;
        status = SOPC_UInt64_Read(&timestamp, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_DsmTimeStamp);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Dataset_LL_DataSetMsg_Set_Timestamp(dsm, timestamp);
        }
    }

    /** PicoSeconds **/
    if (dsm_conf.picoSecondsFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        uint16_t notUsed;
        status = SOPC_UInt16_Read(&notUsed, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Unsupported_DsmPicoseconds);
    }

    /** Status **/
    if (dsm_conf.statusFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        uint16_t notUsed;
        status = SOPC_UInt16_Read(&notUsed, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Short_Failed);
    }

    /** ConfigurationVersion MajorVersion **/
    if (dsm_conf.configurationVersionMajorVersionFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        uint32_t not_used;
        status = SOPC_UInt32_Read(&not_used, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Int_Failed);
    }

    /** ConfigurationVersion MinorVersion **/
    if (dsm_conf.configurationVersionMinorFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        uint32_t not_used;
        status = SOPC_UInt32_Read(&not_used, buffer_payload, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Int_Failed);
    }

    /** Set content mask to associated Dsm */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(dsm, &dsm_conf);
    }

    if (SOPC_STATUS_OK == status && DataSet_LL_MessageType_KeepAlive != dsm_conf.dataSetMessageType)
    {
        /* Data Key Frame DataSetMessage Data */
        status = UADP_To_DataSetFields(buffer_payload, dsm);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_DsmFields_Failed);
    }

    if (0 != dsmSize && SOPC_STATUS_OK == status)
    {
        uint32_t dsmEndBufferPos;
        status = SOPC_Buffer_GetPosition(buffer_payload, &dsmEndBufferPos);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        const uint16_t dsmBufferSize = (uint16_t)(dsmEndBufferPos - dsmStartBufferPos);

        if (dsmBufferSize != dsmSize)
        {
            // Mismatching DSM buffer size.
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_DsmSizeCheck_Failed);
        }
    }

    return code;
}

static inline SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessageHeader_Decode(
    SOPC_Buffer* buffer,
    SOPC_Dataset_LL_NetworkMessage_Header* header)
{
    SOPC_ASSERT(NULL != header);

    SOPC_ReturnStatus status;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    SOPC_Boolean flags1_enabled = false;
    SOPC_Boolean flags2_enabled = false;
    SOPC_Byte data = 0;
    SOPC_Byte version = 0;
    SOPC_UADP_Configuration* conf = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);
    // Publisher Id Type. Read in Extended flags1 or Byte if no flags1
    SOPC_Byte pub_id_type = DataSet_LL_PubId_Byte_Id;

    status = SOPC_Byte_Read(&data, buffer, 0);
    code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

    // Version and Flags
    //  Bit range 0-3: Version of the UADP NetworkMessage
    //  Bit 4: PublisherId enabled
    //  Bit 5: GroupHeader enabled
    //  Bit 6: PayloadHeader enabled
    //  Bit 7: ExtendedFlags1 enabled
    if (SOPC_STATUS_OK == status)
    {
        version = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_4 - 1);
        SOPC_Dataset_LL_NetworkMessage_SetVersion(header, version);
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
    if (flags1_enabled && SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Read(&data, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

        if (SOPC_STATUS_OK == status)
        {
            pub_id_type = data & (uint8_t)(C_NETWORK_MESSAGE_BIT_3 - 1);
            conf->DataSetClassIdFlag = Network_Message_Get_Bool_Bit(data, 3);
            conf->SecurityFlag = Network_Message_Get_Bool_Bit(data, 4);
            conf->TimestampFlag = Network_Message_Get_Bool_Bit(data, 5);
            conf->PicoSecondsFlag = Network_Message_Get_Bool_Bit(data, 6);
            flags2_enabled = Network_Message_Get_Bool_Bit(data, 7);
            if (conf->TimestampFlag || conf->PicoSecondsFlag)
            {
                // not managed yet
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_Flags1);
            }
        }
    }
    else
    {
        conf->DataSetClassIdFlag = false;
        conf->SecurityFlag = false;
        conf->TimestampFlag = false;
        conf->PicoSecondsFlag = false;
        flags2_enabled = false;
    }

    // The following bits are not managed for now:
    // Bit 0: Chunk message
    // Bit 1: PromotedFields enabled
    // Bit range 2-4: UADP NetworkMessage type
    // Others: not used
    if (flags2_enabled && SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Read(&data, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

        if (SOPC_STATUS_OK == status)
        {
            const bool chunkMsg = Network_Message_Get_Bool_Bit(data, 0);
            conf->PromotedFieldsFlag = Network_Message_Get_Bool_Bit(data, 1);
            const uint8_t messageType = (data >> 2) & 0x07;
            if (chunkMsg || conf->PromotedFieldsFlag || messageType != 0)
            {
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_Flags2);
            }
        }
    }

    if (conf->PublisherIdFlag && SOPC_STATUS_OK == status)
    {
        status = Network_Layer_PublisherId_Read(buffer, pub_id_type, header);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Unsupported_PubIdType);
    }

    if (conf->DataSetClassIdFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_ClassId);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_ASSERT(SOPC_NetworkMessage_Error_Code_None != code);
    }
    return code;
}

static inline SOPC_NetworkMessage_Error_Code Decode_GroupHeader(SOPC_Buffer* buffer,
                                                                SOPC_Dataset_LL_NetworkMessage* nm,
                                                                SOPC_Dataset_LL_NetworkMessage_Header* header,
                                                                SOPC_UADP_Configuration* conf)
{
    SOPC_ASSERT(NULL != buffer && NULL != header);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    uint16_t group_id = 0; // default value means not used

    // Group Header
    if (conf->GroupHeaderFlag && SOPC_STATUS_OK == status)
    {
        // Bit 0: WriterGroupId enabled
        // Bit 1: GroupVersion enabled
        // Bit 2: NetworkMessageNumber enabled
        // Bit 3: SequenceNumber enabled
        // Others: not used
        {
            SOPC_Byte data;
            status = SOPC_Byte_Read(&data, buffer, 0);
            code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);

            if (SOPC_STATUS_OK == status)
            {
                conf->GroupIdFlag = Network_Message_Get_Bool_Bit(data, 0);
                conf->GroupVersionFlag = Network_Message_Get_Bool_Bit(data, 1);
                conf->NetworkMessageNumberFlag = Network_Message_Get_Bool_Bit(data, 2);
                conf->SequenceNumberFlag = Network_Message_Get_Bool_Bit(data, 3);
            }
        }

        if (conf->GroupIdFlag && SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt16_Read(&group_id, buffer, 0);
            code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Short_Failed);

            if (SOPC_STATUS_OK == status)
            {
                SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, group_id);
            }
        }

        if (conf->GroupVersionFlag && SOPC_STATUS_OK == status)
        {
            uint32_t group_version;
            status = SOPC_UInt32_Read(&group_version, buffer, 0);
            code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Int_Failed);

            if (SOPC_STATUS_OK == status)
            {
                SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, group_version);
            }
        }

        if (conf->NetworkMessageNumberFlag && SOPC_STATUS_OK == status)
        {
            // not managed yet
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_MessageNum);
        }

        if (conf->SequenceNumberFlag && SOPC_STATUS_OK == status)
        {
            // not managed yet
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_SeqNum);
        }
    }
    return code;
}

static inline SOPC_NetworkMessage_Error_Code Decode_SecurityHeader(SOPC_Buffer* buffer,
                                                                   SOPC_Buffer** buffer_payload,
                                                                   uint32_t payload_position,
                                                                   uint16_t group_id,
                                                                   SOPC_UADP_GetSecurity_Func getSecurity_Func,
                                                                   SOPC_Dataset_LL_NetworkMessage_Header* header,
                                                                   SOPC_UADP_Configuration* conf)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    uint8_t data;
    uint32_t securityTokenId;
    uint8_t securityNonceLength;
    uint8_t securityMessageNonce[4];
    uint32_t sequenceNumber;
    uint8_t securitySignedEnabled = false;
    uint8_t securityEncryptedEnabled = false;
    uint8_t securityFooterEnabled = false;
    uint8_t securityResetEnabled = false;
    SOPC_PubSub_SecurityType* security = NULL;

    if (NULL == getSecurity_Func || 0 == group_id || !conf->PublisherIdFlag)
    {
        // Security information cannot be retrieved. The message is not processed
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_Security_Failed);
    }
    else
    {
        status = SOPC_Byte_Read(&data, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        securitySignedEnabled = Network_Message_Get_Bool_Bit(data, 0);
        securityEncryptedEnabled = Network_Message_Get_Bool_Bit(data, 1);
        securityFooterEnabled = Network_Message_Get_Bool_Bit(data, 2);
        securityResetEnabled = Network_Message_Get_Bool_Bit(data, 3);
    }

    if ((securityFooterEnabled || securityResetEnabled) && SOPC_STATUS_OK == status)
    {
        // security footer not used and reset not managed.
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_SecurityFooterReset);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&securityTokenId, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Int_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* TODO: maybe this message is encrypted but we are not in the right group to decrypt it */
        security = getSecurity_Func(
            securityTokenId, Network_Layer_Convert_PublisherId(SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)),
            group_id);
        // Checked the security configuration of subscriber
        if (NULL == security ||
            !Network_Check_ReceivedSecurityMode(security->mode, securitySignedEnabled, securityEncryptedEnabled))
        {
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SecurityConf_Failed);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(securityTokenId == security->groupKeys->tokenId);

        // Check signature before decoding
        if (securitySignedEnabled && !SOPC_PubSub_Security_Verify(security, buffer, payload_position))
        {
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SecuritySign_Failed);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Read(&securityNonceLength, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (securityNonceLength != 8)
        {
            // See Spec OPC UA Part 14 - Table 76
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SecurityNonce_Failed);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Read(securityMessageNonce, buffer, 4);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Get sequence number and check it is greater than last one received for the same (TokenId, PublisherId)
        status = SOPC_UInt32_Read(&sequenceNumber, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_SeqNum_Failed);
    }

    if (SOPC_STATUS_OK == status && !Network_Layer_Is_Sequence_Number_Newer(sequenceNumber, security->sequenceNumber))
    {
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SeqNum_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        security->sequenceNumber = sequenceNumber;
    }

    // Note: security footer not used for now
    // if (securityFooterEnabled)
    //{
    //    uint16_t securityFooterSize = 0;
    //    status = SOPC_UInt16_Read(&securityFooterSize, buffer, 0);
    //}

    // decrypt Payload
    if (securityEncryptedEnabled && SOPC_STATUS_OK == status)
    {
        uint32_t sizeSignature;
        status = SOPC_PubSub_Security_GetSignSize(security, securitySignedEnabled, &sizeSignature);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_SecuritySignSize_Failed);

        if (SOPC_STATUS_OK == status)
        {
            // New data are Payload and signature.
            uint32_t sizePayload = SOPC_Buffer_Remaining(buffer) - sizeSignature;

            // This data shall be forget after calling Decrypt function
            security->msgNonceRandom = securityMessageNonce;
            // This function decrypt "sizePayload" bytes of the buffer from current position
            // Crypted data are replace by clear data in the buffer
            *buffer_payload = SOPC_PubSub_Security_Decrypt(security, buffer, sizePayload);
            security->msgNonceRandom = NULL;
            if (NULL == *buffer_payload)
            {
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SecurityDecrypt_Failed);
            }
        }
    }
    return code;
}

static inline SOPC_NetworkMessage_Error_Code Decode_Message_V1(
    SOPC_Buffer* buffer,
    uint32_t payload_sign_position,
    SOPC_Dataset_LL_NetworkMessage* nm,
    SOPC_Dataset_LL_NetworkMessage_Header* header,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* readerConf,
    const SOPC_ReaderGroup* group)
{
    SOPC_ASSERT(NULL != header && NULL != nm && NULL != group && NULL != readerConf &&
                NULL != readerConf->callbacks.pGetReader_Func && NULL != readerConf->callbacks.pSetDsm_Func);

    const uint16_t group_id = SOPC_ReaderGroup_Get_GroupId(group);
    const SOPC_DataSetReader** dsmReaders = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;

    // number of DataSetMessage. Should be one
    SOPC_Byte msg_count = 0;
    SOPC_Buffer* buffer_payload = NULL;

    SOPC_UADP_Configuration* conf = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);

    // Payload Header
    // Only DataSetMessage is managed
    if (conf->PayloadHeaderFlag && SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Read(&msg_count, buffer, 0);
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Byte_Failed);
    }
    else if (SOPC_STATUS_OK == status)
    {
        msg_count = 1;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = valid_bool_to_status(SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, msg_count));
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Alloc_Failed);
    }

    if (SOPC_STATUS_OK == status)
    {
        dsmReaders = SOPC_Calloc(msg_count, sizeof(SOPC_DataSetReader*));
        SOPC_ASSERT(NULL != dsmReaders);
    }

    // DataSetMessage Writer Ids (Payload Header)
    if (SOPC_STATUS_OK == status)
    {
        bool mustDecode = false;
        for (int i = 0; i < msg_count && SOPC_STATUS_OK == status; i++)
        {
            SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
            uint16_t writer_id;
            status = SOPC_UInt16_Read(&writer_id, buffer, 0);
            code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_Short_Failed);

            if (SOPC_STATUS_OK == status)
            {
                SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, writer_id);
                dsmReaders[i] = readerConf->callbacks.pGetReader_Func(group, conf, writer_id);
            }

            // Check if there is at last one DSM to read, otherwise decoding can be canceled
            if (dsmReaders[i] != NULL)
            {
                mustDecode = true;
            }
        }
        if (!mustDecode)
        {
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_NoMatchingReader);
        }
    }

    // Timestamp
    if (conf->TimestampFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_Timestamp);
    }

    // Picoseconds
    if (conf->PicoSecondsFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_Picoseconds);
    }

    // Promoted fields
    if (conf->PromotedFieldsFlag && SOPC_STATUS_OK == status)
    {
        // not managed yet
        set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_PromotedFields);
    }

    // Security Header
    if (conf->SecurityFlag && SOPC_STATUS_OK == status)
    {
        code = Decode_SecurityHeader(buffer, &buffer_payload, payload_sign_position, group_id,
                                     readerConf->pGetSecurity_Func, header, conf);

        status = (SOPC_NetworkMessage_Error_Code_None == code) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }
    else
    {
        // check that subscriber expects security mode is none
        if (NULL != readerConf->pGetSecurity_Func && SOPC_STATUS_OK == status) // if NULL, security mode is None
        {
            SOPC_PubSub_SecurityType* security = readerConf->pGetSecurity_Func(
                SOPC_PUBSUB_SKS_DEFAULT_TOKENID,
                Network_Layer_Convert_PublisherId(SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)), group_id);
            // if security is NULL, there is no reader configured with security sign or encrypt/sign
            if (NULL != security && !Network_Check_ReceivedSecurityMode(security->mode, false, false))
            {
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_SecurityNone_Failed);
            }
        }
    }

    // Payload
    if (NULL == buffer_payload && SOPC_STATUS_OK == status)
    {
        buffer_payload = buffer;
    }

    // Store DMS size to check it later
    uint16_t* dsmSizes = NULL;

    // No size if there is only one DataSetMessage
    if (msg_count > 1 && conf->PayloadHeaderFlag && SOPC_STATUS_OK == status)
    {
        dsmSizes = SOPC_Calloc(msg_count, sizeof(uint16_t));
        if (NULL == dsmSizes)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        for (int i = 0; i < msg_count && SOPC_STATUS_OK == status; i++)
        {
            status = SOPC_UInt16_Read(&dsmSizes[i], buffer_payload, 0);
        }
        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_DsmSize_Failed);
    }

    // Decode DataSetMessages

    // Bit 0: DataSetMessage is valid.
    // Bit range 1-2: Field Encoding
    // Bit 3: DataSetMessageSequenceNumber enabled
    // Bit 4: Status enabled
    // Bit 5: ConfigurationVersionMajorVersion enabled
    // Bit 6: ConfigurationVersionMinorVersion enable
    // Bit 7: dataSetFlags2 enabled
    for (int i = 0; i < msg_count && SOPC_STATUS_OK == status; i++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
        const uint16_t size = (NULL == dsmSizes ? 0 : dsmSizes[i]);
        const SOPC_DataSetReader* reader = dsmReaders[i];
        if (NULL != reader)
        {
            SOPC_Conf_PublisherId pubId =
                Network_Layer_Convert_PublisherId(SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header));
            code = decode_dataSetMessage(dsm, buffer_payload, pubId, group_id, size, readerConf);
            status = (SOPC_NetworkMessage_Error_Code_None == code) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

            const uint16_t writerId = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(dsm);
            SOPC_TargetVariableCtx* targetVariable = NULL;
            // If function to retrieve targetVariable is not set create it dynamically and delete it.
            bool clearTargetVariable = true;
            if (SOPC_STATUS_OK == status)
            {
                const SOPC_DataSet_LL_UadpDataSetMessageContentMask* dsm_conf =
                    SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(dsm);
                if (DataSet_LL_MessageType_KeepAlive != dsm_conf->dataSetMessageType)
                {
                    if (NULL != readerConf->targetVariable_Func)
                    {
                        targetVariable = readerConf->targetVariable_Func(&pubId, group_id, writerId);
                        status = (targetVariable != NULL) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                        code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_BadMetaData);
                        clearTargetVariable = false;
                    }
                    else
                    {
                        targetVariable = SOPC_SubTargetVariable_TargetVariablesCtx_Create(reader);
                    }
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                status = readerConf->callbacks.pSetDsm_Func(dsm, readerConf->targetConfig, reader, targetVariable);
                code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_BadMetaData);
            }
            if (SOPC_STATUS_OK == status && NULL != readerConf->updateTimeout_Func)
            {
                readerConf->updateTimeout_Func(&pubId, group_id, writerId);
            }
            if (NULL != targetVariable && clearTargetVariable)
            {
                SOPC_SubTargetVariable_TargetVariableCtx_Delete(&targetVariable);
            }
        }
        else
        {
            // Message has no matching reader, simply skip the DSM using its given size
            if (dsmSizes == 0)
            {
                set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_DsmSizeCheck_Failed);
            }
            else
            {
                SOPC_Buffer_Read(NULL, buffer_payload, size);
                code = checkAndGetErrorCode(status, SOPC_UADP_NetworkMessage_Error_Read_DsmSkip_Failed);
            }
        }
    }

    if (NULL != dsmSizes)
    {
        SOPC_Free(dsmSizes);
    }

    if (NULL != dsmReaders)
    {
        SOPC_Free(dsmReaders);
    }

    // delete the Payload if it has been decrypted
    if (NULL != buffer_payload && buffer != buffer_payload)
    {
        SOPC_Buffer_Delete(buffer_payload);
    }

    return code;
}

SOPC_NetworkMessage_Error_Code SOPC_UADP_NetworkMessage_Decode(
    SOPC_Buffer* buffer,
    const SOPC_UADP_NetworkMessage_Reader_Configuration* reader_config,
    const SOPC_PubSubConnection* connection,
    SOPC_UADP_NetworkMessage** uadp_nm)
{
    if (NULL == uadp_nm || NULL != *uadp_nm || NULL == buffer || NULL == reader_config || NULL == connection ||
        NULL == reader_config->callbacks.pGetGroup_Func)
    {
        return SOPC_NetworkMessage_Error_Code_InvalidParameters;
    }
    const uint32_t payload_sign_position = buffer->position;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    const SOPC_ReaderGroup* group = NULL;
    SOPC_Dataset_LL_NetworkMessage* nm = NULL;
    SOPC_Dataset_LL_NetworkMessage_Header* header = NULL;
    SOPC_UADP_Configuration* conf = NULL;

    *uadp_nm = SOPC_Network_Message_Create();

    if (NULL == *uadp_nm)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        code = SOPC_UADP_NetworkMessage_Error_Read_Alloc_Failed;
    }

    if (SOPC_STATUS_OK == status)
    {
        nm = (*uadp_nm)->nm;
        header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
        conf = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);
        SOPC_ASSERT(NULL != header && NULL != conf);

        // Decode Message Header, and determine message version
        code = SOPC_UADP_NetworkMessageHeader_Decode(buffer, header);
        status = (SOPC_NetworkMessage_Error_Code_None == code) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Decode GroupHeader
    if (SOPC_STATUS_OK == status)
    {
        code = Decode_GroupHeader(buffer, nm, header, conf);
        status = (SOPC_NetworkMessage_Error_Code_None == code) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Use caller callback to identify the reader matching the received Header
    if (SOPC_STATUS_OK == status)
    {
        const uint32_t rcvGroupVersion = SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm);
        const uint32_t rcvGroupId = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm);
        const SOPC_Dataset_LL_PublisherId* pubid = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
        group = reader_config->callbacks.pGetGroup_Func(connection, conf, pubid, rcvGroupVersion, rcvGroupId);

        if (group == NULL)
        {
            // If a reader is not found, stop decoding
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup);
        }
    }

    // Decode the message content (after GroupHeader), depending on message version
    if (SOPC_STATUS_OK == status)
    {
        const uint8_t version = SOPC_Dataset_LL_NetworkMessage_GetVersion(header);
        switch (version)
        {
        case UADP_VERSION1:
            code = Decode_Message_V1(buffer, payload_sign_position, nm, header, reader_config, group);
            status = (SOPC_NetworkMessage_Error_Code_None == code) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            break;
        default:
            set_status_default(&status, &code, SOPC_UADP_NetworkMessage_Error_Unsupported_Version);
            break;
        }
    }

    // Free memory in case of decoding failure
    if (SOPC_STATUS_OK != status)
    {
        SOPC_ASSERT(SOPC_NetworkMessage_Error_Code_None != code);
        SOPC_UADP_NetworkMessage_Delete(*uadp_nm);
        *uadp_nm = NULL;
    }

    return code;
}

void SOPC_UADP_NetworkMessage_Delete(SOPC_UADP_NetworkMessage* uadp_nm)
{
    if (NULL != uadp_nm)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(uadp_nm->nm);
        SOPC_Free(uadp_nm);
    }
}

static SOPC_Conf_PublisherId Network_Layer_Convert_PublisherId(const SOPC_Dataset_LL_PublisherId* src)
{
    SOPC_Conf_PublisherId result;
    if (NULL == src)
    {
        static const SOPC_Conf_PublisherId nullPubId = {.type = SOPC_Null_PublisherId};
        return nullPubId;
    }

    switch (src->type)
    {
    case DataSet_LL_PubId_Byte_Id:
    {
        result.type = SOPC_UInteger_PublisherId;
        result.data.uint = src->data.byte;
        break;
    }
    case DataSet_LL_PubId_UInt16_Id:
    {
        result.type = SOPC_UInteger_PublisherId;
        result.data.uint = src->data.uint16;
        break;
    }
    case DataSet_LL_PubId_UInt32_Id:
    {
        result.type = SOPC_UInteger_PublisherId;
        result.data.uint = src->data.uint32;
        break;
    }
    case DataSet_LL_PubId_UInt64_Id:
    {
        result.type = SOPC_UInteger_PublisherId;
        result.data.uint = src->data.uint64;
        break;
    }
    case DataSet_LL_PubId_String_Id:
    {
        result.type = SOPC_String_PublisherId;
        result.data.string = src->data.string;
        break;
    }
    default:
        result.type = SOPC_Null_PublisherId;
    }
    return result;
}

static bool Network_Check_ReceivedSecurityMode(SOPC_SecurityMode_Type mode, bool ssigned, bool encrypted)
{
    switch (mode)
    {
    case SOPC_SecurityMode_Invalid:
        return false;
    case SOPC_SecurityMode_None:
        return !ssigned && !encrypted;
    case SOPC_SecurityMode_Sign:
        return ssigned && !encrypted;
    case SOPC_SecurityMode_SignAndEncrypt:
        return ssigned && encrypted;
    default:
        return false;
    }
}
