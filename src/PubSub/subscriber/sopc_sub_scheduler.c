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
#include <stdbool.h>

#include "sopc_array.h"
#include "sopc_atomic.h"
#include "sopc_crypto_provider.h"
#include "sopc_eth_sockets.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mqtt_transport_layer.h"
#include "sopc_mutexes.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_reader_layer.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_sockets_mgr.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

char* ENDPOINT_URL = NULL;

/********************************************************************************************************
 * SUBSCRIBER SECURITY CONTEXT
 * This module uses a specific context for security management.
 * A callback is given to the Network Message function to retrieve
 *   Security mode, sequence number and Group keys from information received in a message
 *
 * Entry Points are
 *  - SOPC_SubScheduler_Add_Security_Ctx
 *  - SOPC_SubScheduler_Get_Security_Infos
 ********************************************************************************************************/

/**
 * Add data related to a Reader Group in Susbcriber security context
 */
static void SOPC_SubScheduler_Add_Security_Ctx(SOPC_ReaderGroup* group);

/**
 * Search in Subscriber security context the data associated to a token id, publisher id and writer group id
 * If no context is found, it means the subscriber is not configured to manage security from this parameter.
 * This function is given as callback to the Network Message decoder.
 *
 * \param tokenId tokenId of a received message
 * \param pubCtx a context related to a Publisher found in the Subscriber security context
 * \param writerGroupId writer group id of a received message
 * \return security data or NULL if not found
 */
static SOPC_PubSub_SecurityType* SOPC_SubScheduler_Get_Security_Infos(uint32_t tokenId,
                                                                      const SOPC_Conf_PublisherId pubId,
                                                                      uint16_t writerGroupId);

/**
 * Structure to link a key (publisher, writer group) to a security configuration:
 */
typedef struct SOPC_SubScheduler_Security_Reader_Ctx
{
    /* Writer Group associated to the Reader.
       Writer Group is used as key to retrieve the security infos since it defines the Security Group
       Reader Groups and their filtered Writer Group ( through DataSetReader ) should have same Security Mode
    */
    uint16_t writerGroupId;
    SOPC_PubSub_SecurityType security;
} SOPC_SubScheduler_Security_Reader_Ctx;

/**
 * Context related to publisher of configured messages.
 * These structure links a publisher id to a sequence number and a list of the security configuration.
 *  - pubId : Publisher id to identify this Publisher context
 *  - currentSequenceNumber : last processed sequence number of this publisher
 *  - readers : array of SOPC_SubScheduler_Security_Reader_Ctx containing writer group and security information
 */
typedef struct SOPC_SubScheduler_Security_Pub_Ctx
{
    SOPC_Conf_PublisherId pubId;
    uint32_t currentSequenceNumber; // TODO : for future version, it should be an array, one per token id
    SOPC_Array* readers;            // SOPC_SubScheduler_Security_Reader_Ctx
} SOPC_SubScheduler_Security_Pub_Ctx;

/**
 * Create of Publisher Context or NULL if out of memory.
 * Returned object should be cleared with SOPC_SubScheduler_Pub_Ctx_Clear then freed with SOPC_Free
 */
static SOPC_SubScheduler_Security_Pub_Ctx* SOPC_SubScheduler_Pub_Ctx_Create(const SOPC_Conf_PublisherId* pubId);

/**
 * Free of Publisher Context
 */
static void SOPC_SubScheduler_Pub_Ctx_Clear(SOPC_SubScheduler_Security_Pub_Ctx* ctx);

/**
 * Search in Subscriber security context the data associated to a token id and a publisher.
 * If no context is found, it means the subscriber is not configured to manage security from this publisher
 *
 * \param tokenId tokenId of a received message
 * \param tokenId publisher id of a received message
 * \return a context related to a publisher or NULL if not found
 */
static SOPC_SubScheduler_Security_Pub_Ctx* SOPC_SubScheduler_Get_Security_Pub_Ctx(uint32_t tokenId,
                                                                                  const SOPC_Conf_PublisherId pubId);

/**
 * Create of Reader Context or NULL if out of memory.
 */
static SOPC_SubScheduler_Security_Reader_Ctx* SOPC_SubScheduler_Reader_Ctx_Create(const SOPC_Conf_PublisherId* pubId,
                                                                                  uint16_t writerGroupId,
                                                                                  SOPC_SecurityMode_Type mode);

/**
 * Search in Subscriber security context the data associated to a writer group id
 * If no context is found, it means the subscriber is not configured to manage security from this publisher
 *
 * \param pubCtx a context related to a Publisher found in the Subscriber security context
 * \param writerGroupId writer group id of a received message
 * \return a context related to a DataSet Reader or NULL if not found
 */
static SOPC_SubScheduler_Security_Reader_Ctx* SOPC_SubScheduler_Pub_Ctx_Get_Reader_Ctx(
    SOPC_SubScheduler_Security_Pub_Ctx* pubCtx,
    uint16_t writerGroupId);

// END SUBSCRIBER SECURITY CONTEXT

static bool SOPC_SubScheduler_Start_Sockets(int threadPriority);

/* Transport context. One per connection */
typedef struct SOPC_SubScheduler_TransportCtx SOPC_SubScheduler_TransportCtx;

// Function to clear a transport context. To be implemented for each protocol
typedef void (*SOPC_SubScheduler_TransportCtx_Clear)(SOPC_SubScheduler_TransportCtx*);

static void SOPC_SubScheduler_CtxUdp_Clear(SOPC_SubScheduler_TransportCtx*);

/* Callback defined to release mqtt transport sync context */
static void SOPC_SubScheduler_CtxMqtt_Clear(SOPC_SubScheduler_TransportCtx*);

/* Callback defined to release ethernet transport context */
static void SOPC_SubScheduler_CtxEth_Clear(SOPC_SubScheduler_TransportCtx*);

struct SOPC_SubScheduler_TransportCtx
{
    SOPC_PubSubProtocol_Type protocol;
    SOPC_PubSubConnection* connection;
    SOPC_SubScheduler_TransportCtx_Clear fctClear;

    // specific to SOPC_PubSubProtocol_UDP
    Socket sock;

    // specific to SOPC_PubSubProtocol_MQTT
    MqttTransportHandle* mqttHandle;

    // specific to SOPC_PubSubProtocol_ETH
    SOPC_ETH_Socket_ReceiveAddressInfo* ethAddr;
};

static struct
{
    /* Managing start / stop phase */
    /* TODO: enum states, factorize with pub */
    int32_t isStarted;
    int32_t processingStartStop;

    /* Input parameters */
    SOPC_PubSubConfiguration* config;
    SOPC_SubTargetVariableConfig* targetConfig;
    SOPC_SubscriberStateChanged_Func* pStateCallback;

    /* Internal context */
    SOPC_PubSubState state;

    SOPC_Buffer* receptionBufferSockets; /*For all socket connections (UDP / raw Ethernet)*/
    SOPC_Buffer* receptionBufferMQTT;    /*For all MQTT connections*/

    uint32_t nbConnections;
    SOPC_SubScheduler_TransportCtx* transport;

    // specific to SOPC_PubSubProtocol_UDP
    uint16_t nbSockets;
    uint16_t nbMqttTransportContext;
    Socket* sockArray;

    /* UADP Security */
    // Array of SOPC_SubScheduler_Security_Pub_Ctx
    // One element by publisher id : actually same key and same group for all module
    // TODO future version, it should contain one element per (publisherid, tokenid)
    SOPC_Array* securityCtx;
} schedulerCtx = {.isStarted = false,
                  .processingStartStop = false,

                  .config = NULL,
                  .targetConfig = NULL,
                  .pStateCallback = NULL,

                  .state = SOPC_PubSubState_Disabled,

                  .receptionBufferSockets = NULL,
                  .receptionBufferMQTT = NULL,

                  .nbConnections = 0,
                  .transport = NULL,

                  .nbSockets = 0,
                  .nbMqttTransportContext = 0,
                  .sockArray = NULL,

                  .securityCtx = NULL};

static void set_new_state(SOPC_PubSubState new)
{
    SOPC_PubSubState prev = schedulerCtx.state;
    if ((prev != new) && (NULL != schedulerCtx.pStateCallback))
    {
        schedulerCtx.pStateCallback(new);
    }
    schedulerCtx.state = new;
}

/* The generic callback that decode message and call the configuration-defined callback SetVariables */
static SOPC_ReturnStatus on_message_received(SOPC_PubSubConnection* pDecoderContext,
                                             SOPC_PubSubState state,
                                             SOPC_Buffer* buffer,
                                             SOPC_SubTargetVariableConfig* config);

/* The callback for received messages specific to sockets (UDP or raw Ethernet) transport */
static void on_socket_message_received(void* pInputIdentifier, Socket sock);

/** \brief The callback for received messages specific to the MQTT transport
 *
 * \param pCtx  Transport context handle
 * \param data  Pointer to received data
 * \param size  Size of data received, in bytes
 * \param pInputIdentifier  User context identifying the connection
 */
static void on_mqtt_message_received(MqttTransportHandle* pCtx, uint8_t* data, uint16_t size, void* pInputIdentifier)
{
    SOPC_UNUSED_ARG(pCtx);
    assert(NULL != pInputIdentifier);

    if (schedulerCtx.receptionBufferMQTT != NULL && size < SOPC_PUBSUB_BUFFER_SIZE)
    {
        memcpy(schedulerCtx.receptionBufferMQTT->data, data, size);
        SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(schedulerCtx.receptionBufferMQTT, 0);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetDataLength(schedulerCtx.receptionBufferMQTT, size);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = on_message_received((SOPC_PubSubConnection*) pInputIdentifier, schedulerCtx.state,
                                         schedulerCtx.receptionBufferMQTT, schedulerCtx.targetConfig);
            SOPC_UNUSED_ARG(status);
        }
    }
}

static void on_socket_message_received(void* pInputIdentifier, Socket sock)
{
    assert(NULL != pInputIdentifier);
    SOPC_SubScheduler_TransportCtx* transportCtx = pInputIdentifier;
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(schedulerCtx.receptionBufferSockets, 0);
    if (SOPC_STATUS_OK != status)
    {
        return;
    }

    switch (transportCtx->protocol)
    {
    case SOPC_PubSubProtocol_UDP:
        status = SOPC_UDP_Socket_ReceiveFrom(sock, schedulerCtx.receptionBufferSockets);
        break;
    case SOPC_PubSubProtocol_ETH:
        status = SOPC_ETH_Socket_ReceiveFrom(sock, transportCtx->ethAddr, true, ETH_ETHERTYPE,
                                             schedulerCtx.receptionBufferSockets);
        if (SOPC_STATUS_OK == status)
        {
            // Set position after the ethernet header to reach the UADP encoded message
            status = SOPC_Buffer_SetPosition(schedulerCtx.receptionBufferSockets, ETHERNET_HEADER_SIZE);
        }
        break;
    default:
        assert(false && "Unexpected protocol for reception on socket");
        break;
    }

    // Write input
    if (SOPC_STATUS_OK == status)
    {
        status = on_message_received(transportCtx->connection, schedulerCtx.state, schedulerCtx.receptionBufferSockets,
                                     schedulerCtx.targetConfig);
    }
}

static SOPC_ReturnStatus on_message_received(SOPC_PubSubConnection* pDecoderContext,
                                             SOPC_PubSubState state,
                                             SOPC_Buffer* buffer,
                                             SOPC_SubTargetVariableConfig* config)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == pDecoderContext)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_PubSubState_Operational == state)
    {
        /* TODO: have a more resilient behavior and avoid stopping the subscriber because of
         *  random bytes found on the network */
        result = SOPC_Reader_Read_UADP(pDecoderContext, buffer, config, SOPC_SubScheduler_Get_Security_Infos);

        if (SOPC_STATUS_ENCODING_ERROR == result)
        {
            /* TODO: we can't log systematic decoding errors,
             *  because the volume of these errors is not known */
            result = SOPC_STATUS_OK;
        }
        else if (SOPC_STATUS_OK != result)
        {
            set_new_state(SOPC_PubSubState_Error);
        }
    }

    return result;
}

static void uninit_sub_scheduler_ctx(void)
{
    schedulerCtx.config = NULL;
    schedulerCtx.targetConfig = NULL;
    schedulerCtx.pStateCallback = NULL;

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Stop Subscriber thread");
    for (uint32_t i = 0; i < schedulerCtx.nbConnections; i++)
    {
        if (schedulerCtx.transport[i].fctClear != NULL)
        {
            schedulerCtx.transport[i].fctClear(&schedulerCtx.transport[i]);
        }
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB,
                              "Transport context freed for connection #%" PRIu32 " (subscriber)", i);
    }
    schedulerCtx.nbConnections = 0;
    schedulerCtx.nbSockets = 0;
    if (NULL != schedulerCtx.transport)
    {
        SOPC_Free(schedulerCtx.transport);
        schedulerCtx.transport = NULL;
    }
    if (NULL != schedulerCtx.sockArray)
    {
        SOPC_Free(schedulerCtx.sockArray);
        schedulerCtx.sockArray = NULL;
    }
    SOPC_Buffer_Delete(schedulerCtx.receptionBufferSockets);
    SOPC_Buffer_Delete(schedulerCtx.receptionBufferMQTT);
    schedulerCtx.receptionBufferSockets = NULL;
    schedulerCtx.receptionBufferMQTT = NULL;

    size_t size = SOPC_Array_Size(schedulerCtx.securityCtx);
    for (size_t i = 0; i < size; i++)
    {
        SOPC_SubScheduler_Security_Pub_Ctx* ctx =
            SOPC_Array_Get(schedulerCtx.securityCtx, SOPC_SubScheduler_Security_Pub_Ctx*, i);
        SOPC_SubScheduler_Pub_Ctx_Clear(ctx);
        SOPC_Free(ctx);
    }
    SOPC_Array_Delete(schedulerCtx.securityCtx);

    schedulerCtx.securityCtx = NULL;
}

static SOPC_ReturnStatus init_sub_scheduler_ctx(SOPC_PubSubConfiguration* config,
                                                SOPC_SubTargetVariableConfig* targetConfig,
                                                SOPC_SubscriberStateChanged_Func* pStateChangedCb)
{
    uint32_t nb_connections = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    assert(nb_connections > 0);

    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    schedulerCtx.config = config;
    schedulerCtx.targetConfig = targetConfig;
    schedulerCtx.pStateCallback = pStateChangedCb;

    schedulerCtx.receptionBufferSockets = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
    result = (NULL != schedulerCtx.receptionBufferSockets);

    schedulerCtx.receptionBufferMQTT = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
    result = (NULL != schedulerCtx.receptionBufferMQTT);

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

    if (result)
    {
        schedulerCtx.securityCtx = SOPC_Array_Create(sizeof(SOPC_SubScheduler_Security_Pub_Ctx*),
                                                     SOPC_PUBSUB_MAX_PUBLISHER_PER_SCHEDULER, NULL);

        result = (NULL != schedulerCtx.securityCtx);
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

                if (result)
                {
                    // Parse connection multicast address
                    const char* address = SOPC_PubSubConnection_Get_Address(connection);

                    SOPC_PubSubProtocol_Type protocol = SOPC_PubSub_Protocol_From_URI(address);

                    SOPC_Socket_AddressInfo* multicastAddr = NULL;
                    SOPC_Socket_AddressInfo* localAddr = NULL;
                    MqttManagerHandle* handleMqttMgr = NULL;
                    size_t hostnameLength = 0;
                    size_t portIdx = 0;
                    size_t portLength = 0;

                    switch (protocol)
                    {
                    case SOPC_PubSubProtocol_UDP:
                        result =
                            SOPC_PubSubHelpers_Subscriber_ParseMulticastAddressUDP(address, &multicastAddr, &localAddr);

                        // Create reception socket
                        if (result)
                        {
                            schedulerCtx.transport[iIter].fctClear = &SOPC_SubScheduler_CtxUdp_Clear;
                            schedulerCtx.transport[iIter].protocol = SOPC_PubSubProtocol_UDP;

                            schedulerCtx.nbSockets++;
                            status = SOPC_UDP_Socket_CreateToReceive(
                                multicastAddr, SOPC_PubSubConnection_Get_InterfaceName(connection), true, true,
                                &schedulerCtx.transport[iIter].sock);
                            // Add socket to multicast group
                            if (SOPC_STATUS_OK == status)
                            {
                                status = SOPC_UDP_Socket_AddMembership(
                                    schedulerCtx.transport[iIter].sock,
                                    SOPC_PubSubConnection_Get_InterfaceName(connection), multicastAddr, localAddr);
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
                    case SOPC_PubSubProtocol_MQTT:
                    {
                        handleMqttMgr = SOPC_PubSub_Protocol_GetMqttManagerHandle();

                        if (handleMqttMgr == NULL)
                        {
                            status = SOPC_STATUS_NOK;
                        }
                        else
                        {
                            if (SOPC_Helper_URI_ParseUri_WithPrefix(MQTT_PREFIX, address, &hostnameLength, &portIdx,
                                                                    &portLength) == false)
                            {
                                status = SOPC_STATUS_NOK;
                            }
                            else
                            {
                                schedulerCtx.transport[iIter].mqttHandle = SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(
                                    handleMqttMgr, &address[strlen(MQTT_PREFIX)], MQTT_LIB_TOPIC_NAME,
                                    on_mqtt_message_received, schedulerCtx.transport[iIter].connection);

                                if (schedulerCtx.transport[iIter].mqttHandle == NULL)
                                {
                                    status = SOPC_STATUS_NOK;
                                }
                                else
                                {
                                    schedulerCtx.transport[iIter].fctClear = SOPC_SubScheduler_CtxMqtt_Clear;
                                    schedulerCtx.transport[iIter].protocol = SOPC_PubSubProtocol_MQTT;
                                    schedulerCtx.transport[iIter].sock = -1;
                                }
                            }
                        }

                        if (SOPC_STATUS_OK != status)
                        {
                            schedulerCtx.transport[iIter].fctClear = NULL;
                            schedulerCtx.transport[iIter].protocol = SOPC_PubSubProtocol_UNKOWN;
                            schedulerCtx.transport[iIter].sock = -1;
                            /* Call uninit because at least one error */
                            result = false;
                        }
                    }
                    break;
                    case SOPC_PubSubProtocol_ETH:
                        // Note: configured as multicast and without source address check
                        status = SOPC_ETH_Socket_CreateReceiveAddressInfo(
                            SOPC_PubSubConnection_Get_InterfaceName(connection), true, address + strlen(ETH_PREFIX),
                            NULL, &schedulerCtx.transport[iIter].ethAddr);

                        // Create reception socket
                        if (SOPC_STATUS_OK == status)
                        {
                            status = SOPC_ETH_Socket_CreateToReceive(schedulerCtx.transport[iIter].ethAddr, true,
                                                                     &schedulerCtx.transport[iIter].sock);
                        }

                        if (SOPC_STATUS_OK == status)
                        {
                            schedulerCtx.transport[iIter].fctClear = &SOPC_SubScheduler_CtxEth_Clear;
                            schedulerCtx.transport[iIter].protocol = SOPC_PubSubProtocol_ETH;
                            schedulerCtx.nbSockets++;
                        }
                        else
                        {
                            SOPC_Logger_TraceError(
                                SOPC_LOG_MODULE_PUBSUB,
                                "error configuring the Subscriber Ethernet socket: check it is run in SUDO mode, "
                                "check address format is only with hyphens and zero-terminated.");
                            /* Call uninit because at least one error */
                            SOPC_SubScheduler_CtxEth_Clear(&schedulerCtx.transport[iIter]);
                            result = false;
                        }

                        break;
                    case SOPC_PubSubProtocol_UNKOWN:
                    default:
                        status = SOPC_STATUS_INVALID_PARAMETERS;
                        result = false;
                    }
                }
            }

            if (result)
            {
                // add security context
                uint16_t nbGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
                for (uint16_t rg_i = 0; rg_i < nbGroup; rg_i++)
                {
                    SOPC_ReaderGroup* group = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, rg_i);
                    SOPC_SubScheduler_Add_Security_Ctx(group);
                }
            }
        }
    }

    if (false == result)
    {
        uninit_sub_scheduler_ctx();
        assert(SOPC_STATUS_OK != status);
    }

    return status;
}

bool SOPC_SubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_SubTargetVariableConfig* targetConfig,
                             SOPC_SubscriberStateChanged_Func* pStateChangedCb,
                             int threadPriority)
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
        SOPC_ReturnStatus status = init_sub_scheduler_ctx(config, targetConfig, pStateChangedCb);
        if (SOPC_STATUS_OK == status)
        {
            assert(schedulerCtx.nbConnections <= UINT16_MAX);
            // Run the socket manager with the context
            if (0 < schedulerCtx.nbSockets)
            {
                result = SOPC_SubScheduler_Start_Sockets(threadPriority);
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
    SOPC_Sub_SocketsMgr_Clear();
    set_new_state(SOPC_PubSubState_Disabled);
    uninit_sub_scheduler_ctx();

    SOPC_Atomic_Int_Set(&schedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&schedulerCtx.processingStartStop, false);
}

/*
  precondition :
   - call init_sub_scheduler_ctx
   - schedulerCtx.nbSockets > 0
   - schedulerCtx.sockArray is NULL
*/
static bool SOPC_SubScheduler_Start_Sockets(int threadPriority)
{
    assert(0 < schedulerCtx.nbSockets);
    assert(NULL == schedulerCtx.sockArray);

    uint16_t nb_socket = schedulerCtx.nbSockets;
    schedulerCtx.sockArray = SOPC_Calloc(nb_socket, sizeof(*schedulerCtx.sockArray));
    if (NULL == schedulerCtx.sockArray)
    {
        SOPC_Free(schedulerCtx.sockArray);
        return false;
    }
    uint16_t sockIdx = 0;

    /* TODO: this is not the socket creation step,
     *  socket are already stored in the transport context,
     *  array length should be size_t,
     *  -> remove this code, only sockets_mgr use these arrays, and it only borrows them */
    // Initialize the subscriber scheduler context: create socket + associated Sub connection config
    for (uint32_t iIter = 0; iIter < schedulerCtx.nbConnections; iIter++)
    {
        if (SOPC_PubSubProtocol_UDP == schedulerCtx.transport[iIter].protocol ||
            SOPC_PubSubProtocol_ETH == schedulerCtx.transport[iIter].protocol)
        {
            schedulerCtx.sockArray[sockIdx] = schedulerCtx.transport[iIter].sock;
            sockIdx++;
        }
    }

    assert(nb_socket == sockIdx);
    SOPC_Sub_SocketsMgr_Initialize((void*) schedulerCtx.transport, sizeof(*schedulerCtx.transport),
                                   schedulerCtx.sockArray, nb_socket, on_socket_message_received, NULL, NULL,
                                   threadPriority);

    return true;
}

static void SOPC_SubScheduler_CtxUdp_Clear(SOPC_SubScheduler_TransportCtx* ctx)
{
    SOPC_UDP_Socket_Close(&(ctx->sock));
}

static void SOPC_SubScheduler_CtxMqtt_Clear(SOPC_SubScheduler_TransportCtx* ctx)
{
    if (ctx != NULL && ctx->mqttHandle != NULL)
    {
        SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&(ctx->mqttHandle));
        ctx->mqttHandle = NULL;
    }
}

static void SOPC_SubScheduler_CtxEth_Clear(SOPC_SubScheduler_TransportCtx* ctx)
{
    SOPC_ETH_Socket_Close(&(ctx->sock));
    SOPC_Free(ctx->ethAddr);
    ctx->ethAddr = NULL;
}

/********************************************************************************************************
 * SUBSCRIBER SECURITY CONTEXT
 * Implementation
 ********************************************************************************************************/

static SOPC_SubScheduler_Security_Pub_Ctx* SOPC_SubScheduler_Get_Security_Pub_Ctx(uint32_t tokenId,
                                                                                  const SOPC_Conf_PublisherId pubId)
{
    if (SOPC_PUBSUB_SKS_DEFAULT_TOKENID != tokenId)
    {
        // only one token id is managed
        return NULL;
    }
    // only Integer publisher id is managed
    assert(SOPC_UInteger_PublisherId == pubId.type);
    // get keys
    size_t size = SOPC_Array_Size(schedulerCtx.securityCtx);
    for (size_t i = 0; i < size; i++)
    {
        SOPC_SubScheduler_Security_Pub_Ctx* ctx =
            SOPC_Array_Get(schedulerCtx.securityCtx, SOPC_SubScheduler_Security_Pub_Ctx*, i);
        if (ctx->pubId.type == pubId.type && ctx->pubId.data.uint == pubId.data.uint)
        {
            return ctx;
        }
    }
    return NULL;
}

static SOPC_PubSub_SecurityType* SOPC_SubScheduler_Get_Security_Infos(uint32_t tokenId,
                                                                      const SOPC_Conf_PublisherId pubId,
                                                                      uint16_t writerGroupId)
{
    SOPC_SubScheduler_Security_Pub_Ctx* pubCtx = SOPC_SubScheduler_Get_Security_Pub_Ctx(tokenId, pubId);
    if (NULL == pubCtx)
    {
        // no security context associated to this publisher
        // bad configuration or message not for this subscriber
        return NULL;
    }
    SOPC_SubScheduler_Security_Reader_Ctx* readerCtx = SOPC_SubScheduler_Pub_Ctx_Get_Reader_Ctx(pubCtx, writerGroupId);
    if (NULL == readerCtx)
    {
        // no security context associated to this writer group
        // bad configuration or message not for this subscriber
        return NULL;
    }
    return &readerCtx->security;
}

static void SOPC_SubScheduler_Add_Security_Ctx(SOPC_ReaderGroup* group)
{
    assert(NULL != group);
    if (SOPC_SecurityMode_Invalid == SOPC_ReaderGroup_Get_SecurityMode(group) ||
        SOPC_SecurityMode_None == SOPC_ReaderGroup_Get_SecurityMode(group))
    {
        // security context not needed
        return;
    }

    assert(NULL != schedulerCtx.securityCtx);

    // Create a new sub security context for each publisher id managed by this reader group
    const SOPC_Conf_PublisherId* pubId = SOPC_ReaderGroup_Get_PublisherId(group);
    assert(NULL != pubId); // Reader without publisher id are not managed

    // check the publisher id is already registered.
    SOPC_SubScheduler_Security_Pub_Ctx* pubCtx =
        SOPC_SubScheduler_Get_Security_Pub_Ctx(SOPC_PUBSUB_SKS_DEFAULT_TOKENID, *pubId);

    if (NULL == pubCtx)
    {
        // not found, create a new one
        pubCtx = SOPC_SubScheduler_Pub_Ctx_Create(pubId);
        if (NULL != pubCtx)
        {
            SOPC_UNUSED_RESULT(SOPC_Array_Append(schedulerCtx.securityCtx, pubCtx));
        }
    }

    if (NULL != pubCtx)
    {
        // check if the writer group is already registered.
        uint16_t writerGroupId = SOPC_ReaderGroup_Get_GroupId(group);
        SOPC_SubScheduler_Security_Reader_Ctx* readerCtx =
            SOPC_SubScheduler_Pub_Ctx_Get_Reader_Ctx(pubCtx, writerGroupId);

        if (NULL == readerCtx)
        {
            // not found, create a new one
            readerCtx =
                SOPC_SubScheduler_Reader_Ctx_Create(pubId, writerGroupId, SOPC_ReaderGroup_Get_SecurityMode(group));
            if (NULL != readerCtx)
            {
                SOPC_UNUSED_RESULT(SOPC_Array_Append(pubCtx->readers, readerCtx));
            }
        }
    }
}

static void SOPC_SubScheduler_Pub_Ctx_Clear(SOPC_SubScheduler_Security_Pub_Ctx* ctx)
{
    size_t size = SOPC_Array_Size(ctx->readers);
    for (size_t i = 0; i < size; i++)
    {
        SOPC_SubScheduler_Security_Reader_Ctx* readerCtx =
            SOPC_Array_Get(ctx->readers, SOPC_SubScheduler_Security_Reader_Ctx*, i);
        SOPC_PubSub_Security_Clear(&readerCtx->security);
        SOPC_Free(readerCtx);
    }
    SOPC_Array_Delete(ctx->readers);
    ctx->readers = NULL;
}

static SOPC_SubScheduler_Security_Reader_Ctx* SOPC_SubScheduler_Reader_Ctx_Create(const SOPC_Conf_PublisherId* pubId,
                                                                                  uint16_t writerGroupId,
                                                                                  SOPC_SecurityMode_Type mode)
{
    SOPC_SubScheduler_Security_Reader_Ctx* ctx = SOPC_Calloc(1, sizeof(SOPC_SubScheduler_Security_Reader_Ctx));
    if (NULL == ctx)
    {
        return NULL;
    }
    // Init Key
    assert(NULL != pubId && SOPC_UInteger_PublisherId == pubId->type); // String pub id not managed
    ctx->writerGroupId = writerGroupId;

    // Init Security Infos
    assert(SOPC_SecurityMode_Invalid != mode && SOPC_SecurityMode_None != mode);
    ctx->security.mode = mode;
    ctx->security.sequenceNumber = 0;
    ctx->security.groupKeys = SOPC_LocalSKS_GetSecurityKeys(SOPC_PUBSUB_SKS_DEFAULT_GROUPID, 0);
    ctx->security.provider = SOPC_CryptoProvider_CreatePubSub(SOPC_PUBSUB_SECURITY_POLICY);
    if (NULL == ctx->security.provider || NULL == ctx->security.groupKeys)
    {
        SOPC_PubSub_Security_Clear(&ctx->security);
        SOPC_Free(ctx);
        ctx = NULL;
    }
    return ctx;
}

static SOPC_SubScheduler_Security_Reader_Ctx* SOPC_SubScheduler_Pub_Ctx_Get_Reader_Ctx(
    SOPC_SubScheduler_Security_Pub_Ctx* pubCtx,
    uint16_t writerGroupId)
{
    assert(NULL != pubCtx);
    size_t size = SOPC_Array_Size(pubCtx->readers);
    for (size_t i = 0; i < size; i++)
    {
        SOPC_SubScheduler_Security_Reader_Ctx* readerCtx =
            SOPC_Array_Get(pubCtx->readers, SOPC_SubScheduler_Security_Reader_Ctx*, i);
        assert(NULL != readerCtx);
        if (readerCtx->writerGroupId == writerGroupId)
        {
            return readerCtx;
        }
    }
    return NULL;
}

static SOPC_SubScheduler_Security_Pub_Ctx* SOPC_SubScheduler_Pub_Ctx_Create(const SOPC_Conf_PublisherId* pubId)
{
    assert(NULL != pubId && SOPC_UInteger_PublisherId == pubId->type); // String pub id not managed

    SOPC_SubScheduler_Security_Pub_Ctx* ctx = SOPC_Calloc(1, sizeof(SOPC_SubScheduler_Security_Pub_Ctx));
    if (NULL == ctx)
    {
        return NULL;
    }
    ctx->pubId = *pubId;
    ctx->currentSequenceNumber = 0;
    ctx->readers =
        SOPC_Array_Create(sizeof(SOPC_SubScheduler_Security_Reader_Ctx*), SOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER, NULL);
    if (NULL == ctx->readers)
    {
        SOPC_Free(ctx);
        ctx = NULL;
    }
    return ctx;
}
