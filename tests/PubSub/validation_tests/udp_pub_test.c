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
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_network_layer.h"
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

static SOPC_Dataset_LL_NetworkMessage* UDP_Pub_Test_Get_NetworkMessage(void)
{
    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(1, 1);
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, 1);
    SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(dsm, 5);
    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(header, 15300);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, 1245);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, 963852);
    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, 123);

    SOPC_Variant* variant;
    // variant 1
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_UInt32_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint32 = 12071982;
    bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, 0);
    assert(res);
    // variant 2
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_Byte_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Byte = 239;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, 1);
    assert(res);
    // variant 3
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_UInt16_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint16 = 64852;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, 2);
    assert(res);
    // variant 4
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_DateTime_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Date = SOPC_Time_GetCurrentTimeUTC();
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, 3);
    assert(res);
    // variant 5
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_UInt32_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint32 = 369852;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, 4);
    assert(res);

    return nm;
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
    SOPC_Dataset_LL_NetworkMessage* nm = UDP_Pub_Test_Get_NetworkMessage();
    if (NULL == nm)
    {
        return -1;
    }
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
    SOPC_Buffer_Delete(buffer);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_UDP_SocketAddress_Delete(&multicastAddr);
}
