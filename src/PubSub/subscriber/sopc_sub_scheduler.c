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
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_reader_layer.h"
#include "sopc_rt_subscriber.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_udp_sockets_mgr.h"
#include "sopc_threads.h"
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
    uint32_t inputNumber;
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
    SOPC_Buffer* receptionBufferUDP; /*For all UDP connections*/

    uint32_t nbConnections;
    // size is nbConnections
    SOPC_SubScheduler_TransportCtx* transport;

    // specific to SOPC_PubSubProtocol_UDP
    uint16_t nbSockets;
    uint32_t** sockInputNumbers;
    Socket* sockArray;

    SOPC_RT_Subscriber* pRTSubscriber;
    Thread handleRTSubscriberBeatHeart;
    bool bQuitSubcriberBeatHeart;
} schedulerCtx = {.isStarted = false,
                  .processingStartStop = false,
                  .stateCallback = NULL,
                  .state = SOPC_PubSubState_Disabled,
                  .nbConnections = 0,
                  .nbSockets = 0,
                  .transport = NULL,
                  .sockInputNumbers = NULL,
                  .sockArray = NULL,
                  .receptionBufferUDP = NULL,
                  .pRTSubscriber = NULL,
                  .handleRTSubscriberBeatHeart = (Thread) NULL,
                  .bQuitSubcriberBeatHeart = false};

static void set_new_state(SOPC_PubSubState new)
{
    SOPC_PubSubState prev = schedulerCtx.state;
    if ((prev != new) && (NULL != schedulerCtx.stateCallback))
    {
        schedulerCtx.stateCallback(new);
    }
    schedulerCtx.state = new;
}

// Specific callback for UDP message
static void on_udp_message_received(void* pInputIdentifier, Socket sock)
{
    assert(NULL != pInputIdentifier);

    // Get input
    uint32_t inputIdentifier = *((uint32_t*) pInputIdentifier);

    SOPC_Buffer_SetPosition(schedulerCtx.receptionBufferUDP, 0);
    SOPC_ReturnStatus status = SOPC_UDP_Socket_ReceiveFrom(sock, schedulerCtx.receptionBufferUDP);

    // Write input
    if (SOPC_STATUS_OK == status)
    {
        SOPC_RT_Subscriber_Input_Write(schedulerCtx.pRTSubscriber,               //
                                       inputIdentifier,                          //
                                       schedulerCtx.receptionBufferUDP->data,    //
                                       schedulerCtx.receptionBufferUDP->length); //
    }
}

static SOPC_ReturnStatus SOPC_RT_Subscriber_Callback(SOPC_RT_Subscriber* pSub, // RT Subscriber object
                                                     void* pContext,           // User context
                                                     void* pInputContext,      // Decode context
                                                     uint32_t input_number,    // Input pin number
                                                     uint8_t* pData,           // Data received
                                                     uint32_t size)            // Size of data
{
    (void) input_number;
    (void) pContext;
    (void) pSub;

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    SOPC_PubSubConnection* pDecoderContext = (SOPC_PubSubConnection*) pInputContext;

#if DEBUG_PUBSUB_SCHEDULER_INFO
    printf("# RT Subscriber callback - receive data on input number = %u - size = %u - context = %08lx\r\n", //
           input_number,                                                                                     //
           size,                                                                                             //
           (uint64_t) pInputContext);                                                                        //
#endif

    if (NULL == pDecoderContext)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Buffer buffer;
    buffer.position = 0;
    buffer.length = size;
    buffer.maximum_size = size;
    buffer.current_size = size;
    buffer.data = pData;

    if (SOPC_PubSubState_Operational == schedulerCtx.state)
    {
        result = SOPC_Reader_Read_UADP(pDecoderContext,            //
                                       &buffer,                    //
                                       schedulerCtx.targetConfig); //

        if (SOPC_STATUS_OK != result)
        {
            set_new_state(SOPC_PubSubState_Error);
        }
    }

    return result;
}

// Beat heart thread
static void* cbBeatHeartThreadCallback(void* arg)
{
    (void) arg;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    printf("# RT Subscriber beat heart thread launched !!!\r\n");

    bool readValue = false;
    __atomic_load(&schedulerCtx.bQuitSubcriberBeatHeart, &readValue, __ATOMIC_SEQ_CST);

    while (!readValue)
    {
        result = SOPC_RT_Subscriber_HeartBeat(schedulerCtx.pRTSubscriber);
        if (SOPC_STATUS_OK != result)
        {
            printf("# RT Subscriber beat heart thread error : %u\r\n", result);
        }

        SOPC_Sleep(SOPC_TIMER_RESOLUTION_MS);
        __atomic_load(&schedulerCtx.bQuitSubcriberBeatHeart, &readValue, __ATOMIC_SEQ_CST);
    }

    printf("# RT Subscriber beat heart thread exit !!!\r\n");

    return NULL;
}

static void uninit_sub_scheduler_ctx(void)
{
    schedulerCtx.config = NULL;
    schedulerCtx.targetConfig = NULL;
    schedulerCtx.stateCallback = NULL;

    printf("# Info: Stop RT Subscriber thread. \r\n");
    bool newValue = true;
    __atomic_store(&schedulerCtx.bQuitSubcriberBeatHeart, &newValue, __ATOMIC_SEQ_CST);
    if (schedulerCtx.handleRTSubscriberBeatHeart != (Thread) NULL)
    {
        SOPC_Thread_Join(schedulerCtx.handleRTSubscriberBeatHeart);
        schedulerCtx.handleRTSubscriberBeatHeart = (Thread) NULL;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    while (SOPC_STATUS_INVALID_STATE == status)
    {
        printf("# Info: DeInitialize RT Subscriber. \r\n");
        status = SOPC_RT_Subscriber_DeInitialize(schedulerCtx.pRTSubscriber);
    }
    for (uint32_t i = 0; i < schedulerCtx.nbConnections; i++)
    {
        schedulerCtx.transport[i].fctClear(&schedulerCtx.transport[i]);
        printf("# Info: transport context destroyed for connection #%d (subscriber). \n", i);
    }
    printf("# Info: Destroy RT Subscriber. \r\n");
    SOPC_RT_Subscriber_Destroy(&schedulerCtx.pRTSubscriber);
    schedulerCtx.nbConnections = 0;
    schedulerCtx.nbSockets = 0;
    if (schedulerCtx.transport != NULL)
    {
        SOPC_Free(schedulerCtx.transport);
        schedulerCtx.transport = NULL;
    }
    if (schedulerCtx.sockInputNumbers)
    {
        SOPC_Free(schedulerCtx.sockInputNumbers);
        schedulerCtx.sockInputNumbers = NULL;
    }
    if (schedulerCtx.sockArray)
    {
        SOPC_Free(schedulerCtx.sockArray);
        schedulerCtx.sockArray = NULL;
    }
    schedulerCtx.transport = NULL;
    schedulerCtx.sockInputNumbers = NULL;
    schedulerCtx.sockArray = NULL;
    SOPC_Buffer_Delete(schedulerCtx.receptionBufferUDP);
    schedulerCtx.receptionBufferUDP = NULL;
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

    schedulerCtx.receptionBufferUDP = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
    result = (NULL != schedulerCtx.receptionBufferUDP);

    if (result)
    {
        // Allocate the subscriber scheduler context
        schedulerCtx.nbConnections = nb_connections;
        schedulerCtx.nbSockets = 0;
        schedulerCtx.transport = SOPC_Calloc(schedulerCtx.nbConnections, sizeof(SOPC_SubScheduler_TransportCtx));
        result = (NULL != schedulerCtx.transport);
    }

    if (!result)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_RT_Subscriber_Initializer* pRTInitializer = NULL;

    if (result)
    {
        pRTInitializer = SOPC_RT_Subscriber_Initializer_Create(SOPC_RT_Subscriber_Callback, NULL);
        result = (NULL != pRTInitializer);
        if (!result)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result)
    {
        schedulerCtx.pRTSubscriber = SOPC_RT_Subscriber_Create();
        result = (NULL != schedulerCtx.pRTSubscriber);
        if (!result)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Initialize the subscriber scheduler context: create socket + associated Sub connection config
    for (uint32_t iIter = 0; iIter < nb_connections && result; iIter++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, iIter);
        const SOPC_Conf_PublisherId* pubId = SOPC_PubSubConnection_Get_PublisherId(connection);

        if (SOPC_Null_PublisherId == pubId->type)
        {
            assert(SOPC_PubSubConnection_Nb_WriterGroup(connection) == 0);
            // SOPC_Null_PublisherId pubId => Subscriber connection case

            uint16_t nbReaderGroups = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
            if (nbReaderGroups > 0)
            {
                schedulerCtx.transport[iIter].connection = connection;

                printf("# Add input with context %08lx\r\n", (uint64_t) connection);

                status = SOPC_RT_Subscriber_Initializer_AddInput(pRTInitializer,                        //
                                                                 SOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER, //
                                                                 SOPC_PUBSUB_BUFFER_SIZE,               //
                                                                 SOPC_PIN_MODE_GET_NORMAL,              //
                                                                 (void*) connection,
                                                                 &schedulerCtx.transport[iIter].inputNumber); //

                if (SOPC_STATUS_OK != status)
                {
                    printf("# RT Subscriber initializer add input failed !!!\n");
                    result = false;
                }
                else
                {
                    printf("# RT Subscriber initializer add input %u\r\n", schedulerCtx.transport[iIter].inputNumber);
                }

                if (result)
                {
                    // Parse connection multicast address
                    const char* address = SOPC_PubSubConnection_Get_Address(connection);

                    SOPC_PubSubProtocol_Type protocol = SOPC_PubSub_Protocol_From_URI(address);

                    SOPC_Socket_AddressInfo* multicastAddr = NULL;
                    SOPC_Socket_AddressInfo* localAddr = NULL;

                    switch (protocol)
                    {
                    case SOPC_PubSubProtocol_UDP:
                        result =
                            SOPC_PubSubHelpers_Subscriber_ParseMulticastAddress(address, &multicastAddr, &localAddr);

                        // Create reception socket
                        if (result)
                        {
                            Socket* sock = &schedulerCtx.transport[iIter].sock;
                            schedulerCtx.transport[iIter].fctClear = &SOPC_SubScheduler_CtxUdp_Clear;
                            schedulerCtx.transport[iIter].protocol = SOPC_PubSubProtocol_UDP;

                            schedulerCtx.nbSockets++;
                            status = SOPC_UDP_Socket_CreateToReceive(multicastAddr, true, sock);
                            // Add socket to multicast group
                            if (SOPC_STATUS_OK == status)
                            {
                                status = SOPC_UDP_Socket_AddMembership(*sock, multicastAddr, localAddr);
                            }
                            else
                            {
                                status = SOPC_STATUS_NOK;
                            }
                        }
                        else
                        {
                            status = SOPC_STATUS_INVALID_PARAMETERS;
                        }

                        if (SOPC_STATUS_OK != status)
                        {
                            /* Call uninit because at least one error */
                            result = false;
                        }

                        SOPC_Socket_AddrInfoDelete(&multicastAddr);
                        SOPC_Socket_AddrInfoDelete(&localAddr);
                        break;

                    case SOPC_PubSubProtocol_UNKOWN:
                    default:
                        status = SOPC_STATUS_INVALID_PARAMETERS;
                        result = false;
                    }
                }
            }
        }
    }

    if (result)
    {
        uint32_t out = 0;
        SOPC_RT_Subscriber_Initializer_AddOutput(pRTInitializer, 1, 1, 1, &out);
        status = SOPC_RT_Subscriber_Initialize(schedulerCtx.pRTSubscriber, pRTInitializer);
        if (status != SOPC_STATUS_OK)
        {
            printf("# Rt subscriber initialization failed !!!\r\n");
            result = false;
        }
        else
        {
            printf("# Rt subscriber initialized\r\n");
        }
    }

    if (pRTInitializer != NULL)
    {
        SOPC_RT_Subscriber_Initializer_Destroy(&pRTInitializer);
    }

    // Create reception buffer
    if (result)
    {
        bool newValue = false;
        __atomic_store(&schedulerCtx.bQuitSubcriberBeatHeart, &newValue, __ATOMIC_SEQ_CST);
        status = SOPC_Thread_Create(&schedulerCtx.handleRTSubscriberBeatHeart, //
                                    cbBeatHeartThreadCallback,                 //
                                    NULL,                                      //
                                    "SubHeart");                               //

        if (SOPC_STATUS_OK != status)
        {
            result = false;
            printf("# Error creation of rt subscriber beat heart thread\r\n");
        }
        else
        {
            printf("# Rt subscriber beat heart thread created\r\n");
        }
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
    assert(NULL == schedulerCtx.sockArray && NULL == schedulerCtx.sockInputNumbers);

    uint16_t nb_socket = schedulerCtx.nbSockets;
    schedulerCtx.sockArray = SOPC_Calloc(nb_socket, sizeof(*schedulerCtx.sockArray));
    schedulerCtx.sockInputNumbers = SOPC_Calloc(nb_socket, sizeof(uint32_t*));
    if (NULL == schedulerCtx.sockArray || NULL == schedulerCtx.sockInputNumbers)
    {
        SOPC_Free(schedulerCtx.sockArray);
        SOPC_Free(schedulerCtx.sockInputNumbers);
        return false;
    }
    uint16_t sockIdx = 0;
    // Initialize the subscriber scheduler context: create socket + associated Sub connection config
    for (uint32_t iIter = 0; iIter < schedulerCtx.nbConnections; iIter++)
    {
        if (SOPC_PubSubProtocol_UDP == schedulerCtx.transport[iIter].protocol)
        {
            schedulerCtx.sockArray[sockIdx] = schedulerCtx.transport[iIter].sock;
            schedulerCtx.sockInputNumbers[sockIdx] =
                &schedulerCtx.transport[iIter].inputNumber; // Connection input number
            sockIdx++;
        }
    }

    assert(nb_socket == sockIdx);
    SOPC_UDP_SocketsMgr_Initialize((void**) schedulerCtx.sockInputNumbers, schedulerCtx.sockArray, nb_socket,
                                   on_udp_message_received, NULL, NULL);

    return true;
}

static void SOPC_SubScheduler_CtxUdp_Clear(SOPC_SubScheduler_TransportCtx* ctx)
{
    SOPC_UDP_Socket_Close(&(ctx->sock));
}
