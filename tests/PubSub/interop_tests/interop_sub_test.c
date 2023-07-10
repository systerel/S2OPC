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

/**
 * This Subscriber is waiting for a single UDP message.
 * Expected message should contains :
 *  - opc.udp://232.1.2.100:4840
 *  - network message version = 1
 *  - group id = 10
 *  - group version = 0
 *  - publisher ID = 3
 *  - One DataSetMessage:
 *    - Field 1 with value 'The Ultimate Question of Life, the Universe and Everything' and type String
 *    - Field 2 with value 42 and type UInt32
 *    - Field 3 with value -314 and type Int16
 *    - Field 4 with value True and type Boolean
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_reader_layer.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"
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

static SOPC_PubSubConfiguration* configuration = NULL;
static SOPC_PubSubConnection* subConnection = NULL;

SOPC_SKManager* g_skmanager = NULL;

SOPC_Byte signingKey[SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_Signature] = {0};
SOPC_Byte encryptingKey[SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_CryptoKey] = {0};
SOPC_Byte keyNonce[SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_KeyNonce] = {0};

typedef enum
{
    SECU_NONE,
    SECU_ENCRYPTED
} subscriberSecurityMode;

subscriberSecurityMode gSubSecuMode = SECU_NONE;

SOPC_PubSub_SecurityType gSubSecurityType;

static void set_subscriber_security_info(void)
{
    gSubSecurityType.groupKeys = SOPC_PubSubSKS_GetSecurityKeys(SOPC_PUBSUB_SKS_DEFAULT_GROUPID, 0);
    gSubSecurityType.mode = SOPC_SecurityMode_SignAndEncrypt;
    gSubSecurityType.provider = SOPC_CryptoProvider_CreatePubSub(SOPC_SecurityPolicy_PubSub_Aes256_URI);
}

static SOPC_SKManager* createSKmanager(void)
{
    /* Create Service Keys manager and set constant keys */
    SOPC_SKManager* skm = SOPC_SKManager_Create();
    SOPC_ASSERT(NULL != skm && "SOPC_SKManager_Create failed");
    uint32_t nbKeys = 0;
    SOPC_Buffer* keysBuffer = SOPC_Buffer_Create(sizeof(signingKey) + sizeof(encryptingKey) + sizeof(keyNonce));
    SOPC_ReturnStatus status = (NULL == keysBuffer ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, signingKey, (uint32_t) sizeof(signingKey));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, encryptingKey, (uint32_t) sizeof(encryptingKey));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, keyNonce, (uint32_t) sizeof(keyNonce));
    }
    SOPC_ByteString keys;
    SOPC_ByteString_Initialize(&keys);
    SOPC_String securityPolicyUri;
    SOPC_String_Initialize(&securityPolicyUri);
    if (SOPC_STATUS_OK == status)
    {
        nbKeys = 1;
        // Set buffer as a byte string for API compatibility
        keys.DoNotClear = true;
        keys.Length = (int32_t) keysBuffer->length;
        keys.Data = (SOPC_Byte*) keysBuffer->data;
        // Set security policy
        status = SOPC_String_AttachFromCstring(&securityPolicyUri, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKManager_SetKeys(skm, &securityPolicyUri, 1, &keys, nbKeys, UINT32_MAX, UINT32_MAX);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_SKManager_Clear(skm);
        SOPC_Free(skm);
        skm = NULL;
    }
    SOPC_Buffer_Delete(keysBuffer);

    return skm;
}

// Create connection, group and setup metadata so that message is accepted
static void setupConnection(void)
{
    SOPC_FieldMetaData* meta = NULL;
    SOPC_ReaderGroup* subReader = NULL;
    SOPC_DataSetReader* dsReader = NULL;
    configuration = SOPC_PubSubConfiguration_Create();
    SOPC_ASSERT(NULL != configuration);

    // Matching "config_pub_interop.xml"
    SOPC_PubSubConfiguration_Allocate_SubConnection_Array(configuration, 1);
    subConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(configuration, 0);

    SOPC_PubSubConnection_Allocate_ReaderGroup_Array(subConnection, 1);
    subReader = SOPC_PubSubConnection_Get_ReaderGroup_At(subConnection, 0);

    /* PubSub Security Keys configuration */
    g_skmanager = createSKmanager();
    SOPC_ASSERT(NULL != g_skmanager && "SOPC_SKManager_SetKeys failed");
    SOPC_PubSubSKS_Init();
    SOPC_PubSubSKS_SetSkManager(g_skmanager);

    if (SECU_NONE == gSubSecuMode)
    {
        SOPC_ReaderGroup_Set_SecurityMode(subReader, SOPC_SecurityMode_None);
    }
    else
    {
        SOPC_ReaderGroup_Set_SecurityMode(subReader, SOPC_SecurityMode_SignAndEncrypt);
        set_subscriber_security_info();
    }
    SOPC_ReaderGroup_Allocate_DataSetReader_Array(subReader, 1);
    dsReader = SOPC_ReaderGroup_Get_DataSetReader_At(subReader, 0);
    SOPC_ReaderGroup_Set_GroupId(subReader, (uint16_t) 10);
    SOPC_ReaderGroup_Set_GroupVersion(subReader, 0);
    SOPC_ReaderGroup_Set_PublisherId_UInteger(subReader, 3);

    SOPC_DataSetReader_Set_DataSetWriterId(dsReader, 62541);

    SOPC_DataSetReader_Allocate_FieldMetaData_Array(dsReader, SOPC_TargetVariablesDataType, 4);
    // Var 1
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 0);
    SOPC_ASSERT(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_String_Id);
    // Var 2
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 1);
    SOPC_ASSERT(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_UInt32_Id);
    // Var 3
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 2);
    SOPC_ASSERT(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_Int16_Id);
    // Var 4
    meta = SOPC_DataSetReader_Get_FieldMetaData_At(dsReader, 3);
    SOPC_ASSERT(NULL != meta);
    SOPC_FieldMetaData_Set_ValueRank(meta, -1);
    SOPC_FieldMetaData_Set_BuiltinType(meta, SOPC_Boolean_Id);
}

static void printVariant(const SOPC_Variant* variant)
{
    printf("DataSetField NOK\n");
    printf("   - Variant Type %d\n", (int) variant->BuiltInTypeId);
    switch (variant->BuiltInTypeId)
    {
    case SOPC_Boolean_Id:
        printf("   - Variant Value %d\n", variant->Value.Boolean);
        break;
    case SOPC_Int16_Id:
        printf("   - Variant Value %d\n", variant->Value.Int16);
        break;
    case SOPC_UInt32_Id:
        printf("   - Variant Value %u\n", variant->Value.Uint32);
        break;
    case SOPC_String_Id:
        printf("   - Variant Value %s\n", SOPC_String_GetRawCString(&(variant->Value.String)));
        break;
    default:
        printf("   - Variant type not managed: %d\n", (int) variant->BuiltInTypeId);
    }
}

static void clear_setupConnection(void)
{
    if (NULL != g_skmanager)
    {
        SOPC_SKManager_Clear(g_skmanager);
        SOPC_Free(g_skmanager);
        g_skmanager = NULL;
    }
}

static int TestNetworkMessage(const SOPC_UADP_NetworkMessage* uadp_nm)
{
    if (NULL == uadp_nm)
    {
        printf("Network Message NOK. Message is null\n");
        const SOPC_UADP_NetworkMessage_Error_Code errCode = SOPC_UADP_NetworkMessage_Get_Last_Error();

        printf("Last SOPC_UADP_NetworkMessage_Get_Last_Error()= %d (0x%08X)\n", (int) errCode, (int) errCode);
        return -1;
    }
    int result = 0;
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    const SOPC_UADP_Configuration* config = SOPC_Dataset_LL_NetworkMessage_GetHeaderConfig(header);
    SOPC_ASSERT(NULL != config);

    if (1 != SOPC_Dataset_LL_NetworkMessage_GetVersion(header))
    {
        printf("UADP Version NOK : %d\n", SOPC_Dataset_LL_NetworkMessage_GetVersion(header));
        result = -1;
    }
    {
        const SOPC_Dataset_LL_PublisherId* publisher_id = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
        if (false == config->GroupIdFlag || DataSet_LL_PubId_UInt16_Id != publisher_id->type ||
            3 != publisher_id->data.uint16)
        {
            printf("Publisher Id NOK :\n - Enabled %d\n - Type %d\n - Value %d\n", config->GroupIdFlag,
                   (int) publisher_id->type, publisher_id->data.byte);
            result = -1;
        }
    }
    if (!config->GroupHeaderFlag)
    {
        printf("Group Header should be enabled\n");
        result = -1;
    }
    if (false == config->GroupIdFlag || 10 != SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm))
    {
        printf("Writer Group Id NOK:\n - Enabled %d\n - Value %d\n", config->GroupIdFlag,
               SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
        result = -1;
    }

    if (false == config->GroupVersionFlag || 0 != SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm))
    {
        printf("Writer Group Version NOK:\n - Enabled %d\n - Value %u\n", config->GroupVersionFlag,
               SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));
        result = -1;
    }

    const uint8_t nbDsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm);
    if (1 != nbDsm)
    {
        printf("Nb DataSetMessage NOK. Should be 1 : %d\n", nbDsm);
        result = -1;
    }
    else
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
        uint16_t nbDsf = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        if (4 != nbDsf)
        {
            printf("Nb DataSetField NOK. Should be 4 : %d\n", nbDsf);
            result = -1;
        }
        else
        {
            {
                const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, 0);
                if (SOPC_String_Id != variant->BuiltInTypeId ||
                    0 != strcmp("The Ultimate Question of Life, the Universe and Everything",
                                SOPC_String_GetRawCString(&(variant->Value.String))))
                {
                    printVariant(variant);
                    result = -1;
                }
            }
            {
                const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, 1);
                if (SOPC_UInt32_Id != variant->BuiltInTypeId || 42 != (variant->Value.Uint32))
                {
                    printVariant(variant);
                    result = -1;
                }
            }
            {
                const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, 2);
                if (SOPC_Int16_Id != variant->BuiltInTypeId || -314 != (variant->Value.Int16))
                {
                    printVariant(variant);
                    result = -1;
                }
            }

            {
                const SOPC_Variant* variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, 3);
                if (SOPC_Boolean_Id != variant->BuiltInTypeId || true != (variant->Value.Boolean))
                {
                    printVariant(variant);
                    result = -1;
                }
            }
        }
    }
    return result;
}

static SOPC_PubSub_SecurityType* retrieve_security_info(uint32_t tokenId,
                                                        const SOPC_Conf_PublisherId pubId,
                                                        uint16_t writerGroupId)
{
    SOPC_UNUSED_ARG(tokenId);
    SOPC_UNUSED_ARG(pubId);
    SOPC_UNUSED_ARG(writerGroupId);

    if (SECU_NONE == gSubSecuMode)
    {
        return NULL;
    }
    else
    {
        return &gSubSecurityType;
    }
}

static void readyToReceive(void* sockContext, Socket sock)
{
    SOPC_UNUSED_ARG(sockContext);
    if (SOPC_Atomic_Int_Get(&stop))
    {
        return;
    }

    const SOPC_UADP_NetworkMessage_Reader_Configuration readerConf = {
        .pGetSecurity_Func = retrieve_security_info,
        .callbacks = SOPC_Reader_NetworkMessage_Default_Readers,
        .checkDataSetMessageSN_Func = NULL,
        .targetConfig = NULL};

    SOPC_ReturnStatus status = SOPC_UDP_Socket_ReceiveFrom(sock, buffer);
    if (SOPC_STATUS_OK == status && buffer->length > 1)
    {
        uint64_t i = 0;
        status = SOPC_Buffer_Read((uint8_t*) &i, buffer, 8);
        buffer->position = 0;
        if (SOPC_STATUS_OK == status)
        {
            // do not manage security
            SOPC_UADP_NetworkMessage* uadp_nm = SOPC_UADP_NetworkMessage_Decode(buffer, &readerConf, subConnection);
            returnCode = TestNetworkMessage(uadp_nm);
            SOPC_UADP_NetworkMessage_Delete(uadp_nm);
        }
        SOPC_Atomic_Int_Set(&stop, true);
    }
    else if (SOPC_STATUS_OK == status && buffer->length == 1)
    {
        SOPC_ASSERT(buffer->data[0] == 0);
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
    SOPC_ASSERT(CTX_VALUE == (uintptr_t) tickCtx);
    // printf("tick !\n");
}

int main(int argc, char** argv)
{
    Socket sock;
    SOPC_Socket_AddressInfo* listenAddr = SOPC_UDP_SocketAddress_Create(false, MCAST_ADDR, MCAST_PORT);
    SOPC_Socket_AddressInfo* localAddr = SOPC_UDP_SocketAddress_Create(false, NULL, MCAST_PORT);
    if (argc > 1)
    {
        if (0 == strncmp(argv[1], "none", 4))
        {
            gSubSecuMode = SECU_NONE;
        }
        else if (0 == strncmp(argv[1], "encrypted", 9))
        {
            gSubSecuMode = SECU_ENCRYPTED;
        }
        else
        {
            printf("Error: Security mode accepted are none or encrypted\n");
        }
    }
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
    clear_setupConnection();

    return returnCode;
}
