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
#include <stdbool.h>

#include "sopc_atomic.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_reader_layer.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_udp_sockets_mgr.h"
#include "sopc_udp_sockets.h"

char* ENDPOINT_URL = NULL;

static bool SOPC_SubScheduler_Start_UDP(void);

/* Transport context. One per connection */
typedef struct SOPC_SubScheduler_TransportCtx SOPC_SubScheduler_TransportCtx;

// Function to clear a transport context. To be implemented for each protocol
typedef void (*SOPC_SubScheduler_TransportCtx_Clear)(SOPC_SubScheduler_TransportCtx*);

static void SOPC_SubScheduler_CtxUdp_Clear(SOPC_SubScheduler_TransportCtx*);

struct SOPC_SubScheduler_TransportCtx
{
    SOPC_PubSubProtocol_Type protocol;
    SOPC_PubSubConnection* connection;
    SOPC_SubScheduler_TransportCtx_Clear fctClear;

    // specific to SOPC_PubSubProtocol_UDP
    Socket sock;
};

static struct
{
    /* Managing start / stop phase */
    int32_t isStarted;
    int32_t processingStartStop;

    /* Input parameters */
    SOPC_PubSubConfiguration* config;
    SOPC_SubTargetVariableConfig* targetConfig;
    SOPC_SubscriberStateChanged_Func stateCallback;

    /* Internal context */
    SOPC_PubSubState state;
    SOPC_Buffer* receptionBuffer;

    uint32_t nbConnections;
    // size is nbConnections
    SOPC_SubScheduler_TransportCtx* transport;

    // specific to SOPC_PubSubProtocol_UDP
    uint16_t nbSockets;
    SOPC_PubSubConnection** connectionArray;
    Socket* sockArray;

} schedulerCtx = {.isStarted = false,
                  .processingStartStop = false,
                  .stateCallback = NULL,
                  .state = SOPC_PubSubState_Disabled,
                  .nbConnections = 0,
                  .nbSockets = 0,
                  .transport = NULL,
                  .receptionBuffer = NULL};

static void set_new_state(SOPC_PubSubState new)
{
    SOPC_PubSubState prev = schedulerCtx.state;
    if ((prev != new) && (NULL != schedulerCtx.stateCallback))
    {
        schedulerCtx.stateCallback(new);
    }
    schedulerCtx.state = new;
}

// Common fonction to process received message
static void SOPC_SubScheduler_Read_UADP(SOPC_ReturnStatus pstatus, SOPC_PubSubConnection* connection);

// Specific callback for UDP message
static void on_udp_message_received(void* sockContext, Socket sock)
{
    assert(NULL != sockContext);

    SOPC_PubSubConnection* connection = (SOPC_PubSubConnection*) sockContext;

    SOPC_ReturnStatus status = SOPC_UDP_Socket_ReceiveFrom(sock, schedulerCtx.receptionBuffer);

    SOPC_SubScheduler_Read_UADP(status, connection);
}

static void uninit_sub_scheduler_ctx(void)
{
    schedulerCtx.config = NULL;
    schedulerCtx.targetConfig = NULL;
    schedulerCtx.stateCallback = NULL;

    for (uint32_t i = 0; i < schedulerCtx.nbConnections; i++)
    {
        schedulerCtx.transport[i].fctClear(&schedulerCtx.transport[i]);
    }
    schedulerCtx.nbConnections = 0;
    schedulerCtx.nbSockets = 0;
    SOPC_Free(schedulerCtx.transport);
    SOPC_Free(schedulerCtx.connectionArray);
    SOPC_Free(schedulerCtx.sockArray);
    schedulerCtx.transport = NULL;
    schedulerCtx.connectionArray = NULL;
    schedulerCtx.sockArray = NULL;
    SOPC_Buffer_Delete(schedulerCtx.receptionBuffer);
    schedulerCtx.receptionBuffer = NULL;
}

static SOPC_ReturnStatus init_sub_scheduler_ctx(SOPC_PubSubConfiguration* config,
                                                SOPC_SubTargetVariableConfig* targetConfig,
                                                SOPC_SubscriberStateChanged_Func stateChangedCb)
{
    uint32_t nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    assert(nb_connections > 0);

    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    schedulerCtx.config = config;
    schedulerCtx.targetConfig = targetConfig;
    schedulerCtx.stateCallback = stateChangedCb;

    // Allocate the subscriber scheduler context
    schedulerCtx.nbConnections = nb_connections;
    schedulerCtx.nbSockets = 0;
    schedulerCtx.transport = SOPC_Calloc(schedulerCtx.nbConnections, sizeof(*schedulerCtx.transport));
    result = (NULL != schedulerCtx.transport);

    if (!result)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        SOPC_Free(schedulerCtx.transport);
        schedulerCtx.nbConnections = 0;
        schedulerCtx.nbSockets = 0;
    }

    // Initialize the subscriber scheduler context: create socket + associated Sub connection config
    for (uint32_t i = 0; i < nb_connections && SOPC_STATUS_OK == status; i++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, i);
        const SOPC_Conf_PublisherId* pubId = SOPC_PubSubConnection_Get_PublisherId(connection);
        if (SOPC_Null_PublisherId == pubId->type)
        {
            assert(SOPC_PubSubConnection_Nb_WriterGroup(connection) == 0);
            // SOPC_Null_PublisherId pubId => Subscriber connection case

            uint16_t nbReaderGroups = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
            if (nbReaderGroups > 0)
            {
                schedulerCtx.transport[i].connection = connection;

                // Parse connection multicast address
                const char* address = SOPC_PubSubConnection_Get_Address(connection);

                SOPC_PubSubProtocol_Type protocol = SOPC_PubSub_Protocol_From_URI(address);

                SOPC_Socket_AddressInfo* multicastAddr = NULL;
                SOPC_Socket_AddressInfo* localAddr = NULL;

                switch (protocol)
                {
                case SOPC_PubSubProtocol_UDP:
                    result = SOPC_PubSubHelpers_Subscriber_ParseMulticastAddress(address, &multicastAddr, &localAddr);

                    // Create reception socket
                    if (result)
                    {
                        Socket* sock = &schedulerCtx.transport[i].sock;
                        schedulerCtx.transport[i].fctClear = &SOPC_SubScheduler_CtxUdp_Clear;
                        schedulerCtx.transport[i].protocol = SOPC_PubSubProtocol_UDP;

                        schedulerCtx.nbSockets++;
                        status = SOPC_UDP_Socket_CreateToReceive(multicastAddr, true, sock);
                        // Add socket to multicast group
                        if (SOPC_STATUS_OK == status)
                        {
                            status = SOPC_UDP_Socket_AddMembership(*sock, multicastAddr, localAddr);
                        }
                    }
                    else
                    {
                        status = SOPC_STATUS_INVALID_PARAMETERS;
                    }

                    SOPC_Socket_AddrInfoDelete(&multicastAddr);
                    SOPC_Socket_AddrInfoDelete(&localAddr);
                    break;
                case SOPC_PubSubProtocol_MQTT:
                case SOPC_PubSubProtocol_UNKOWN:
                default:
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }
            }
        }
    }

    // Create reception buffer
    if (result)
    {
        schedulerCtx.receptionBuffer = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
        result = (NULL != schedulerCtx.receptionBuffer);
    }

    if (false == result)
    {
        uninit_sub_scheduler_ctx();
    }

    return status;
}

bool SOPC_SubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_SubTargetVariableConfig* targetConfig,
                             SOPC_SubscriberStateChanged_Func stateChangedCb)
{
    SOPC_Helper_EndiannessCfg_Initialize(); // TODO: centralize / avoid recompute in S2OPC !

    if (NULL == config || NULL == targetConfig)
    {
        return false;
    }

    if (true == SOPC_Atomic_Int_Get(&schedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&schedulerCtx.processingStartStop))
    {
        return false;
    }
    else
    {
        SOPC_Atomic_Int_Set(&schedulerCtx.processingStartStop, true);
    }

    bool result = true;
    uint32_t nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    result = nb_connections > 0;

    if (result)
    {
        // Prepare connections context: socket creation & connection config context
        SOPC_ReturnStatus status = init_sub_scheduler_ctx(config, targetConfig, stateChangedCb);
        if (SOPC_STATUS_OK == status)
        {
            assert(schedulerCtx.nbConnections <= UINT16_MAX);
            // Run the socket manager with the context
            if (0 < schedulerCtx.nbSockets)
            {
                result = SOPC_SubScheduler_Start_UDP();
            }
        }
        else
        {
            result = false;
        }
    }

    if (result)
    {
        SOPC_Atomic_Int_Set(&schedulerCtx.isStarted, true);
        set_new_state(SOPC_PubSubState_Operational);
    }

    SOPC_Atomic_Int_Set(&schedulerCtx.processingStartStop, false);

    return result;
}

void SOPC_SubScheduler_Stop(void)
{
    if (false == SOPC_Atomic_Int_Get(&schedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&schedulerCtx.processingStartStop))
    {
        return;
    }

    // true because isStarted is false
    assert(schedulerCtx.nbConnections > 0);
    SOPC_Atomic_Int_Set(&schedulerCtx.processingStartStop, true);
    SOPC_UDP_SocketsMgr_Clear();
    set_new_state(SOPC_PubSubState_Disabled);
    uninit_sub_scheduler_ctx();

    SOPC_Atomic_Int_Set(&schedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&schedulerCtx.processingStartStop, false);
}

/*
  precondition :
   - call init_sub_scheduler_ctx
   - schedulerCtx.nbSockets > 0
   - schedulerCtx.sockArray and schedulerCtx.connectionArray are NULL
*/
static bool SOPC_SubScheduler_Start_UDP(void)
{
    assert(0 < schedulerCtx.nbSockets);
    assert(NULL == schedulerCtx.sockArray && NULL == schedulerCtx.connectionArray);

    uint16_t nb_socket = schedulerCtx.nbSockets;
    schedulerCtx.sockArray = SOPC_Calloc(nb_socket, sizeof(*schedulerCtx.sockArray));
    schedulerCtx.connectionArray = SOPC_Calloc(nb_socket, sizeof(*schedulerCtx.connectionArray));
    if (NULL == schedulerCtx.sockArray || NULL == schedulerCtx.connectionArray)
    {
        SOPC_Free(schedulerCtx.sockArray);
        SOPC_Free(schedulerCtx.connectionArray);
        return false;
    }
    uint16_t sockIdx = 0;
    // Initialize the subscriber scheduler context: create socket + associated Sub connection config
    for (uint32_t i = 0; i < schedulerCtx.nbConnections; i++)
    {
        if (SOPC_PubSubProtocol_UDP == schedulerCtx.transport[i].protocol)
        {
            schedulerCtx.sockArray[sockIdx] = schedulerCtx.transport[i].sock;
            schedulerCtx.connectionArray[sockIdx] = schedulerCtx.transport[i].connection;
            sockIdx++;
        }
    }

    assert(nb_socket == sockIdx);
    SOPC_UDP_SocketsMgr_Initialize((void**) schedulerCtx.connectionArray, schedulerCtx.sockArray, nb_socket,
                                   on_udp_message_received, NULL, NULL);

    return true;
}

static void SOPC_SubScheduler_Read_UADP(SOPC_ReturnStatus pstatus, SOPC_PubSubConnection* connection)
{
    SOPC_ReturnStatus status = pstatus;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Reader_Read_UADP(connection, schedulerCtx.receptionBuffer, schedulerCtx.targetConfig);
        if (SOPC_STATUS_OK != status)
        {
            set_new_state(SOPC_PubSubState_Error);
        }
        else
        {
            set_new_state(SOPC_PubSubState_Operational);
        }
    }
    else
    {
        set_new_state(SOPC_PubSubState_Error);
    }
    status = SOPC_Buffer_SetPosition(schedulerCtx.receptionBuffer, 0);
    assert(SOPC_STATUS_OK == status);
}

static void SOPC_SubScheduler_CtxUdp_Clear(SOPC_SubScheduler_TransportCtx* ctx)
{
    SOPC_UDP_Socket_Close(&(ctx->sock));
}
