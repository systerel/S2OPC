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

#include "sopc_dataset_layer.h"

#include "sopc_assert.h"
#include "sopc_dataset_ll_layer.h"

static const SOPC_DataSet_LL_UadpDataSetMessageContentMask default_Uapd_DSM_Mask = {
    .validFlag = true,
    .fieldEncoding = DataSet_LL_FieldEncoding_Variant,
    .dataSetMessageSequenceNumberFlag = true,
    .statusFlag = false,
    .configurationVersionMajorVersionFlag = false,
    .configurationVersionMinorFlag = false,
    .dataSetMessageType = DataSet_LL_MessageType_KeyFrame,
    .timestampFlag = false,
    .picoSecondsFlag = false};

static void SOPC_NetworkMessage_Set_PublisherId(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_WriterGroup* group);

SOPC_Dataset_NetworkMessage* SOPC_Create_NetworkMessage_From_WriterGroup(SOPC_WriterGroup* group, bool isKeepAlive)
{
    SOPC_ASSERT(NULL != group);
    uint8_t nb_dataSetWriter = SOPC_WriterGroup_Nb_DataSetWriter(group);

    // TODO :replace by a configurable value through writer group
    SOPC_Dataset_LL_NetworkMessage* msg_nm =
        SOPC_Dataset_LL_NetworkMessage_Create(nb_dataSetWriter, UADP_DEFAULT_VERSION);
    if (NULL == msg_nm)
    {
        return NULL;
    }
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(msg_nm);
    if (NULL == header)
    {
        return NULL;
    }

    // Define DataSetMessage flags 1 & 2 values

    // UADP version is already set to default one

    SOPC_NetworkMessage_Set_PublisherId(header, group);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(msg_nm, SOPC_WriterGroup_Get_Id(group));
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(msg_nm, SOPC_WriterGroup_Get_Version(group));

    const uint8_t nbDataSet = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(msg_nm);
    for (uint8_t iDataSet = 0; iDataSet < nbDataSet; iDataSet++)
    {
        SOPC_DataSetWriter* conf_dsw = SOPC_WriterGroup_Get_DataSetWriter_At(group, iDataSet);
        SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(msg_nm, iDataSet);
        SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, SOPC_DataSetWriter_Get_Id(conf_dsw));
        SOPC_DataSet_LL_UadpDataSetMessageContentMask conf = default_Uapd_DSM_Mask;

        SOPC_PublishedDataSetSourceType sourceType =
            SOPC_PublishedDataSet_Get_DataSet_SourceType(SOPC_DataSetWriter_Get_DataSet(conf_dsw));
        if (SOPC_PublishedDataItemsDataType == sourceType)
        {
            SOPC_ASSERT(!isKeepAlive &&
                        "Source type for acyclic messages shall be SOPC_PublishedDataSetCustomSourceDataType or "
                        "SOPC_PublishedEventsDataType");
        }
        else
        {
            if (isKeepAlive)
            {
                conf.dataSetMessageType = DataSet_LL_MessageType_KeepAlive;
            }
            else
            {
                conf.dataSetMessageType = DataSet_LL_MessageType_Event;
            }
        }

        const SOPC_DataSetWriter_Options* option = SOPC_DataSetWriter_Get_Options(conf_dsw);
        SOPC_ASSERT(NULL != option);
        conf.dataSetMessageSequenceNumberFlag = !option->noUseSeqNum;
        conf.timestampFlag = !option->noTimestamp;

        SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(msg_dsm, &conf);

        SOPC_Dataset_LL_DataSetMsg_Set_EnableEmission(msg_dsm, !option->disableEmission);

        if (!isKeepAlive)
        {
            const uint16_t nbFields = SOPC_PublishedDataSet_Nb_FieldMetaData(SOPC_DataSetWriter_Get_DataSet(conf_dsw));

            const bool status = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, nbFields);
            if (!status)
            {
                SOPC_Dataset_LL_NetworkMessage_Delete(msg_nm);
                return NULL;
            }
        }
    }
    return msg_nm;
}

void SOPC_Delete_NetworkMessage(SOPC_Dataset_NetworkMessage* nm)
{
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}

void SOPC_NetworkMessage_Set_Variant_At(SOPC_Dataset_NetworkMessage* nm,
                                        uint8_t dsm_index,
                                        uint16_t dsf_index,
                                        SOPC_Variant* variant,
                                        const SOPC_FieldMetaData* metadata)
{
    SOPC_ASSERT(NULL != nm && NULL != variant && NULL != metadata);
    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, dsm_index);
    // checks bad index
    SOPC_ASSERT(NULL != dsm);

    bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, dsf_index);
    SOPC_ASSERT(res); // valid index
}

// private
static void SOPC_NetworkMessage_Set_PublisherId(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_WriterGroup* group)
{
    const SOPC_PubSubConnection* conf_connection = SOPC_WriterGroup_Get_Connection(group);
    const SOPC_Conf_PublisherId* conf_pubid = SOPC_PubSubConnection_Get_PublisherId(conf_connection);
    if (SOPC_UInteger_PublisherId == conf_pubid->type)
    {
        uint64_t pubid = conf_pubid->data.uint;
        if (UINT32_MAX < pubid)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(nmh, pubid);
        }
        else if (UINT16_MAX < pubid)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(nmh, (uint32_t) pubid);
        }
        else if (UINT8_MAX < pubid)
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(nmh, (uint16_t) pubid);
        }
        else
        {
            SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nmh, (uint8_t) pubid);
        }
    }
    else
    {
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_String(nmh, conf_pubid->data.string);
    }
}
