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

#include "sopc_dataset_layer.h"
#include "sopc_dataset_ll_layer.h"

static void SOPC_NetworkMessage_Set_PublisherId(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_WriterGroup* group);

SOPC_Dataset_NetworkMessage* SOPC_Create_NetworkMessage_From_WriterGroup(SOPC_WriterGroup* group)
{
    assert(NULL != group);
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

        const uint16_t nbFields = SOPC_PublishedDataSet_Nb_FieldMetaData(SOPC_DataSetWriter_Get_DataSet(conf_dsw));

        const bool status = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, nbFields);
        if (!status)
        {
            SOPC_Dataset_LL_NetworkMessage_Delete(msg_nm);
            return NULL;
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
    assert(NULL != nm && NULL != variant && NULL != metadata);
    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, dsm_index);
    // checks bad index
    assert(NULL != dsm);

    bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, dsf_index);
    assert(res); // valid index
}

// private
static void SOPC_NetworkMessage_Set_PublisherId(SOPC_Dataset_LL_NetworkMessage_Header* nmh, SOPC_WriterGroup* group)
{
    const SOPC_PubSubConnection* conf_connection = SOPC_WriterGroup_Get_Connection(group);
    const SOPC_Conf_PublisherId* conf_pubid = SOPC_PubSubConnection_Get_PublisherId(conf_connection);
    // String not managed
    uint64_t conf_id = conf_pubid->data.uint;
    assert(SOPC_UInteger_PublisherId == conf_pubid->type);
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
