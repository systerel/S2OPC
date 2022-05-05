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
#include <signal.h>
#include <stdlib.h>

#include "sopc_atomic.h"
#include "sopc_dataset_layer.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_network_layer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"

#define MCAST_PORT "4840"
#define MCAST_ADDR "232.1.2.100"

static int32_t stopPublisher = false;

static void Test_StopSignal(int sig)
{
    SOPC_UNUSED_ARG(sig);

    if (SOPC_Atomic_Int_Get(&stopPublisher) != false)
    {
        exit(1);
    }
    else
    {
        SOPC_Atomic_Int_Set(&stopPublisher, true);
    }
}

static SOPC_PubSubConfiguration* UDP_Pub_Test_Get_Conf(void)
{
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, 1);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
    bool alloc = SOPC_PubSubConnection_Set_Name(connection, "Kimi no na wa");
    assert(alloc);
    SOPC_PubSubConnection_Set_Enabled(connection, true);
    SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, 14562);
    alloc = SOPC_PubSubConnection_Set_TransportProfileUri(connection, "udp:uadp");
    assert(alloc);
    alloc = SOPC_PubSubConnection_Set_Address(connection, MCAST_ADDR);
    assert(alloc);

    SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, 1);
    SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, 0);

    SOPC_WriterGroup_Set_Id(group, 45612);
    SOPC_WriterGroup_Set_Version(group, 123456);
    SOPC_WriterGroup_Set_PublishingInterval(group, 2000.);

    SOPC_UadpNetworkMessageContentMask contentMask;
    contentMask.PublisherIdFlag = true;
    contentMask.GroupHeaderFlag = true;
    contentMask.GroupIdFlag = true;
    contentMask.GroupVersionFlag = false;
    contentMask.NetworkMessageNumberFlag = false;
    contentMask.SequenceNumberFlag = false;
    contentMask.PayloadHeaderFlag = true;
    contentMask.TimestampFlag = false;
    contentMask.PicoSecondsFlag = false;
    contentMask.DataSetClassIdFlag = false;
    contentMask.SecurityFlag = false;
    contentMask.PromotedFieldsFlag = false;
    SOPC_WriterGroup_Set_NetworkMessageContentMask(group, contentMask);

    // Create one DataSet Writer
    SOPC_WriterGroup_Allocate_DataSetWriter_Array(group, 1);
    SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);
    SOPC_DataSetWriter_Set_Id(writer, 12);

    // Create one PublishedDataSet
    SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 1);
    SOPC_PublishedDataSet* dataset = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0);
    SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataItemsDataType, 4);
    SOPC_DataSetWriter_Set_DataSet(writer, dataset);

    // n > 1: the dataType is an array with the specified number of dimensions.
    // OneDimension (1)
    // OneOrMoreDimensions (0)
    // Scalar (−1)
    // Any (−2)
    // ScalarOrOneDimension (−3)

    // Scalar uint16
    SOPC_FieldMetaData* fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, 0);
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, SOPC_UInt16_Id);

    // Scalar DateTime
    fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, 1);
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, SOPC_DateTime_Id);

    // Array one dimension uint32
    fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, 2);
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, SOPC_UInt32_Id);

    // Scalar String
    fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, 3);
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, SOPC_String_Id);

    // SOPC_PublishedVariable* SOPC_PublishedDataSet_Get_SourceVariable_At(SOPC_PublishedDataSet* dataset, uint16_t
    // index);

    return config;
}

static void UDP_Pub_Test_Fill_NetworkMessage(SOPC_WriterGroup* group, SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_DataSetWriter* conf_writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);

    SOPC_FieldMetaData* metadata;
    SOPC_Variant* variant;
    // variant 1
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_UInt16_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint16 = 1402;
    const SOPC_PublishedDataSet* conf_dataset = SOPC_DataSetWriter_Get_DataSet(conf_writer);
    metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(conf_dataset, 0);
    SOPC_NetworkMessage_Set_Variant_At(nm, 0, 0, variant, metadata);
    // variant 2
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_DateTime_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Date = SOPC_Time_GetCurrentTimeUTC();
    metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(conf_dataset, 1);
    SOPC_NetworkMessage_Set_Variant_At(nm, 0, 1, variant, metadata);
    // variant 3
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_UInt32_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint32 = 64852;
    metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(conf_dataset, 2);
    SOPC_NetworkMessage_Set_Variant_At(nm, 0, 2, variant, metadata);
    // variant 4
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_String_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    SOPC_String_Initialize(&(variant->Value.String));
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&(variant->Value.String), "Ma chaine de caractère");
    assert(SOPC_STATUS_OK == status);
    metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(conf_dataset, 3);
    SOPC_NetworkMessage_Set_Variant_At(nm, 0, 3, variant, metadata);
}

int main(void)
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    Socket sock = SOPC_INVALID_SOCKET;
    SOPC_Socket_AddressInfo* multicastAddr = SOPC_UDP_SocketAddress_Create(false, MCAST_ADDR, MCAST_PORT);
    SOPC_Helper_EndiannessCfg_Initialize();

    status = SOPC_UDP_Socket_CreateToSend(multicastAddr, NULL, true, &sock);
    SOPC_PubSubConfiguration* config = UDP_Pub_Test_Get_Conf();
    SOPC_PubSubConnection* conf_connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
    if (NULL == conf_connection)
    {
        return -1;
    }
    SOPC_WriterGroup* conf_group = SOPC_PubSubConnection_Get_WriterGroup_At(conf_connection, 0);
    if (NULL == conf_group)
    {
        return -1;
    }
    SOPC_Dataset_NetworkMessage* nm = SOPC_Create_NetworkMessage_From_WriterGroup(conf_group);
    if (NULL == nm)
    {
        return -1;
    }
    UDP_Pub_Test_Fill_NetworkMessage(conf_group, nm);

    SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(nm, NULL);

    if (SOPC_STATUS_OK == status && buffer != NULL)
    {
        while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&stopPublisher) == false)
        {
            status = SOPC_UDP_Socket_SendTo(sock, multicastAddr, buffer);
            assert(SOPC_STATUS_OK == status);
            SOPC_Sleep(100);
        }
    }
    else
    {
        return 1;
    }

    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_Buffer_Delete(buffer);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_UDP_SocketAddress_Delete(&multicastAddr);
}
