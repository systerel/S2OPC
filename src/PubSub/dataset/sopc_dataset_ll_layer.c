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
#include <string.h>

#include "sopc_mem_alloc.h"
#include "sopc_pubsub_conf.h"

#include "sopc_dataset_ll_layer.h"

struct SOPC_Dataset_LL_DataSetField
{
    SOPC_Variant* variant;
};

typedef struct SOPC_Dataset_LL_DataSetField SOPC_Dataset_LL_DataSetField;

struct SOPC_Dataset_LL_DataSetMessage
{
    SOPC_Dataset_LL_DataSetField* dataset_fields;
    uint16_t dataset_fields_length;
    uint16_t dataset_writer_id;
};

typedef struct Dataset_LL_Group
{
    uint16_t id;
    uint32_t version;
} Dataset_LL_Group;

// UADP Comment network Header
struct SOPC_Dataset_LL_NetworkMessage_Header
{
    uint8_t UADP_version;
    SOPC_UADP_Configuration flagsConfig;
    int networkMessage_type;
    SOPC_Dataset_LL_PublisherId publisher_id;
    // dataset_classId not managed
};

struct SOPC_Dataset_LL_NetworkMessage
{
    // network_message_header
    SOPC_Dataset_LL_NetworkMessage_Header msgHeader;

    // only DataSetMessage payload is managed

    Dataset_LL_Group group;

    // group_header

    // payload_header

    // extended_network_message_header

    /* security_header
       not implemented
     */

    SOPC_Dataset_LL_DataSetMessage* dataset_messages;
    uint8_t dataset_messages_length;
};

/**
 * Private
 * Free attributes of a given field
 */
void Dataset_LL_Delete_DataSetFieldAttributes(SOPC_Dataset_LL_DataSetField* field);

/**
 * Private
 * Free dataset message array
 */
void Dataset_LL_Delete_DataSetMessages_Array(SOPC_Dataset_LL_NetworkMessage* nm);

/**
 * BODY NetworkMessage
 */

SOPC_Dataset_LL_NetworkMessage* SOPC_Dataset_LL_NetworkMessage_Create(uint8_t dsm_nb, uint8_t uadp_version)
{
    struct SOPC_Dataset_LL_NetworkMessage* result = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    if (NULL == result)
    {
        return NULL;
    }
    result->msgHeader.UADP_version = uadp_version;
    bool status = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(result, dsm_nb);
    if (!status)
    {
        SOPC_Free(result);
        return NULL;
    }
    return result;
}

SOPC_Dataset_LL_NetworkMessage* SOPC_Dataset_LL_NetworkMessage_CreateEmpty(void)
{
    struct SOPC_Dataset_LL_NetworkMessage* result = SOPC_Calloc(1, sizeof(SOPC_Dataset_LL_NetworkMessage));

    memset(&result->msgHeader, 0, sizeof(result->msgHeader));

    // publisher id type is int 32
    result->msgHeader.flagsConfig.PublisherIdFlag = true;
    result->msgHeader.publisher_id.data.byte = 0;
    result->msgHeader.publisher_id.type = DataSet_LL_PubId_Byte_Id;

    // only DataSetMessage payload is managed
    result->msgHeader.networkMessage_type = 0;

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(result, 0);

    return result;
}

void SOPC_Dataset_LL_NetworkMessage_Delete(SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return;
    }
    Dataset_LL_Delete_DataSetMessages_Array(nm);
    SOPC_Free(nm);
}

uint8_t SOPC_Dataset_LL_NetworkMessage_GetVersion(const SOPC_Dataset_LL_NetworkMessage_Header* nmh)
{
    if (NULL == nmh)
    {
        return 0;
    }
    return nmh->UADP_version;
}

void SOPC_Dataset_LL_NetworkMessage_SetVersion(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint8_t version)
{
    assert(version <= 15); // Encoded on 4 bits
    if (NULL != nmh)
    {
        nmh->UADP_version = version;
    }
}

uint8_t SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return 0;
    }
    return nm->dataset_messages_length;
}

SOPC_Dataset_LL_DataSetMessage* SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(SOPC_Dataset_LL_NetworkMessage* nm,
                                                                                 int index)
{
    if (NULL == nm || index < 0 || index >= nm->dataset_messages_length)
    {
        return NULL;
    }
    return nm->dataset_messages + index;
}

void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_Byte id)
{
    if (NULL != nmh)
    {
        nmh->publisher_id.type = DataSet_LL_PubId_Byte_Id;
        nmh->publisher_id.data.byte = id;
        nmh->flagsConfig.PublisherIdFlag = true;
    }
}

void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint16_t id)
{
    if (NULL != nmh)
    {
        nmh->publisher_id.type = DataSet_LL_PubId_UInt16_Id;
        nmh->publisher_id.data.uint16 = id;
        nmh->flagsConfig.PublisherIdFlag = true;
    }
}

void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint32_t id)
{
    if (NULL != nmh)
    {
        nmh->publisher_id.type = DataSet_LL_PubId_UInt32_Id;
        nmh->publisher_id.data.uint32 = id;
        nmh->flagsConfig.PublisherIdFlag = true;
    }
}

void SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(SOPC_Dataset_LL_NetworkMessage_Header* nmh, uint64_t id)
{
    if (NULL != nmh)
    {
        nmh->publisher_id.type = DataSet_LL_PubId_UInt64_Id;
        nmh->publisher_id.data.uint64 = id;
        nmh->flagsConfig.PublisherIdFlag = true;
    }
}

const SOPC_Dataset_LL_PublisherId* SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh)
{
    if (NULL == nmh || !nmh->flagsConfig.PublisherIdFlag)
    {
        return NULL;
    }
    return &(nmh->publisher_id);
}

/**
 * Hyp: only uint32 is managed
 */
SOPC_DataSet_LL_PublisherIdType SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh)
{
    return nmh->publisher_id.type;
}

/**
 * BODY DataSetMessage
 */
bool SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t dsf_nb)
{
    assert(NULL != dsm);
    if (dsf_nb > 0)
    {
        dsm->dataset_fields = SOPC_Calloc(dsf_nb, sizeof(SOPC_Dataset_LL_DataSetField));
        if (NULL == dsm->dataset_fields)
        {
            return false;
        }
    }
    dsm->dataset_fields_length = dsf_nb;
    return true;
}

uint16_t SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    if (NULL == dsm)
    {
        return 0;
    }
    return dsm->dataset_fields_length;
}

static const SOPC_Dataset_LL_DataSetField* SOPC_Dataset_LL_DataSetMsg_Get_DataSetField_At(
    const SOPC_Dataset_LL_DataSetMessage* dsm,
    uint16_t index)
{
    if (NULL == dsm || index >= dsm->dataset_fields_length)
    {
        return NULL;
    }
    return &dsm->dataset_fields[index];
}

void SOPC_Dataset_LL_DataSetMsg_Set_WriterId(SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t id)
{
    if (NULL != dsm)
    {
        dsm->dataset_writer_id = id;
    }
}

uint16_t SOPC_Dataset_LL_DataSetMsg_Get_WriterId(const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    if (NULL == dsm)
    {
        return 0;
    }
    return dsm->dataset_writer_id;
}

/**
 * BODY DataSetField
 */

bool SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(SOPC_Dataset_LL_DataSetMessage* dsm,
                                                            SOPC_Variant* variant,
                                                            uint16_t index)
{
    if (NULL == dsm || index >= dsm->dataset_fields_length || NULL == variant)
    {
        return false;
    }
    else
    {
        SOPC_Variant_Delete(dsm->dataset_fields[index].variant);
    }
    dsm->dataset_fields[index].variant = variant;
    return true;
}

const SOPC_Variant* SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(const SOPC_Dataset_LL_DataSetMessage* dsm, uint16_t index)
{
    if (NULL == dsm)
    {
        return NULL;
    }
    const SOPC_Dataset_LL_DataSetField* dsf = SOPC_Dataset_LL_DataSetMsg_Get_DataSetField_At(dsm, index);
    if (NULL == dsf)
    {
        return NULL;
    }
    return dsf->variant;
}

/**
 * Body Group
 */

void SOPC_Dataset_LL_NetworkMessage_Set_GroupId(SOPC_Dataset_LL_NetworkMessage* nm, uint16_t id)
{
    if (NULL != nm)
    {
        nm->group.id = id;
    }
}

uint16_t SOPC_Dataset_LL_NetworkMessage_Get_GroupId(const SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return 0;
    }
    return nm->group.id;
}

void SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(SOPC_Dataset_LL_NetworkMessage* nm, uint32_t version)
{
    if (NULL != nm)
    {
        nm->group.version = version;
    }
}

uint32_t SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(const SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return 0;
    }
    return nm->group.version;
}

void SOPC_Dataset_LL_DataSetMsg_Delete_DataSetField_Array(SOPC_Dataset_LL_DataSetMessage* dsm)
{
    if (NULL != dsm && NULL != dsm->dataset_fields)
    {
        for (int i = 0; i < dsm->dataset_fields_length; i++)
        {
            Dataset_LL_Delete_DataSetFieldAttributes(&dsm->dataset_fields[i]);
        }
        dsm->dataset_fields_length = 0;
        SOPC_Free(dsm->dataset_fields);
        dsm->dataset_fields = NULL;
    }
}

/**
 * Private
 */

void Dataset_LL_Delete_DataSetMessages_Array(SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL != nm && NULL != nm->dataset_messages)
    {
        for (int i = 0; i < nm->dataset_messages_length; i++)
        {
            SOPC_Dataset_LL_DataSetMessage* message = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
            SOPC_Dataset_LL_DataSetMsg_Delete_DataSetField_Array(message);
        }
        nm->dataset_messages_length = 0;
        SOPC_Free(nm->dataset_messages);
        nm->dataset_messages = NULL;
    }
}

void Dataset_LL_Delete_DataSetFieldAttributes(SOPC_Dataset_LL_DataSetField* field)
{
    if (NULL != field && NULL != field->variant)
    {
        SOPC_Variant_Delete(field->variant);
        field->variant = NULL;
    }
}

const SOPC_UADP_Configuration* SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig_Const(
    const SOPC_Dataset_LL_NetworkMessage_Header* nmh)
{
    if (NULL == nmh)
    {
        return NULL;
    }
    return &nmh->flagsConfig;
}

SOPC_UADP_Configuration* SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(SOPC_Dataset_LL_NetworkMessage_Header* nmh)
{
    if (NULL == nmh)
    {
        return NULL;
    }
    return &nmh->flagsConfig;
}

const SOPC_Dataset_LL_NetworkMessage_Header* SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(
    const SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return NULL;
    }
    return &nm->msgHeader;
}

SOPC_Dataset_LL_NetworkMessage_Header* SOPC_Dataset_LL_NetworkMessage_GetHeader(SOPC_Dataset_LL_NetworkMessage* nm)
{
    if (NULL == nm)
    {
        return NULL;
    }
    return &nm->msgHeader;
}

bool SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(SOPC_Dataset_LL_NetworkMessage* nm, uint8_t dsm_nb)
{
    assert(NULL != nm);
    nm->dataset_messages = SOPC_Calloc(dsm_nb, sizeof(SOPC_Dataset_LL_DataSetMessage));
    if (NULL == nm->dataset_messages)
    {
        return false;
    }
    nm->dataset_messages_length = dsm_nb;
    return true;
}
