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
#include "sopc_network_layer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"
#include "sopc_xml_loader.h"

#define MCAST_PORT "4840"
#define MCAST_ADDR "232.1.2.100"

static int32_t stopPublisher = false;

static void Test_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    if (SOPC_Atomic_Int_Get(&stopPublisher) != false)
    {
        exit(1);
    }
    else
    {
        SOPC_Atomic_Int_Set(&stopPublisher, true);
    }
}

static void UDP_Pub_Test_Fill_NetworkMessage(SOPC_WriterGroup* group, SOPC_Dataset_LL_NetworkMessage* nm)
{
    SOPC_DataSetWriter* conf_writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);

    SOPC_FieldMetaData* metadata;
    SOPC_Variant* variant;
    // variant 1
    variant = SOPC_Variant_Create();
    variant->BuiltInTypeId = SOPC_Boolean_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Boolean = true;
    const SOPC_PublishedDataSet* conf_dataset = SOPC_DataSetWriter_Get_DataSet(conf_writer);
    metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(conf_dataset, 0);
    SOPC_NetworkMessage_Set_Variant_At(nm, 0, 0, variant, metadata);
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

    status = SOPC_UDP_Socket_CreateToSend(multicastAddr, true, &sock);
    FILE* fd = fopen("./config_pub.xml", "r");
    assert(NULL != fd);
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);

    int closed = fclose(fd);
    assert(0 == closed);
    assert(NULL != config);

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
