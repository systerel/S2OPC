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

#ifndef SOPC_DATASET_LL_LAYER_H_
#define SOPC_DATASET_LL_LAYER_H_

#include <stdbool.h>

#include "sopc_builtintypes.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_security.h"

#define UADP_VERSION1 1
#define UADP_DEFAULT_VERSION UADP_VERSION1

typedef struct SOPC_Dataset_LL_DataSetMessage SOPC_Dataset_LL_DataSetMessage;

typedef struct SOPC_Dataset_LL_NetworkMessage SOPC_Dataset_LL_NetworkMessage;
typedef struct SOPC_Dataset_LL_NetworkMessage_Header SOPC_Dataset_LL_NetworkMessage_Header;
typedef struct SOPC_Dataset_LL_DataSetField SOPC_Dataset_LL_DataSetField;

typedef enum SOPC_DataSet_LL_PublisherIdType
{
    DataSet_LL_PubId_Byte_Id = 0,
    DataSet_LL_PubId_UInt16_Id = 1,
    DataSet_LL_PubId_UInt32_Id = 2,
    DataSet_LL_PubId_UInt64_Id = 3,
    DataSet_LL_PubId_String_Id = 4
} SOPC_DataSet_LL_PublisherIdType;

typedef struct SOPC_Dataset_LL_PublisherId
{
    SOPC_DataSet_LL_PublisherIdType type;
    union
    {
        SOPC_Byte byte;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
        SOPC_String string;
    } data;
} SOPC_Dataset_LL_PublisherId;

typedef enum SOPC_DataSet_LL_DataSetMessageType
{
    DataSet_LL_MessageType_KeyFrame = 0x00,
    DataSet_LL_MessageType_DeltaFrame = 0x01,
    DataSet_LL_MessageType_Event = 0x02,
    DataSet_LL_MessageType_KeepAlive = 0x03
} SOPC_DataSet_LL_DataSetMessageType;

typedef enum SOPC_DataSet_LL_FieldEncoding
{
    DataSet_LL_FieldEncoding_Variant = 0,
    DataSet_LL_FieldEncoding_RawData = 1,
    DataSet_LL_FieldEncoding_DataValue = 2
} SOPC_DataSet_LL_FieldEncoding;

typedef struct SOPC_DataSet_LL_UadpDataSetMessageContentMask
{
    bool validFlag;
    SOPC_DataSet_LL_FieldEncoding fieldEncoding;
    bool dataSetMessageSequenceNumberFlag;
    bool statusFlag;
    bool configurationVersionMajorVersionFlag;
    bool configurationVersionMinorFlag;
    SOPC_DataSet_LL_DataSetMessageType dataSetMessageType;
    bool timestampFlag;
    bool picoSecondsFlag;
} SOPC_DataSet_LL_UadpDataSetMessageContentMask;

struct SOPC_PubFixedBuffer_Buffer_Ctx;

/**
 * Header NetworkMessage
 */

/**
 * \brief Create a NetworkMessage
 *
 * \param dsm_nb  Number of DataSetMessage to allocate
 * \param uadp_version  Version of UADP message
 *
 * \return Dataset_LL_NetworkMessage
 */
SOPC_Dataset_LL_NetworkMessage* SOPC_Dataset_LL_NetworkMessage_Create(uint8_t dsm_nb, uint8_t uadp_version);

bool SOPC_DataSet_LL_NetworkMessage_is_Preencode_Buffer_Enabled(SOPC_Dataset_LL_NetworkMessage* nm);

void SOPC_DataSet_LL_NetworkMessage_Set_Preencode_Buffer(SOPC_Dataset_LL_NetworkMessage* nm,
                                                         struct SOPC_PubFixedBuffer_Buffer_Ctx* preencode);
struct SOPC_PubFixedBuffer_Buffer_Ctx* SOPC_DataSet_LL_NetworkMessage_Get_Preencode_Buffer(
    SOPC_Dataset_LL_NetworkMessage* nm);

// todo change interface to create nm without dsm
SOPC_Dataset_LL_NetworkMessage* SOPC_Dataset_LL_NetworkMessage_CreateEmpty(void);

/** \brief returns the Header of a network message */
SOPC_Dataset_LL_NetworkMessage_Header* SOPC_Dataset_LL_NetworkMessage_GetHeader(SOPC_Dataset_LL_NetworkMessage* nm);
const SOPC_Dataset_LL_NetworkMessage_Header* SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(
    const SOPC_Dataset_LL_NetworkMessage* nm);

/** \brief returns the Configuration of a network message */
SOPC_UADP_Configuration* SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(SOPC_Dataset_LL_NetworkMessage_Header* nmh);
const SOPC_UADP_Configuration* SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig_Const(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh);

/**
 * Allocate memory for the internal dataset messages array of a Network Message
 */
bool SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(SOPC_Dataset_LL_NetworkMessage* nm, uint8_t dsm_nb);

/**
 * \brief Free the given networkMessage and its attibutes
 */
void SOPC_Dataset_LL_NetworkMessage_Delete(SOPC_Dataset_LL_NetworkMessage* nm);

uint8_t SOPC_Dataset_LL_NetworkMessage_GetVersion(const SOPC_Dataset_LL_NetworkMessage_Header* nmh);

// Only for decoding: precondition: version <= 15 (4 bits)
void SOPC_Dataset_LL_NetworkMessage_SetVersion(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint8_t version);

uint8_t SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(SOPC_Dataset_LL_NetworkMessage* nm);

SOPC_Dataset_LL_DataSetMessage* SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                                 int index);

/* PUBLISHER ID */
// publisher id type
SOPC_DataSet_LL_PublisherIdType SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh);
// publisher id
/* Return address of the internal publisher id type
 * User shall not delete data
 */
const SOPC_Dataset_LL_PublisherId* SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh);

void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_Byte id);
void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint16_t id);
void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint32_t id);
void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint64_t id);
void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_String(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_String id);
/* WRITER GROUP ID */
void SOPC_Dataset_LL_NetworkMessage_Set_GroupId(SOPC_Dataset_LL_NetworkMessage* nm, uint16_t id);
uint16_t SOPC_Dataset_LL_NetworkMessage_Get_GroupId(const SOPC_Dataset_LL_NetworkMessage* nm);
/* WRITER GROUP VERSION */
void SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(SOPC_Dataset_LL_NetworkMessage* nm, uint32_t version);
uint32_t SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(const SOPC_Dataset_LL_NetworkMessage* nm);

/**
 * Header DataSetMessage
 */

bool SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t dsf_nb);

/**
 * Free dataset fields array
 */
void SOPC_Dataset_LL_DataSetMsg_Delete_DataSetField_Array(SOPC_Dataset_LL_DataSetMessage* dsm);

uint16_t SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(const SOPC_Dataset_LL_DataSetMessage* dsm);

// dataset writer id
void SOPC_Dataset_LL_DataSetMsg_Set_WriterId(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t id);

// dataset writer id
uint16_t SOPC_Dataset_LL_DataSetMsg_Get_WriterId(const SOPC_Dataset_LL_DataSetMessage* dsm);

// Set UADP conf associated to the DSM
void SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(SOPC_Dataset_LL_DataSetMessage* dsm,
                                                const SOPC_DataSet_LL_UadpDataSetMessageContentMask* conf);

// Get UADP conf associated to the DSM
const SOPC_DataSet_LL_UadpDataSetMessageContentMask* SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(
    const SOPC_Dataset_LL_DataSetMessage* dsm);

// dataset message type (default is Key Frame without DataSetFlags2 content mask)
SOPC_DataSet_LL_DataSetMessageType SOPC_Dataset_LL_DataSetMsg_Get_MessageType(
    const SOPC_Dataset_LL_DataSetMessage* dsm);

// dataset message timestamp
uint64_t SOPC_Dataset_LL_DataSetMsg_Get_Timestamp(const SOPC_Dataset_LL_DataSetMessage* dsm);
void SOPC_Dataset_LL_DataSetMsg_Set_Timestamp(SOPC_Dataset_LL_DataSetMessage* dsm, uint64_t timestamp);
const uint64_t* SOPC_Dataset_LL_DataSetMsg_Get_TimestampPointer(const SOPC_Dataset_LL_DataSetMessage* dsm);

// dataset message sequence number
void SOPC_Dataset_LL_DataSetMsg_Set_SequenceNumber(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t sn);
uint16_t SOPC_Dataset_LL_DataSetMsg_Get_SequenceNumber(const SOPC_Dataset_LL_DataSetMessage* dsm);
const uint16_t* SOPC_Dataset_LL_DataSetMsg_Get_SequenceNumberPointer(const SOPC_Dataset_LL_DataSetMessage* dsm);
/**
 * Header DataSetField
 */

/**
 * Set the variant of a dataset field
 * The dataset field is now the ownership of the variant.
 * Deleting the given dataset field will delete this variant too.
 *
 * Note: previous variant is freed if existing
 *
 * \return true if succeeded, false otherwise
 */
bool SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(SOPC_Dataset_LL_DataSetMessage* dsm,
                                                            SOPC_Variant* variant,
                                                            uint16_t index);

/**
 * Get the variant of a dataset field
 * This variant shall not be removed.
 */
const SOPC_Variant* SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(const SOPC_Dataset_LL_DataSetMessage* dsm,
                                                                   uint16_t index);
SOPC_Variant* SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t index);
const SOPC_Dataset_LL_DataSetField* SOPC_Dataset_LL_DataSetMsg_Get_ConstDataSetField_At(
    const SOPC_Dataset_LL_DataSetMessage* dsm,
    uint16_t index);
SOPC_Dataset_LL_DataSetField* SOPC_Dataset_LL_DataSetMsg_Get_DataSetField_At(SOPC_Dataset_LL_DataSetMessage* dsm,
                                                                             uint16_t index);

const SOPC_Variant* SOPC_Dataset_LL_DataSetField_Get_Variant(const SOPC_Dataset_LL_DataSetField* dsf);

#endif /* SOPC_DATASET_LL_LAYER_H_ */
