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
#include "sopc_dataset_layer.h"
#include "sopc_dataset_ll_layer.h"

// TODO: Most of those flags should be configurable in the future (once encoder supports all cases)
const uint8_t DATASET_DSM_ENCODING_TYPE = 0;          // Only 0 supported
const bool DATASET_DSM_IS_VALID = true;               // Only true supported
const bool DATASET_DSM_SEQ_NUMBER_ENABLED = true;     // Only true supported
const bool DATASET_DSM_STATUS_ENABLED = false;        // Only false supported
const bool DATASET_DSM_MAJOR_VERSION_ENABLED = false; // Only false supported
const bool DATASET_DSM_MINOR_VERSION_ENABLED = false; // Only false supported
const bool DATASET_DSM_CLASSID_ENABLED = false;       // Only false supported

const bool DATASET_DSM_TIMESTAMP_ENABLED = false;    // Only false supported
const bool DATASET_DSM_PICOSECONDS_INCLUDED = false; // Only false supported

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
        SOPC_UadpDataSetMessageContentMask conf;
        memset(&conf, 0, sizeof(conf));
        conf.NotValidFlag = !DATASET_DSM_IS_VALID;
        conf.FieldEncoding = DATASET_DSM_ENCODING_TYPE;
        conf.DataSetMessageSequenceNumberFlag = DATASET_DSM_SEQ_NUMBER_ENABLED;
        conf.StatusFlag = DATASET_DSM_STATUS_ENABLED;
        conf.ConfigurationVersionMajorVersionFlag = DATASET_DSM_MAJOR_VERSION_ENABLED;
        conf.ConfigurationVersionMinorFlag = DATASET_DSM_MINOR_VERSION_ENABLED;
        conf.DataSetFlags2 = false;                                // by default depends on source type
        conf.DataSetMessageType = DataSet_LL_MessageType_KeyFrame; // by default depends on source type
        conf.TimestampFlag = DATASET_DSM_TIMESTAMP_ENABLED;
        conf.PicoSecondsFlag = DATASET_DSM_PICOSECONDS_INCLUDED;
        switch (SOPC_PublishedDataSet_Get_DataSet_SourceType(SOPC_DataSetWriter_Get_DataSet(conf_dsw)))
        {
        case SOPC_PublishedDataItemsDataType:
            SOPC_ASSERT(!isKeepAlive);
            break;
        case SOPC_PublishedEventsDataType:
            conf.DataSetFlags2 = true;
            if (isKeepAlive)
            {
                conf.DataSetMessageType = DataSet_LL_MessageType_KeepAlive;
            }
            else
            {
                conf.DataSetMessageType = DataSet_LL_MessageType_Event;
            }
            break;
        case SOPC_PublishedDataSetCustomSourceDataType:
            conf.DataSetFlags2 = true;
            if (isKeepAlive)
            {
                conf.DataSetMessageType = DataSet_LL_MessageType_KeepAlive;
            }
            else
            {
                conf.DataSetMessageType = DataSet_LL_MessageType_Event;
            }
            break;
        default:
            SOPC_ASSERT(false);
        }
        SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(msg_dsm, conf);
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
                                        SOPC_FieldMetaData* metadata)
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
    // String not managed
    uint64_t conf_id = conf_pubid->data.uint;
    SOPC_ASSERT(SOPC_UInteger_PublisherId == conf_pubid->type);
    if (UINT32_MAX < conf_id)
    {
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt64(nmh, conf_id);
    }
    else if (UINT16_MAX < conf_id)
    {
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(nmh, (uint32_t) conf_id);
    }
    else if (UINT8_MAX < conf_id)
    {
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt16(nmh, (uint16_t) conf_id);
    }
    else
    {
        SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nmh, (uint8_t) conf_id);
    }
}
