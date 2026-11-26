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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "opcua_statuscodes.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_buffer.h"
#include "sopc_common_constants.h"
#include "sopc_date_time.h"
#include "sopc_encoder.h"
#include "sopc_enums.h"
#include "sopc_event_handler.h"
#include "sopc_helper_string.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_missing_c99.h"
#include "sopc_mutexes.h"
#include "sopc_protocol_constants.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_services_api_internal.h"
#include "sopc_services_request_timeout.h"
#include "sopc_time_reference.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include "io_dispatch_mgr.h"
#include "toolkit_header_init.h"
#include "util_b2c.h"
#include "util_write_value_pool.h"

/**
 * \brief Indicates whether the server configuration is locked (configured) or not.
 *        When unlocked, ignore the server related events until it is locked again.
 */
static bool isServerConfigurationLocked = false;

static SOPC_Looper* servicesLooper = NULL;
static bool servicesPreCleared = false; // Only accessed by the thread calling SOPC_Services_PreClear/Clear
static SOPC_EventHandler* secureChannelsEventHandler = NULL;
static SOPC_EventHandler* servicesEventHandler = NULL;

/* Hooks called by the services thread (SOPC_Services_Hooks*): set from any thread, read from the services thread */
static void* servicesHooks = NULL;

// Structure used to close all connections in a synchronous way
// (necessary on toolkit clear)
static struct
{
    SOPC_Mutex mutex;
    SOPC_Condition cond;
    bool allDisconnectedFlag;
    bool requestedFlag;
    bool clientOnlyFlag;
} closeAllConnectionsSync = {.allDisconnectedFlag = false, .requestedFlag = false, .clientOnlyFlag = false};

/** Hysteresis state of a Services queue load shedding category (HIGH enter / LOW leave) */
typedef struct
{
    bool isBusy;
    uint32_t nbRefused;           /**< Requests refused since the busy state was entered */
    SOPC_TimeReference busyStart; /**< Time the busy state was entered */
} SOPC_Services_BusyState;

/** Services queue load shedding states, one per request category */
static SOPC_Services_BusyState servicesBusyLocal = {false, 0, 0};
static SOPC_Services_BusyState servicesBusyExternal = {false, 0, 0};

/**
 * \brief Updates and returns whether the Services looper queues measured by a given category (\p queueSize pending
 *        events) are considered busy. Enter busy at \p high, leave at \p low. \p state holds the category
 *        hysteresis state. A warning is logged on each state change only, the one leaving the busy state reports the
 *        number of requests refused while busy.
 */
static bool SOPC_Services_UpdateQueueBusyState(SOPC_Services_BusyState* state,
                                               uint32_t queueSize,
                                               uint32_t high,
                                               uint32_t low,
                                               const char* label)
{
    if (!state->isBusy)
    {
        if (queueSize >= high)
        {
            state->isBusy = true;
            state->nbRefused = 0;
            state->busyStart = SOPC_TimeReference_GetCurrent();
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "ServicesMgr: Services queue busy for %s (size=%" PRIu32 " >= HIGH=%" PRIu32
                                     "), refusing new requests until size <= LOW=%" PRIu32,
                                     label, queueSize, high, low);
        }
    }
    else if (queueSize <= low)
    {
        state->isBusy = false;
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "ServicesMgr: Services queue no longer busy for %s (size=%" PRIu32 " <= LOW=%" PRIu32
                                 "): %" PRIu32 " requests refused in %" PRIu64 " ms",
                                 label, queueSize, low, state->nbRefused,
                                 (uint64_t)(SOPC_TimeReference_GetCurrent() - state->busyStart));
    }

    return state->isBusy;
}

/** Categories of the Services looper pending events counted for the busy watermarks (see SOPC_EventHandler) */
#define SOPC_SERVICES_PENDING_LOCAL_REQUESTS 0
#define SOPC_SERVICES_PENDING_EXTERNAL_REQUESTS 1

/** \brief ::SOPC_EventHandler_CategoryFct of the Services events: local service requests are counted */
static int32_t SOPC_Services_ServicesEventCategory(int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(params);
    SOPC_UNUSED_ARG(auxParam);
    return (APP_TO_SE_LOCAL_SERVICE_REQUEST == (SOPC_Services_Event) event) ? SOPC_SERVICES_PENDING_LOCAL_REQUESTS
                                                                            : SOPC_EVENT_HANDLER_NO_CATEGORY;
}

/** \brief ::SOPC_EventHandler_CategoryFct of the SecureChannels events: requests received on server side are counted */
static int32_t SOPC_Services_SecureChannelsEventCategory(int32_t event,
                                                         uint32_t eltId,
                                                         uintptr_t params,
                                                         uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(params);
    SOPC_UNUSED_ARG(auxParam);
    return (SC_SERVICE_RCV_REQUEST == (SOPC_SecureChannels_OutputEvent) event) ? SOPC_SERVICES_PENDING_EXTERNAL_REQUESTS
                                                                               : SOPC_EVENT_HANDLER_NO_CATEGORY;
}

/**
 * \brief Updates and returns whether the Services looper queue is considered busy for local (application-originated)
 *        service requests, measured by the number of pending local service requests (prioritized ones included).
 *        Enter busy at ::SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL, leave at ::SOPC_SERVICES_QUEUE_BUSY_LOW_LOCAL.
 */
static bool SOPC_Services_IsQueueBusyLocal(void)
{
    return SOPC_Services_UpdateQueueBusyState(
        &servicesBusyLocal,
        SOPC_EventHandler_Get_PendingCountByCategory(servicesEventHandler, SOPC_SERVICES_PENDING_LOCAL_REQUESTS),
        (uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL, (uint32_t) SOPC_SERVICES_QUEUE_BUSY_LOW_LOCAL, "local");
}

/**
 * \brief Updates and returns whether the Services looper queue is considered busy for external (OPC UA client)
 *        service requests, measured by the number of pending requests received on server side. Enter busy at
 *        ::SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL, leave at ::SOPC_SERVICES_QUEUE_BUSY_LOW_EXTERNAL.
 */
static bool SOPC_Services_IsQueueBusyExternal(void)
{
    return SOPC_Services_UpdateQueueBusyState(
        &servicesBusyExternal,
        SOPC_EventHandler_Get_PendingCountByCategory(servicesEventHandler, SOPC_SERVICES_PENDING_EXTERNAL_REQUESTS),
        (uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL, (uint32_t) SOPC_SERVICES_QUEUE_BUSY_LOW_EXTERNAL,
        "external");
}

/**
 * \brief Frees the local request and returns a ServiceFault with OpcUa_BadServerTooBusy to the application.
 */
static void SOPC_Services_RefuseLocalServiceRequestBusy(uint32_t endpointConfigIdx,
                                                        uintptr_t requestParams,
                                                        uintptr_t appContext)
{
    void* msg = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_EncodeableType* encType = NULL;

    if (NULL != (void*) requestParams)
    {
        encType = *(SOPC_EncodeableType**) requestParams;
        SOPC_EncodeableObject_Delete(encType, (void**) &requestParams);
    }

    status = SOPC_EncodeableObject_Create(&OpcUa_ServiceFault_EncodeableType, &msg);
    if (SOPC_STATUS_OK == status && NULL != msg)
    {
        ((OpcUa_ServiceFault*) msg)->ResponseHeader.ServiceResult = OpcUa_BadServerTooBusy;
    }
    else
    {
        msg = NULL;
    }

    status = SOPC_App_EnqueueComEvent(SE_LOCAL_SERVICE_RESPONSE, endpointConfigIdx, (uintptr_t) msg, appContext);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                           "ServicesMgr: APP_TO_SE_LOCAL_SERVICE_REQUEST refused (server too busy) epCfgIdx=%" PRIu32
                           " msgType=%s ctx=%" PRIuPTR,
                           endpointConfigIdx, SOPC_EncodeableType_GetName(encType), appContext);
}

/**
 * \brief Replies to an external service request with ServiceFault OpcUa_BadServerTooBusy and frees the request buffer.
 */
static void SOPC_Services_RefuseExternalServiceRequestBusy(uint32_t scConnectionIdx,
                                                           SOPC_Buffer* requestBuffer,
                                                           uintptr_t requestId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_EncodeableType* encType = NULL;
    OpcUa_RequestHeader* reqHeader = NULL;
    OpcUa_ResponseHeader respHeader;
    SOPC_Buffer* respBuffer = NULL;
    uint32_t requestHandle = 0;

    SOPC_ASSERT(NULL != requestBuffer);

    status = SOPC_MsgBodyType_Read(requestBuffer, &encType);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_DecodeMsg_HeaderOrBody(requestBuffer, &OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
    }
    if (SOPC_STATUS_OK == status && NULL != reqHeader)
    {
        requestHandle = reqHeader->RequestHandle;
        SOPC_EncodeableObject_Delete(&OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
        reqHeader = NULL;

        OpcUa_ResponseHeader_Initialize(&respHeader);
        respHeader.Timestamp = SOPC_Time_GetCurrentTimeUTC();
        respHeader.RequestHandle = requestHandle;
        respHeader.ServiceResult = OpcUa_BadServerTooBusy;

        // The maximum is the minimum buffer size accepted by OPC UA, thus never above the negotiated send buffer size
        // of the channel
        status = SOPC_EncodeMsg_NewBuffer(SOPC_TCP_UA_MIN_BUFFER_SIZE, &OpcUa_ServiceFault_EncodeableType,
                                          &OpcUa_ResponseHeader_EncodeableType, (void*) &respHeader, NULL, &respBuffer);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, scConnectionIdx, (uintptr_t) respBuffer,
                                                      requestId);
            if (SOPC_STATUS_OK == status)
            {
                respBuffer = NULL; /* ownership transferred to SC layer */
            }
        }
        SOPC_Buffer_Delete(respBuffer);
        OpcUa_ResponseHeader_Clear(&respHeader);
    }

    SOPC_Buffer_Delete(requestBuffer);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: failed to send BadServerTooBusy ServiceFault scIdx=%" PRIu32
                               " reqId=%" PRIuPTR " status=%d",
                               scConnectionIdx, requestId, (int) status);
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SERVICE_RCV_REQUEST refused (server too busy) scIdx=%" PRIu32
                               " reqId=%" PRIuPTR " msgType=%s",
                               scConnectionIdx, requestId, SOPC_EncodeableType_GetName(encType));
    }
}

SOPC_EventHandler* SOPC_Services_GetEventHandler(void)
{
    return servicesEventHandler;
}

static void SOPC_Internal_AllClientSecureChannelsDisconnected(bool clientOnly)
{
    SOPC_Mutex_Lock(&closeAllConnectionsSync.mutex);
    SOPC_ASSERT(closeAllConnectionsSync.clientOnlyFlag == clientOnly);
    if (closeAllConnectionsSync.requestedFlag)
    {
        closeAllConnectionsSync.allDisconnectedFlag = true;
        SOPC_Condition_SignalAll(&closeAllConnectionsSync.cond);
    }
    SOPC_Mutex_Unlock(&closeAllConnectionsSync.mutex);
}

static SOPC_STRONG_INLINE void call_event_treated_hook(void)
{
    SOPC_Services_Hooks* hooks = (SOPC_Services_Hooks*) SOPC_Atomic_Ptr_Get(&servicesHooks);
    if (NULL != hooks && NULL != hooks->eventTreatedCb)
    {
        hooks->eventTreatedCb();
    }
}

static void call_server_stopped_hook(void)
{
    SOPC_Services_Hooks* hooks = (SOPC_Services_Hooks*) SOPC_Atomic_Ptr_Get(&servicesHooks);
    if (NULL != hooks && NULL != hooks->serverStoppedCb)
    {
        hooks->serverStoppedCb();
    }
}

/**
 * \brief Provides a message received on a secure channel to the B model (client side response or server side request)
 *
 * \param scConnectionIdx  secure channel connection index
 * \param buffer           received message buffer, freed by the B model
 * \param requestContext   request Id of a server side request, 0 for a client side response
 * \param receptionTime    time at which the Secure Channels layer provided the message to the Services layer, used to
 *                         evaluate the request header TimeoutHint during the message treatment
 */
static void SOPC_Services_ReceiveMsgBuffer(uint32_t scConnectionIdx,
                                           SOPC_Buffer* buffer,
                                           uintptr_t requestContext,
                                           SOPC_TimeReference receptionTime)
{
    bool bres = false;
    SOPC_Services_SetCurrentMsgReceptionTime(receptionTime);
    io_dispatch_mgr__receive_msg_buffer(scConnectionIdx, (constants__t_byte_buffer_i) buffer,
                                        (constants__t_request_context_i) requestContext, &bres);
    SOPC_Services_ResetCurrentMsgReceptionTime();
    if (!bres)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: message received on scIdx=%" PRIu32 " reqId/0=%" PRIuPTR
                               " considered invalid",
                               scConnectionIdx, requestContext);
    }
}

static void onSecureChannelEvent(SOPC_EventHandler* handler,
                                 int32_t event,
                                 uint32_t id,
                                 uintptr_t params,
                                 uintptr_t auxParam)
{
    SOPC_SecureChannels_OutputEvent scEvent = (SOPC_SecureChannels_OutputEvent) event;
    bool bres = false;
    uint32_t channel_config_idx = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (scEvent)
    {
    case EP_CONNECTED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_EP_SC_CONNECTED epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIuPTR
                               " scIdx=%" PRIuPTR,
                               id, params, auxParam);

        // id ==  endpoint configuration index
        // params = channel configuration index
        // auxParam == connection Id
        SOPC_ASSERT(id <= INT32_MAX);
        channel_config_idx = (uint32_t) params;
        SOPC_ASSERT(channel_config_idx <= constants__t_channel_config_idx_i_max);
        SOPC_ASSERT(auxParam <= constants__t_channel_i_max);

        io_dispatch_mgr__server_channel_connected_event(id, channel_config_idx, (uint32_t) auxParam, &bres);
        if (bres == false)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Services: channel state incoherent or maximum reached epCfgIdx=%" PRIu32
                                   " scIdx=%" PRIuPTR,
                                   id, auxParam);

            SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT, (uint32_t) auxParam,
                                             (uintptr_t) OpcUa_BadSecureChannelClosed, 0);
        }

        break;
    case EP_CLOSED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_EP_CLOSED epCfgIdx=%" PRIu32 " returnStatus=%" PRIuPTR, id, auxParam);
        // id == endpoint configuration index
        // params = NULL
        // auxParam == status
        // => B model entry point to add
        status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, (uintptr_t) NULL, auxParam);
        SOPC_ASSERT(status == SOPC_STATUS_OK);
        break;
    case EP_REVERSE_CLOSED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: EP_REVERSE_CLOSED reverseEpCfgIdx=%" PRIu32 " returnStatus=%" PRIuPTR, id,
                               auxParam);
        //  id = reverse endpoint config index,
        // auxParams = SOPC_ReturnStatus
        status = SOPC_App_EnqueueComEvent(SE_REVERSE_ENDPOINT_CLOSED, id, (uintptr_t) NULL, auxParam);
        SOPC_ASSERT(status == SOPC_STATUS_OK);
        break;
    case SC_CONNECTED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SC_CONNECTED scIdx=%" PRIu32 " scCfgIdx=%" PRIuPTR, id, auxParam);
        // id == connection Id
        // auxParam == secure channel configuration index
        // => B model entry point to add
        SOPC_ASSERT(id <= constants__t_channel_i_max);
        SOPC_ASSERT(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__client_channel_connected_event((uint32_t) auxParam,
                                                        constants__c_reverse_endpoint_config_idx_indet, id);
        break;
    case SC_REVERSE_CONNECTED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_REVERSE_CONNECTED scIdx=%" PRIu32 " scCfgIdx=%" PRIuPTR, id, auxParam);
        // id = secure channel connection index,
        // params = (uint32_t) secure channel configuration index,
        // auxParams = (uint32) reverse endpoint configuration index
        // => B model entry point to add
        SOPC_ASSERT(id <= constants__t_channel_i_max);
        SOPC_ASSERT(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__client_channel_connected_event((uint32_t) params, (uint32_t) auxParam, id);
        break;
    case SC_CONNECTION_TIMEOUT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: SC_SC_CONNECTION_TIMEOUT scCfgIdx=%" PRIu32,
                               id);

        // id == secure channel configuration index
        // => B model entry point to add
        SOPC_ASSERT(id <= constants_bs__t_channel_config_idx_i_max);
        io_dispatch_mgr__client_secure_channel_timeout(id);
        break;
    case SC_DISCONNECTED:
        channel_config_idx = (uint32_t) params;
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SC_DISCONNECTED scIdx=%" PRIu32 " scCfgIdx=%" PRIu32
                               " with status=x%08" PRIX32,
                               id, channel_config_idx, (uint32_t) auxParam);
        // id == connection Id
        // params == secure channel configuration index (server only)
        // auxParam = status
        io_dispatch_mgr__secure_channel_lost(id);
        // Acknowledge the disconnected state is set in service layer to free the connection index
        SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECTED_ACK, id, params, 0);
        break;
    case SC_SERVICE_RCV_REQUEST:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SERVICE_RCV_REQUEST scIdx=%" PRIu32 " reqId=%" PRIuPTR, id, auxParam);

        // id ==  connection Id
        // params = message content (byte buffer)
        // auxParam == requestId (server side, chosen by the client)
        SOPC_ASSERT(NULL != (void*) params);
        // Server side request: shed when the Services queue is busy
        if (SOPC_Services_IsQueueBusyExternal())
        {
            SOPC_Services_RefuseExternalServiceRequestBusy(id, (SOPC_Buffer*) params, auxParam);
            servicesBusyExternal.nbRefused++;
        }
        else
        {
            SOPC_Services_ReceiveMsgBuffer(id, (SOPC_Buffer*) params, auxParam,
                                           SOPC_EventHandler_GetCurrentEventPostTime(handler));
        }
        // params is freed by services manager
        break;
    case SC_SERVICE_RCV_MSG:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: SC_SERVICE_RCV_MSG scIdx=%" PRIu32, id);

        // id ==  connection Id
        // params = message content (byte buffer)
        // auxParam == 0 (client side)
        SOPC_ASSERT(NULL != (void*) params);
        SOPC_Services_ReceiveMsgBuffer(id, (SOPC_Buffer*) params, 0,
                                       SOPC_EventHandler_GetCurrentEventPostTime(handler));
        // params is freed by services manager
        break;
    case SC_SND_FAILURE:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SND_FAILURE scIdx=%" PRIu32 " reqId/Handle=%" PRIuPTR
                               " statusCode=%" PRIXPTR,
                               id, (uintptr_t) params, auxParam);

        constants_statuscodes_bs__t_StatusCode_i statusCode;
        util_status_code__C_to_B((SOPC_StatusCode) auxParam, &statusCode);
        io_dispatch_mgr__snd_msg_failure(id, (constants__t_request_context_i) params, statusCode);
        break;
    case SC_REQUEST_TIMEOUT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_REQUEST_TIMEOUT scIdx=%" PRIu32 " reqHandle=%" PRIuPTR, id, auxParam);

        /* id = secure channel connection index,
           auxParam = request handle */
        SOPC_ASSERT(id <= constants__t_channel_i_max);
        SOPC_ASSERT(auxParam <= SOPC_MAX_PENDING_REQUESTS);
        io_dispatch_mgr__client_request_timeout(id, (uint32_t) auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown event");
    }

    call_event_treated_hook();
}

static void SOPC_Array_Free_WriteDataChanged(void* data)
{
    SOPC_WriteDataChanged* writeDataChanged = (SOPC_WriteDataChanged*) data;
    SOPC_WriteValuePool_ClearAndFree(writeDataChanged->oldValue);
    SOPC_WriteValuePool_ClearAndFree(writeDataChanged->newValue);
}

static void SOPC_Array_Free_NodeChanged(void* data)
{
    SOPC_NodeChanged* nodeChanged = (SOPC_NodeChanged*) data;
    SOPC_NodeId_Clear(nodeChanged->nodeId);
    SOPC_Free(nodeChanged->nodeId);
}

// Frees the parameters of a services event filtered or never treated (see ::SOPC_EventHandler_FreeEventFct)
static void SOPC_Services_FreeServicesEvent(int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(id);
    SOPC_Internal_AsyncSendMsgData* msg_data = NULL;
    SOPC_Internal_EventContext* eventContext = NULL;
    SOPC_Internal_SessionAppContext* sessionContext = NULL;
    SOPC_Internal_DiscoveryContext* discoveryContext = NULL;
    void* msg = (void*) params;

    switch ((SOPC_Services_Event) event)
    {
    case SE_TO_SE_SERVER_DATA_CHANGED:
        SOPC_Array_Set_Free_Func((SOPC_Array*) params, &SOPC_Array_Free_WriteDataChanged);
        SOPC_Array_Delete((SOPC_Array*) params);
        break;
    case SE_TO_SE_SERVER_NODE_CHANGED:
        SOPC_Array_Set_Free_Func((SOPC_Array*) params, &SOPC_Array_Free_NodeChanged);
        SOPC_Array_Delete((SOPC_Array*) params);
        break;
    case SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO:
        msg_data = (SOPC_Internal_AsyncSendMsgData*) params;
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) msg_data->msgToSend, (void**) &msg_data->msgToSend);
        SOPC_Free(msg_data);
        break;
    case APP_TO_SE_LOCAL_SERVICE_REQUEST:
    case APP_TO_SE_SEND_SESSION_REQUEST:
        // Note: the request context is not returned to the application
        if (NULL != msg)
        {
            SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) msg, &msg);
        }
        break;
    case APP_TO_SE_TRIGGER_EVENT:
        eventContext = (SOPC_Internal_EventContext*) params;
        SOPC_ASSERT(NULL != eventContext);
        SOPC_NodeId_Clear(&eventContext->notifierNodeId);
        SOPC_Event_Delete(&eventContext->event);
        SOPC_Free(eventContext);
        break;
    case APP_TO_SE_ACTIVATE_SESSION:
        sessionContext = (SOPC_Internal_SessionAppContext*) auxParam;
        SOPC_ASSERT(NULL != sessionContext);
        SOPC_ExtensionObject_Clear(sessionContext->userToken);
        SOPC_Free(sessionContext->userToken);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(sessionContext->userTokenKey);
        SOPC_Free(sessionContext->sessionName);
        SOPC_Free(sessionContext);
        break;
    case APP_TO_SE_SEND_DISCOVERY_REQUEST:
        discoveryContext = (SOPC_Internal_DiscoveryContext*) auxParam;
        SOPC_ASSERT(NULL != discoveryContext);
        if (NULL != discoveryContext->opcuaMessage)
        {
            SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) discoveryContext->opcuaMessage,
                                         &discoveryContext->opcuaMessage);
        }
        SOPC_Free(discoveryContext);
        break;
    case APP_TO_SE_CLOSE_CONNECTION:
        SOPC_Free((void*) auxParam); // Encapsulated application context
        break;
    case SE_TO_SE_SC_ALL_DISCONNECTED:
    case SE_TO_SE_ACTIVATE_ORPHANED_SESSION:
    case SE_TO_SE_CREATE_SESSION:
    case SE_TO_SE_ACTIVATE_SESSION:
    case SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO:
    case SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION:
    case TIMER_SE_EVAL_SESSION_TIMEOUT:
    case TIMER_SE_PUBLISH_CYCLE_TIMEOUT:
    case APP_TO_SE_SERVER_CONFIGURED:
    case APP_TO_SE_OPEN_ENDPOINT:
    case APP_TO_SE_CLOSE_ENDPOINT:
    case APP_TO_SE_OPEN_REVERSE_ENDPOINT:
    case APP_TO_SE_CLOSE_REVERSE_ENDPOINT:
    case APP_TO_SE_CLOSE_SESSION:
    case APP_TO_SE_CLOSE_ALL_CONNECTIONS:
    case APP_TO_SE_REEVALUATE_SCS:
    case APP_TO_SE_EVAL_USR_CRT_SESSIONS:
    case APP_TO_SE_CLOSE_SERVER_SESSIONS:
    default:
        // No parameter to free (SE_TO_SE_ACTIVATE_SESSION user token belongs to the session)
        break;
    }
}

// Frees the parameters of a secure channels event never treated (see ::SOPC_EventHandler_FreeEventFct)
static void SOPC_Services_FreeSecureChannelsEvent(int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(id);
    SOPC_UNUSED_ARG(auxParam);
    switch ((SOPC_SecureChannels_OutputEvent) event)
    {
    case SC_SERVICE_RCV_MSG:
    case SC_SERVICE_RCV_REQUEST:
        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    case EP_CONNECTED:
    case EP_CLOSED:
    case EP_REVERSE_CLOSED:
    case SC_CONNECTED:
    case SC_REVERSE_CONNECTED:
    case SC_CONNECTION_TIMEOUT:
    case SC_DISCONNECTED:
    case SC_SND_FAILURE:
    case SC_REQUEST_TIMEOUT:
    default:
        // No parameter to free
        break;
    }
}

static bool filterServerEvents(SOPC_Services_Event event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bParam = false;
    bool treatEvent = false;
    switch (event)
    {
    case APP_TO_SE_SERVER_CONFIGURED:
        bParam = (bool) params;
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_SERVER_CONFIGURED active=%" PRIuPTR, params);
        if (isServerConfigurationLocked == bParam)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: APP_TO_SE_SERVER_CONFIGURED IGNORED: received with same lock state=%s",
                bParam ? "locked" : "unlocked");
        }
        else
        {
            if (bParam)
            {
                // Register the Services layer as using the configuration before forwarding the event to SC
                status = SOPC_ToolkitServer_UsingLockedConfig(true);
                if (SOPC_STATUS_OK == status)
                {
                    isServerConfigurationLocked = true;
                    status = SOPC_SecureChannels_EnqueueEvent(SE_TO_SCS_SERVER_CONFIGURED, 0, (uintptr_t) true, 0);
                    SOPC_ASSERT(SOPC_STATUS_OK == status);
                }
            }
            else
            {
                isServerConfigurationLocked = false;
                status = SOPC_SecureChannels_EnqueueEvent(SE_TO_SCS_SERVER_CONFIGURED, 0, (uintptr_t) false, 0);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
                // Clear server context in B model (not configured anymore)
                io_dispatch_mgr__clear_server_configuration_context();
                /* Release the Services layer only after the B-model reset is complete. Otherwise
                 * SOPC_ToolkitServer_UnConfigure() could return as soon as SecureChannels releases its
                 * reference, allowing a new AddressSpace to be configured during the reset. */
                status = SOPC_ToolkitServer_UsingLockedConfig(false);
            }
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ServicesMgr: APP_TO_SE_SERVER_CONFIGURED FAILED: error in setting "
                                       "server config as used: %s (status: %d)",
                                       bParam ? "locked" : "unlocked", status);
            }
        }
        break;
    case SE_TO_SE_SERVER_DATA_CHANGED:
    case SE_TO_SE_SERVER_NODE_CHANGED:
    case SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO:
        if (!isServerConfigurationLocked)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "filterServerEvents: %s IGNORED: server configuration is not locked (configured)",
                                   SE_TO_SE_SERVER_DATA_CHANGED == event ? "SE_TO_SE_SERVER_DATA_CHANGED"
                                   : SE_TO_SE_SERVER_NODE_CHANGED == event
                                       ? "SE_TO_SE_SERVER_NODE_CHANGED"
                                       : "SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO");
        }
        else
        {
            treatEvent = true;
        }
        break;
    /* Note: since sessions are now closed by SOPC_HelperInternal_ActualShutdownServer on Stop
             and clear_server_configuration_context on clear, all
             sessions / subscriptions  should be terminated when configuration has been cleared.
             But avoid inhibiting timeout and closing operations of sessions/subscriptions
             as those do not use the configuration at all.
    case SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO:
    case SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION:
    case TIMER_SE_EVAL_SESSION_TIMEOUT:
    case TIMER_SE_PUBLISH_CYCLE_TIMEOUT:
    */
    case APP_TO_SE_OPEN_ENDPOINT:
    case APP_TO_SE_CLOSE_ENDPOINT:
    case APP_TO_SE_LOCAL_SERVICE_REQUEST:
    case APP_TO_SE_TRIGGER_EVENT:
    case APP_TO_SE_EVAL_USR_CRT_SESSIONS:
        if (!isServerConfigurationLocked)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "filterServerEvents: %s IGNORED: server configuration is not locked (configured)",
                                   APP_TO_SE_OPEN_ENDPOINT == event           ? "APP_TO_SE_OPEN_ENDPOINT"
                                   : APP_TO_SE_CLOSE_ENDPOINT == event        ? "APP_TO_SE_CLOSE_ENDPOINT"
                                   : APP_TO_SE_LOCAL_SERVICE_REQUEST == event ? "APP_TO_SE_LOCAL_SERVICE_REQUEST"
                                   : APP_TO_SE_TRIGGER_EVENT == event         ? "APP_TO_SE_TRIGGER_EVENT"
                                                                              : "APP_TO_SE_EVAL_USR_CRT_SESSIONS");
        }
        else
        {
            treatEvent = true;
        }
        break;
    default:
        treatEvent = true;
        break;
    }
    if (!treatEvent)
    {
        // Note: for APP_TO_SE_LOCAL_SERVICE_REQUEST, the response context is not returned to application,
        // this is managed in server wrapper by using the waiting request context list
        SOPC_Services_FreeServicesEvent((int32_t) event, id, params, auxParam);
    }
    return treatEvent;
}

static void onServiceEvent(SOPC_EventHandler* handler,
                           int32_t scEvent,
                           uint32_t id,
                           uintptr_t params,
                           uintptr_t auxParam)
{
    SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* epConfig = NULL;
    constants_statuscodes_bs__t_StatusCode_i sCode = constants_statuscodes_bs__e_sc_ok;
    SOPC_EncodeableType* encType = NULL;
    bool lbres = false;
    bool bres = false;
    void* msg = NULL;
    size_t length = 0;
    SOPC_Array* dataOrNodeChangedArray = NULL;
    SOPC_WriteDataChanged* writeDataChanged = NULL;
    SOPC_NodeChanged* nodeChanged = NULL;
    OpcUa_WriteValue* old_value = NULL;
    OpcUa_WriteValue* new_value = NULL;
    SOPC_Internal_AsyncSendMsgData* msg_data;
    const SOPC_NodeId* nodeId2 = NULL;
    char* nodeIdStr = NULL;
    char* nodeIdStr2 = NULL;
    const char* reverseEndpointURL = NULL;
    SOPC_Internal_SessionAppContext* sessionContext = NULL;
    SOPC_ExtensionObject* userToken = NULL;
    SOPC_Internal_DiscoveryContext* discoveryContext = NULL;
    SOPC_Internal_EventContext* eventContext = NULL;
    SOPC_DateTime currentTime = 0;

    bool treatEvent = filterServerEvents(event, id, params, auxParam);
    if (!treatEvent)
    {
        return;
    }

    switch (event)
    {
    case SE_TO_SE_SC_ALL_DISCONNECTED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SC_ALL_DISCONNECTED clientOnly=%" PRIuPTR, params);
        // Call directly toolkit configuration callback
        SOPC_Internal_AllClientSecureChannelsDisconnected((bool) params);
        break;

    case SE_TO_SE_ACTIVATE_ORPHANED_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_ACTIVATE_ORPHANED_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR,
                               id, auxParam);

        SOPC_ASSERT(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__internal_client_activate_orphaned_session(id, (constants__t_channel_config_idx_i) auxParam);
        break;
    case SE_TO_SE_CREATE_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_CREATE_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR, id,
                               auxParam);
        SOPC_ASSERT(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__internal_client_create_session((constants__t_session_i) id,
                                                        (constants__t_channel_config_idx_i) auxParam);
        break;
    case SE_TO_SE_ACTIVATE_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32,
                               id);

        if (NULL != (void*) params)
        {
            io_dispatch_mgr__client_reactivate_session_new_user(id, (constants__t_user_token_i) params);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32 " user parameter is NULL",
                                   id);
            sCode = constants_statuscodes_bs__e_sc_bad_generic_mask;
        }
        break;
    case SE_TO_SE_SERVER_DATA_CHANGED:
        /* Server side only:
           params = (SOPC_Array*) array of SOPC_WriteDataChanged
         */
        SOPC_ASSERT((void*) params != NULL);

        dataOrNodeChangedArray = (SOPC_Array*) params;
        length = SOPC_Array_Size(dataOrNodeChangedArray);
        for (size_t i = 0; i < length; i++)
        {
            writeDataChanged = (SOPC_WriteDataChanged*) SOPC_Array_Get_Ptr(dataOrNodeChangedArray, i);
            SOPC_ASSERT(writeDataChanged != NULL);
            old_value = writeDataChanged->oldValue;
            new_value = writeDataChanged->newValue;
            /* Note: write values deallocation managed by B model */
            io_dispatch_mgr__internal_server_data_changed(old_value, new_value);
        }
        SOPC_Array_Delete(dataOrNodeChangedArray);
        break;
    case SE_TO_SE_SERVER_NODE_CHANGED:
        /* Server side only:
           params = (SOPC_Array*) array of SOPC_NodeChanged
         */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SERVER_NODE_CHANGED batched notifications");

        SOPC_ASSERT(NULL != (void*) params);
        dataOrNodeChangedArray = (SOPC_Array*) params;
        length = SOPC_Array_Size(dataOrNodeChangedArray);
        lbres = SOPC_LOG_LEVEL_DEBUG == SOPC_Logger_GetTraceLogLevel();
        for (size_t i = 0; i < length; i++)
        {
            nodeChanged = (SOPC_NodeChanged*) SOPC_Array_Get_Ptr(dataOrNodeChangedArray, i);
            SOPC_ASSERT(NULL != nodeChanged);
            if (lbres)
            {
                nodeIdStr = SOPC_NodeId_ToCString(nodeChanged->nodeId);
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ServicesMgr: SE_TO_SE_SERVER_NODE_CHANGED %s nodeId: %s",
                                       nodeChanged->added ? "added" : "deleted", nodeIdStr);
                SOPC_Free(nodeIdStr);
            }
            io_dispatch_mgr__internal_server_node_changed(nodeChanged->added, nodeChanged->nodeId);
            SOPC_NodeId_Clear(nodeChanged->nodeId);
            SOPC_Free(nodeChanged->nodeId);
        }
        SOPC_Array_Delete(dataOrNodeChangedArray);
        break;
    case SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO:
        /* Server side only:
           id = session id
           auxParam = (int32_t) session state
         */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO session=%" PRIu32
                               " sessionState=%" PRIuPTR,
                               id, auxParam);

        io_dispatch_mgr__internal_server_inactive_session_prio_event((constants__t_session_i) id,
                                                                     (constants__t_sessionState_i) auxParam, &bres);

        if (bres == false)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO session=%" PRIu32 " treatment failed", id);
        }
        break;
    case SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO:
        /* Server side only:
           id = session id
           params = (SOPC_Internal_AsyncSendMsgData*)
         */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32, id);

        msg_data = (void*) params;
        SOPC_ASSERT(msg_data != NULL);

        io_dispatch_mgr__internal_server_send_publish_response_prio_event(
            (constants__t_session_i) id, msg_data->requestHandle, msg_data->requestId, msg_data->msgToSend,
            (constants_statuscodes_bs__t_StatusCode_i) msg_data->bStatusCode, &bres);
        SOPC_Free(msg_data);

        if (!bres)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32 " treatment failed", id);
        }
        break;
    case SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION:
        /* Server side only:
           id = subscription id
         */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION subscription=%" PRIu32, id);

        io_dispatch_mgr__internal_server_close_subscription((constants__t_subscription_i) id, &bres);

        if (!bres)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION subscription=%" PRIu32 " treatment failed", id);
        }
        break;
    case TIMER_SE_EVAL_SESSION_TIMEOUT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: TIMER_SE_EVAL_SESSION_TIMEOUT session=%" PRIu32, id);
        io_dispatch_mgr__internal_server_evaluate_session_timeout((constants__t_session_i) id);
        break;
    case TIMER_SE_PUBLISH_CYCLE_TIMEOUT:
        /* Server side only: id = subscription id */
        io_dispatch_mgr__internal_server_subscription_publish_timeout((constants__t_subscription_i) id, &bres);
        if (bres == false)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: TIMER_SE_PUBLISH_CYCLE_TIMEOUT subscription=%" PRIu32 " treatment failed", id);
        }
        break;

    /* App to Services events */
    case APP_TO_SE_SERVER_CONFIGURED:
        /* Already managed in filterServerEvents */
        break;
    case APP_TO_SE_OPEN_ENDPOINT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: APP_TO_SE_OPEN_ENDPOINT epCfgIdx=%" PRIu32,
                               id);

        // id ==  endpoint configuration index
        // => B model entry point to add
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
        if (NULL == epConfig)
        {
            status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, (uintptr_t) NULL, SOPC_STATUS_INVALID_PARAMETERS);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
        else
        {
            status = SOPC_SecureChannels_EnqueueEvent(EP_OPEN,
                                                      id, // Server endpoint config idx
                                                      (uintptr_t) NULL, 0);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
        break;
    case APP_TO_SE_CLOSE_ENDPOINT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: APP_TO_SE_CLOSE_ENDPOINT epCfgIdx=%" PRIu32,
                               id);

        // id ==  endpoint configuration index
        // => B model entry point to add
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
        if (NULL == epConfig)
        {
            status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, (uintptr_t) NULL, SOPC_STATUS_INVALID_PARAMETERS);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
        else
        {
            status = SOPC_SecureChannels_EnqueueEvent(EP_CLOSE, id, (uintptr_t) NULL, 0);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
        break;

    case APP_TO_SE_LOCAL_SERVICE_REQUEST:
        if ((void*) params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_LOCAL_SERVICE_REQUEST epCfgIdx=%" PRIu32
                               " msgType=%s ctx=%" PRIuPTR,
                               id, SOPC_EncodeableType_GetName(encType), auxParam);

        // id =  endpoint configuration index
        // params = local service request
        // auxParam = user application session context
        SOPC_ASSERT(id <= INT32_MAX);

        if (SOPC_Services_IsQueueBusyLocal())
        {
            SOPC_Services_RefuseLocalServiceRequestBusy(id, params, auxParam);
            servicesBusyLocal.nbRefused++;
        }
        else
        {
            io_dispatch_mgr__server_treat_local_service_request(id, (constants__t_msg_i) params, auxParam, &sCode);
            if (constants_statuscodes_bs__e_sc_ok != sCode)
            {
                // Error case
                status = SOPC_EncodeableObject_Create(&OpcUa_ServiceFault_EncodeableType, &msg);
                if (SOPC_STATUS_OK == status && NULL != msg)
                {
                    util_status_code__B_to_C(sCode, &((OpcUa_ServiceFault*) msg)->ResponseHeader.ServiceResult);
                }
                else
                {
                    msg = NULL;
                }
                status = SOPC_App_EnqueueComEvent(SE_LOCAL_SERVICE_RESPONSE, id, (uintptr_t) msg, auxParam);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "ServicesMgr: APP_TO_SE_LOCAL_SERVICE_REQUEST failed epCfgIdx=%" PRIu32
                                         " msgType=%s ctx=%" PRIuPTR,
                                         id, SOPC_EncodeableType_GetName(encType), auxParam);
            }
        }
        break;
    case APP_TO_SE_TRIGGER_EVENT:
        // params =  (SOPC_Internal_EventContext*)
        eventContext = (SOPC_Internal_EventContext*) params;
        SOPC_ASSERT(NULL != eventContext);
        nodeIdStr = SOPC_NodeId_ToCString(&eventContext->notifierNodeId);
        nodeId2 = SOPC_Event_GetEventTypeId(eventContext->event);
        nodeIdStr2 = (NULL == nodeId2 ? SOPC_strdup("") : SOPC_NodeId_ToCString(nodeId2));
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_TRIGGER_EVENT eventTypeId=%s, notifierId=%s", nodeIdStr2,
                               nodeIdStr);

        // Update the receiveTime variable (and time variable if not set)
        currentTime = SOPC_Time_GetCurrentTimeUTC();
        status = SOPC_Event_SetReceiveTime(eventContext->event, currentTime);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        if (0 == SOPC_Event_GetTime(eventContext->event))
        {
            status = SOPC_Event_SetTime(eventContext->event, currentTime);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }

        io_dispatch_mgr__internal_server_event_triggered(&eventContext->notifierNodeId, eventContext->event,
                                                         eventContext->optSessionId, eventContext->optSubscriptionId,
                                                         eventContext->optMonitoredItemId, &bres);
        if (!bres)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ServicesMgr: failure in treatment of APP_TO_SE_TRIGGER_EVENT eventTypeId=%s, "
                                   "notifierId=%s",
                                   nodeIdStr2, nodeIdStr);
        }
        SOPC_Free(nodeIdStr);
        SOPC_Free(nodeIdStr2);

        SOPC_NodeId_Clear(&eventContext->notifierNodeId);
        SOPC_Event_Delete(&eventContext->event);
        SOPC_Free(eventContext);
        break;
    case APP_TO_SE_OPEN_REVERSE_ENDPOINT:
        /* id = reverse endpoint description config index */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_OPEN_REVERSE_ENDPOINT reverseEpCfgIdx=%" PRIu32, id);
        // Check config index is valid
        reverseEndpointURL = SOPC_ToolkitClient_GetReverseEndpointURL(id);
        SOPC_ASSERT(NULL != reverseEndpointURL && "Invalid reverse endpoint configuration index provided");
        status = SOPC_SecureChannels_EnqueueEvent(REVERSE_EP_OPEN,
                                                  id, // Reverse endpoint config idx
                                                  (uintptr_t) NULL, 0);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        break;
    case APP_TO_SE_CLOSE_REVERSE_ENDPOINT:
        /* id = reverse endpoint description config index */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_CLOSE_REVERSE_ENDPOINT reverseEpCfgIdx=%" PRIu32, id);
        // Check config index is valid
        reverseEndpointURL = SOPC_ToolkitClient_GetReverseEndpointURL(id);
        SOPC_ASSERT(NULL != reverseEndpointURL && "Invalid reverse endpoint configuration index provided");
        status = SOPC_SecureChannels_EnqueueEvent(REVERSE_EP_CLOSE, id, (uintptr_t) NULL, 0);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        break;
    case APP_TO_SE_ACTIVATE_SESSION:
        // id = secure channel config index,
        // params = reverse endpoint connection index or 0 if not a reverse connection
        // auxParam = (SOPC_Internal_SessionAppContext*)
        SOPC_ASSERT(id <= constants__t_channel_config_idx_i_max);
        SOPC_ASSERT((void*) auxParam != NULL);
        sessionContext = (SOPC_Internal_SessionAppContext*) auxParam;
        userToken = sessionContext->userToken;
        sessionContext->userToken = NULL; // Provided as separated parameter

        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_ACTIVATE_SESSION scCfgIdx=%" PRIu32 " reverseEpCfgIdx=%" PRIuPTR
                               " ctx=%" PRIuPTR,
                               id, params, sessionContext->userSessionContext);

        io_dispatch_mgr__client_activate_new_session(id, (uint32_t) params, userToken, sessionContext, &bres);

        if (!bres)
        {
            SOPC_App_EnqueueComEvent(SE_SESSION_ACTIVATION_FAILURE,
                                     0,                                   // session id (not yet defined)
                                     (uintptr_t) NULL,                    // user ?
                                     sessionContext->userSessionContext); // user application session context
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "ServicesMgr: APP_TO_SE_ACTIVATE_SESSION failed scCfgIdx=%" PRIu32
                                     " reverseEpCfgIdx=%" PRIuPTR " ctx=%" PRIuPTR,
                                     id, params, auxParam);
            SOPC_ExtensionObject_Clear(userToken);
            SOPC_Free(userToken);
            SOPC_Free(sessionContext);
        }
        break;
    case APP_TO_SE_SEND_SESSION_REQUEST:
        if ((void*) params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST  session=%" PRIu32
                               " msgType=%s ctx=%" PRIuPTR,
                               id, SOPC_EncodeableType_GetName(encType), auxParam);

        // id == session id
        // params = request
        SOPC_ASSERT(id <= constants__t_session_i_max);

        io_dispatch_mgr__client_send_service_request(id, (constants__t_msg_i) params, auxParam, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            status = SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, util_status_code__B_to_return_status_C(sCode),
                                              (uintptr_t) encType, auxParam);
            SOPC_ASSERT(SOPC_STATUS_OK == status);

            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST failed session=%" PRIu32
                                     " msgType=%s ctx=%" PRIuPTR,
                                     id, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        break;
    case APP_TO_SE_CLOSE_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: APP_TO_SE_CLOSE_SESSION  session=%" PRIu32,
                               id);

        // id == session id
        SOPC_ASSERT(id <= constants__t_session_i_max);

        io_dispatch_mgr__client_send_close_session_request(id, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ServicesMgr: APP_TO_SE_CLOSE_SESSION failed session=%" PRIu32, id);
        }
        break;
    case APP_TO_SE_CLOSE_CONNECTION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_CLOSE_CONNECTION  scCfgIdx=%" PRIu32, id);
        // id = secure channel config index
        io_dispatch_mgr__client_close_channel(id, auxParam, &bres);
        if (!bres)
        {
            SOPC_App_EnqueueComEvent(SE_CLOSED_CHANNEL, id, (uintptr_t) false, auxParam);
        }
        break;
    case APP_TO_SE_SEND_DISCOVERY_REQUEST:
        // id = secure channel config index,
        // params = reverse endpoint connection index or 0 if not a reverse connection
        // auxParam = (SOPC_Internal_DiscoveryContext*)
        SOPC_ASSERT(id <= constants_bs__t_channel_config_idx_i_max);
        SOPC_ASSERT((void*) auxParam != NULL);

        discoveryContext = (SOPC_Internal_DiscoveryContext*) auxParam;

        if (discoveryContext->opcuaMessage != NULL)
        {
            encType = *(SOPC_EncodeableType**) discoveryContext->opcuaMessage;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_SEND_DISCOVERY_REQUEST scCfgIdx=%" PRIu32
                               " reverseEpCfgIdx=%" PRIuPTR " msgType=%s ctx=%" PRIuPTR,
                               id, params, SOPC_EncodeableType_GetName(encType), discoveryContext->discoveryAppContext);

        io_dispatch_mgr__client_send_discovery_request(id, (uint32_t) params,
                                                       (constants__t_msg_i) discoveryContext->opcuaMessage,
                                                       discoveryContext->discoveryAppContext, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            status = SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, util_status_code__B_to_return_status_C(sCode),
                                              (uintptr_t) encType, discoveryContext->discoveryAppContext);
            SOPC_ASSERT(SOPC_STATUS_OK == status);

            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "ServicesMgr: APP_TO_SE_SEND_DISCOVERY_REQUEST failed scCfgIdx=%" PRIu32
                                     " reverseEpCfgIdx=%" PRIuPTR " msgType=%s ctx=%" PRIuPTR,
                                     id, params, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        SOPC_Free(discoveryContext);
        break;
    case APP_TO_SE_CLOSE_ALL_CONNECTIONS:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_CLOSE_ALL_CONNECTIONS clientOnly=%" PRIuPTR, params);

        io_dispatch_mgr__close_all_active_connections((bool) params, &bres);
        if (!bres)
        {
            // All connections already closed: simulate new service event
            onServiceEvent(handler, SE_TO_SE_SC_ALL_DISCONNECTED, id, params, auxParam);
        }
        break;
    case APP_TO_SE_REEVALUATE_SCS:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_REEVALUATE_SCS isServer=%" PRIuPTR " isOwnCert=%" PRIuPTR,
                               params, auxParam);
        status = SOPC_SecureChannels_EnqueueEvent(SCS_REEVALUATE_SCS, id, params, auxParam);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        break;
    case APP_TO_SE_EVAL_USR_CRT_SESSIONS:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: APP_TO_SE_EVAL_USR_CRT_SESSION");
        io_dispatch_mgr__internal_server_evaluate_all_session_user_cert();
        break;
    case APP_TO_SE_CLOSE_SERVER_SESSIONS:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: APP_TO_SE_CLOSE_SERVER_SESSIONS exceptSessionId=%" PRIu32 " isShutdown=%s",
                               id, ((bool) params ? "TRUE" : "FALSE"));
        io_dispatch_mgr__server_close_sessions((SOPC_SessionId) id);
        if ((bool) params)
        {
            io_dispatch_mgr__server_stopped();
            // Sessions closures are already counted: hooked server cumulated values can be reset
            call_server_stopped_hook();
        }
        break;
    default:
        SOPC_ASSERT(false);
    }

    // Note: not called for filtered events (not treated)
    call_event_treated_hook();
}

void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_ASSERT(servicesEventHandler != NULL);
    SOPC_EventHandler_Post(servicesEventHandler, (int32_t) seEvent, id, params, auxParam);
}

void SOPC_Services_EnqueueEventAsNext(SOPC_Services_Event seEvent, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_ASSERT(servicesEventHandler != NULL);
    SOPC_EventHandler_PostAsNext(servicesEventHandler, (int32_t) seEvent, id, params, auxParam);
}

void SOPC_Services_SetHooks(SOPC_Services_Hooks* hooks)
{
    SOPC_Atomic_Ptr_Set(&servicesHooks, (void*) hooks);
}

uint32_t SOPC_Services_Get_QueueSize(void)
{
    return SOPC_EventHandler_Get_QueueSize(servicesEventHandler);
}

uint32_t SOPC_Services_Get_PendingLocalRequests(void)
{
    return SOPC_EventHandler_Get_PendingCountByCategory(servicesEventHandler, SOPC_SERVICES_PENDING_LOCAL_REQUESTS);
}

uint32_t SOPC_Services_Get_PendingExternalRequests(void)
{
    return SOPC_EventHandler_Get_PendingCountByCategory(servicesEventHandler, SOPC_SERVICES_PENDING_EXTERNAL_REQUESTS);
}

void SOPC_Services_Initialize(SOPC_SetListenerFunc* setSecureChannelsListener)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    int priority = 0;
    int cpuAffinity = -1;
    servicesBusyLocal = (SOPC_Services_BusyState){false, 0, 0};
    servicesBusyExternal = (SOPC_Services_BusyState){false, 0, 0};

    SOPC_ToolkitInternal_GetThreadProperties(SOPC_TOOLKIT_THREAD_SERVICES, &priority, &cpuAffinity);
    servicesLooper = SOPC_Looper_CreatePrioritized("Services", priority, cpuAffinity);
    SOPC_ASSERT(servicesLooper != NULL);

    // Pending local and external requests are counted for the busy watermarks
    servicesEventHandler =
        SOPC_EventHandler_CreateWithCategory(servicesLooper, onServiceEvent, SOPC_Services_ServicesEventCategory);
    SOPC_ASSERT(servicesEventHandler != NULL);

    secureChannelsEventHandler = SOPC_EventHandler_CreateWithCategory(servicesLooper, onSecureChannelEvent,
                                                                      SOPC_Services_SecureChannelsEventCategory);
    SOPC_ASSERT(secureChannelsEventHandler != NULL);

    SOPC_EventHandler_SetFreeEventFct(servicesEventHandler, SOPC_Services_FreeServicesEvent);
    SOPC_EventHandler_SetFreeEventFct(secureChannelsEventHandler, SOPC_Services_FreeSecureChannelsEvent);

    // Init async close management flag
    status = SOPC_Mutex_Initialization(&closeAllConnectionsSync.mutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Condition_Init(&closeAllConnectionsSync.cond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    setSecureChannelsListener(secureChannelsEventHandler);

    /* Init B model */
    INITIALISATION();
}

void SOPC_Services_CloseAllSCs(bool clientOnly)
{
    SOPC_Mutex_Lock(&closeAllConnectionsSync.mutex);
    closeAllConnectionsSync.requestedFlag = true;
    closeAllConnectionsSync.clientOnlyFlag = clientOnly;
    // Do a synchronous connections closed (effective on client only)
    SOPC_EventHandler_Post(servicesEventHandler, APP_TO_SE_CLOSE_ALL_CONNECTIONS, 0, (uintptr_t) clientOnly, 0);
    while (!closeAllConnectionsSync.allDisconnectedFlag)
    {
        SOPC_Mutex_UnlockAndWaitCond(&closeAllConnectionsSync.cond, &closeAllConnectionsSync.mutex);
    }
    closeAllConnectionsSync.allDisconnectedFlag = false;
    closeAllConnectionsSync.clientOnlyFlag = false;
    closeAllConnectionsSync.requestedFlag = false;
    SOPC_Mutex_Unlock(&closeAllConnectionsSync.mutex);
}

void SOPC_Services_PreClear(void)
{
    if (servicesPreCleared)
    {
        return;
    }
    SOPC_Looper_Stop(servicesLooper);

    /* B model uninitialized by the calling thread. Hypotheses:
     * - sockets, secure channels and services threads stopped: no concurrent B model execution,
     * - timers evaluation stopped but timers not cleared: session and publish timers are cancelled,
     * - application thread running: client session closures are notified to the application,
     * - services handlers not deallocated: server session closures post an untreated event. */
    io_dispatch_mgr__UNINITIALISATION();
    servicesPreCleared = true;
}

void SOPC_Services_Clear(void)
{
    SOPC_Services_PreClear();
    SOPC_Looper_Delete(servicesLooper);
    servicesLooper = NULL;
    // Handlers deallocated by SOPC_Looper_Delete call
    servicesEventHandler = NULL;
    secureChannelsEventHandler = NULL;
    servicesPreCleared = false;
    SOPC_Atomic_Ptr_Set(&servicesHooks, NULL);

    closeAllConnectionsSync.allDisconnectedFlag = false;
    closeAllConnectionsSync.clientOnlyFlag = false;
    closeAllConnectionsSync.requestedFlag = false;
    SOPC_Mutex_Clear(&closeAllConnectionsSync.mutex);
    SOPC_Condition_Clear(&closeAllConnectionsSync.cond);
}
