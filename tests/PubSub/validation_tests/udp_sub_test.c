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
#include <inttypes.h>
#include <stdio.h>

#include "sopc_atomic.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_network_layer.h"
#include "sopc_reader_layer.h"
#include "sopc_sub_sockets_mgr.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"

#define MCAST_PORT "4840"
#define MCAST_ADDR "232.1.2.100"

#define CTX_VALUE (uintptr_t) 1

static SOPC_Buffer* buffer = NULL;
static int32_t stop = 0;

static int returnCode = -1;
static int sleepCount = 20;

static const uint32_t subGroupVersion = 963852;
static const uint32_t subGroupId = 1245;
static SOPC_PubSubConfiguration* configuration = NULL;
static SOPC_PubSubConnection* subConnection = NULL;

// Create connection, group and setup metadata so that message is accepted
// There are 2 groups created (one for "udp_pub_test" and the second for "udp_pub_conf_test")
static void setupConnection(void)
{
    SOPC_FieldMetaData* meta = NULL;
    SOPC_ReaderGroup* subReader = NULL;
    SOPC_DataSetReader* dsReader = NULL;
    configuration = SOPC_PubSubConfiguration_Create();
    assert(NULL != configuration);
    // "udp_pub_test"
    SOPC_PubSubConfiguration_Allocate_SubConnection_Array(configuration, 1);
    subConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(configuration, 0);

    SOPC_PubSubConnection_Allocate_ReaderGroup_Array(subConnection, 2);
    subReader = SOPC_PubSubConnection_Get_ReaderGroup_At(subConnection, 0);

    SOPC_ReaderGroup_Set_SecurityMode(subReader, SOPC_SecurityMode_None);
    SOPC_ReaderGroup_Allocate_DataSetReader_Array(subReader, 1);
    dsReader = SOPC_ReaderGroup_Get_DataSetReader_At(subReader, 0);
    SOPC_ReaderGroup_Set_GroupId(subReader, (uint16_t) subGroupId);
    SOPC_ReaderGroup_Set_GroupVersion(subReader, subGroupVersion);
    SOPC_ReaderGroup_Set_PublisherId_UInteger(subReader, 15300);

    SOPC_DataSetReader_Set_DataSetWriterId(dsReader, 123);

    SOPC_DataSetReader_Allocate_FieldMetaData_Array(dsReader, SOPC_TargetVariablesDataType, 5);
    // Var 1
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 0);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt32_Id);
    // Var 2
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 1);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_Byte_Id);
    // Var 3
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 2);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt16_Id);
    // Var 4
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 3);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_DateTime_Id);
    // Var 5
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 4);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt32_Id);

    // Configuration for "udp_pub_conf_test"
    subReader = SOPC_PubSubConnection_Get_ReaderGroup_At(subConnection, 1);

    SOPC_ReaderGroup_Set_SecurityMode(subReader, SOPC_SecurityMode_None);
    SOPC_ReaderGroup_Allocate_DataSetReader_Array(subReader, 1);
    dsReader = SOPC_ReaderGroup_Get_DataSetReader_At(subReader, 0);
    SOPC_ReaderGroup_Set_GroupId(subReader, (uint16_t) 45612);
    SOPC_ReaderGroup_Set_GroupVersion(subReader, 123456);
    //    SOPC_ReaderGroup_Set_PublisherId_UInteger(subReader, 15300);

    SOPC_DataSetReader_Set_DataSetWriterId(dsReader, 12);

    SOPC_DataSetReader_Allocate_FieldMetaData_Array(dsReader, SOPC_TargetVariablesDataType, 4);
    // Var 1
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 0);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt16_Id);
    // Var 2
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 1);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_DateTime_Id);
    // Var 3
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 2);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt32_Id);
    // Var 4
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 3);
    assert(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_String_Id);
}

static SOPC_UADP_NetworkMessage* Decode_NetworkMessage_NoSecu(SOPC_Buffer* pBuffer)
{
    assert(NULL != subConnection);
    const SOPC_UADP_NetworkMessage_Reader_Configuration readerConf = {
        .pGetSecurity_Func = NULL, .callbacks = SOPC_Reader_NetworkMessage_Default_Readers, .targetConfig = NULL};

    return SOPC_UADP_NetworkMessage_Decode(pBuffer, &readerConf, subConnection);
}

static void printVariant(const SOPC_Variant* variant)
{
    time_t time;

    printf("   - Variant Type %d\n", (int) variant->BuiltInTypeId);
    printf("   - Variant Array Type %d\n", (int) variant->ArrayType);
    switch (variant->BuiltInTypeId)
    {
    case SOPC_Byte_Id:
        printf("   - Variant Value %" PRIu8 "\n", variant->Value.Byte);
        break;
    case SOPC_UInt16_Id:
        printf("   - Variant Value %" PRIu16 "\n", variant->Value.Uint16);
        break;
    case SOPC_UInt32_Id:
        printf("   - Variant Value %" PRIu32 "\n", variant->Value.Uint32);
        break;
    case SOPC_UInt64_Id:
        printf("   - Variant Value %" PRIu64 "\n", variant->Value.Uint64);
        break;
    case SOPC_Int64_Id:
        printf("   - Variant Value %" PRIi64 "\n", variant->Value.Int64);
        break;
    case SOPC_DateTime_Id:
        SOPC_Time_ToTimeT(variant->Value.Date, &time);
        printf("   - Variant Value %s", ctime(&time));
        break;
    case SOPC_String_Id:
        printf("   - Variant Value %s\n", SOPC_String_GetRawCString(&(variant->Value.String)));
        break;
    default:
        printf("   - Variant type not managed\n");
    }
}

static void printPublisherId(const SOPC_UADP_NetworkMessage* uadp_nm)
{
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    const SOPC_UADP_Configuration* config = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);
    const SOPC_Dataset_LL_PublisherId* publisher_id = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
    switch (publisher_id->type)
    {
    case DataSet_LL_PubId_Byte_Id:
        printf("Publisher Id:\n - Enabled %d\n - Type Byte - Value %" PRIu8 "\n", config->GroupIdFlag,
               publisher_id->data.byte);
        break;
    case DataSet_LL_PubId_UInt16_Id:
        printf("Publisher Id:\n - Enabled %d\n - Type UInt16 - Value %" PRIu16 "\n", config->GroupIdFlag,
               publisher_id->data.uint16);
        break;
    case DataSet_LL_PubId_UInt32_Id:
        printf("Publisher Id:\n - Enabled %d\n - Type UInt32 - Value %" PRIu32 "\n", config->GroupIdFlag,
               publisher_id->data.uint32);
        break;
    case DataSet_LL_PubId_UInt64_Id:
        printf("Publisher Id:\n - Enabled %d\n - Type UInt64 - Value %" PRIu64 "\n", config->GroupIdFlag,
               publisher_id->data.uint64);
        break;
    case DataSet_LL_PubId_String_Id:
        printf("Publisher Id:\n - Enabled %d\n - Type String not managed\n", config->PublisherIdFlag);
        break;
    default:
        printf("Publisher Id:\n - Enabled %d\n - Type not managed %u\n", config->PublisherIdFlag, publisher_id->type);
    }
}

static void printNetworkMessage(const SOPC_UADP_NetworkMessage* uadp_nm)
{
    if (NULL != uadp_nm)
    {
        SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
        SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
        const SOPC_UADP_Configuration* config = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);
        printf("UADP Version %d\n", SOPC_Dataset_LL_NetworkMessage_GetVersion(header));
        printPublisherId(uadp_nm);
        printf("Security Enabled %d\n", config->SecurityFlag);
        printf("Group Header Enabled %d\n", config->GroupHeaderFlag);
        printf("Writer Group Id:\n - Enabled %d\n - Value %d\n", config->GroupIdFlag,
               SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
        printf("Writer Group Version:\n - Enabled %d\n - Value %u\n", config->GroupVersionFlag,
               SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));
        uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
        printf("Nb DataSetMessage %d\n", nbDsm);

        for (int i = 0; i < nbDsm; i++)
        {
            printf("== DataSetMessage %d ==\n", i);
            SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, i);
            uint16_t nbDsf = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
            printf(" - Nb DataSetField %d\n", nbDsf);
            for (uint16_t j = 0; j < nbDsf; j++)
            {
                const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, j);
                printf(" - DataSetField %" PRIu16 "\n", j);
                printVariant(variant);
            }
        }
    }
    else
    {
        printf("UADP Msg = <NULL>\n");
        const SOPC_UADP_NetworkMessage_Error_Code errCode = SOPC_UADP_NetworkMessage_Get_Last_Error();

        printf("Last SOPC_UADP_NetworkMessage_Get_Last_Error()= %d (0x%08X)\n", (int) errCode, (int) errCode);
    }
}

static void readyToReceive(void* sockContext, Socket sock)
{
    SOPC_UNUSED_ARG(sockContext);
    if (SOPC_Atomic_Int_Get(&stop))
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_UDP_Socket_ReceiveFrom(sock, buffer);
    if (SOPC_STATUS_OK == status && buffer->length > 1)
    {
        uint64_t i = 0;
        status = SOPC_Buffer_Read((uint8_t*) &i, buffer, 8);
        buffer->position = 0;
        if (SOPC_STATUS_OK == status)
        {
            SOPC_UADP_NetworkMessage* uadp_nm = Decode_NetworkMessage_NoSecu(buffer);
            printNetworkMessage(uadp_nm);
            if (NULL != uadp_nm)
            {
                returnCode = 0;
            }
            SOPC_UADP_NetworkMessage_Delete(uadp_nm);
        }
        SOPC_Atomic_Int_Set(&stop, true);
    }
    else if (SOPC_STATUS_OK == status && buffer->length == 1)
    {
        assert(buffer->data[0] == 0);
        printf("Received 'empty' data, used to check that subscriber is running !\n");
    }
    else
    {
        SOPC_Atomic_Int_Set(&stop, true);
    }
    SOPC_Buffer_SetPosition(buffer, 0);
}

static void tick(void* tickCtx)
{
    assert(CTX_VALUE == (uintptr_t) tickCtx);
    // printf("tick !\n");
}

int main(void)
{
    printf("main -UDP_SUB_TEST\n");
    Socket sock;
    SOPC_Socket_AddressInfo* listenAddr = SOPC_UDP_SocketAddress_Create(false, MCAST_ADDR, MCAST_PORT);
    SOPC_Socket_AddressInfo* localAddr = SOPC_UDP_SocketAddress_Create(false, NULL, MCAST_PORT);

    setupConnection();

    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_ReturnStatus status = SOPC_UDP_Socket_CreateToReceive(listenAddr, NULL, true, true, &sock);
    buffer = SOPC_Buffer_Create(4096);

    if (SOPC_STATUS_OK == status && buffer != NULL)
    {
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UDP_Socket_AddMembership(sock, NULL, listenAddr, localAddr);
            if (SOPC_STATUS_OK != status)
            {
                printf("SOPC_UDP_Socket_AddMembership failed\n");
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Sub_SocketsMgr_Initialize(NULL, 0, &sock, 1, readyToReceive, tick, (void*) CTX_VALUE, 0);
    }

    while (SOPC_STATUS_OK == status && false == SOPC_Atomic_Int_Get(&stop) && sleepCount > 0)
    {
        SOPC_Sleep(100);
        sleepCount--;
    }
    SOPC_Sub_SocketsMgr_Clear();

    SOPC_Buffer_Delete(buffer);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_UDP_SocketAddress_Delete(&listenAddr);
    SOPC_UDP_SocketAddress_Delete(&localAddr);
    SOPC_PubSubConfiguration_Delete(configuration);

    return returnCode;
}
