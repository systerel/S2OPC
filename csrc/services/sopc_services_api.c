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
#include <stdio.h>

#include "sopc_enums.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_services_api_internal.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include "io_dispatch_mgr.h"
#include "monitored_item_pointer_bs.h"
#include "service_mgr_bs.h"
#include "toolkit_header_init.h"
#include "util_b2c.h"

static SOPC_Looper* servicesLooper = NULL;
static SOPC_EventHandler* secureChannelsEventHandler = NULL;
static SOPC_EventHandler* servicesEventHandler = NULL;

// Structure used to close all connections in a synchronous way
// (necessary on toolkit clear)
static struct
{
    Mutex mutex;
    Condition cond;
    bool allDisconnectedFlag;
    bool requestedFlag;
} closeAllConnectionsSync = {.requestedFlag = false, .allDisconnectedFlag = false};

SOPC_EventHandler* SOPC_Services_GetEventHandler()
{
    return servicesEventHandler;
}

static void SOPC_Internal_AllClientSecureChannelsDisconnected(void)
{
    Mutex_Lock(&closeAllConnectionsSync.mutex);
    if (closeAllConnectionsSync.requestedFlag != false)
    {
        closeAllConnectionsSync.allDisconnectedFlag = true;
        Condition_SignalAll(&closeAllConnectionsSync.cond);
    }
    Mutex_Unlock(&closeAllConnectionsSync.mutex);
}

static void onSecureChannelEvent(SOPC_EventHandler* handler,
                                 int32_t event,
                                 uint32_t id,
                                 void* params,
                                 uintptr_t auxParam)
{
    (void) handler;
    SOPC_SecureChannels_OutputEvent scEvent = (SOPC_SecureChannels_OutputEvent) event;

    switch (scEvent)
    {
    case EP_CONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_EP_SC_CONNECTED epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32
                               " scIdx=%" PRIuPTR,
                               id, params == NULL ? 0 : *(uint32_t*) params, auxParam);

        // id ==  endpoint configuration index
        // params = channel configuration index POINTER
        // auxParam == connection Id
        assert(id <= INT32_MAX);
        assert(params != NULL);
        uint32_t channel_config_idx = *((uint32_t*) params);
        assert(channel_config_idx <= constants__t_channel_config_idx_i_max);
        assert(auxParam <= constants__t_channel_i_max);

        bool bres = false;
        io_dispatch_mgr__server_channel_connected_event(id, channel_config_idx, (uint32_t) auxParam, &bres);
        if (bres == false)
        {
            SOPC_Logger_TraceError("Services: channel state incoherent epCfgIdx=%" PRIu32 " scIdx=%" PRIuPTR, id,
                                   auxParam);

            SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT, (uint32_t) auxParam, NULL, 0);
        }

        break;
    case EP_CLOSED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_EP_CLOSED epCfgIdx=%" PRIu32 " returnStatus=%" PRIdPTR, id, auxParam);
        // id == endpoint configuration index
        // params = NULL
        // auxParam == status
        // => B model entry point to add
        SOPC_ReturnStatus status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, NULL, auxParam);
        assert(status == SOPC_STATUS_OK);
        break;
    case SC_CONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_SC_CONNECTED scIdx=%" PRIu32 " scCfgIdx=%" PRIuPTR, id, auxParam);
        // id == connection Id
        // auxParam == secure channel configuration index
        // => B model entry point to add
        assert(id <= constants__t_channel_i_max);
        assert(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__client_channel_connected_event((uint32_t) auxParam, id);
        break;
    case SC_CONNECTION_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_SC_CONNECTION_TIMEOUT scCfgIdx=%" PRIu32, id);

        // id == secure channel configuration index
        // => B model entry point to add
        assert(id <= constants_bs__t_channel_config_idx_i_max);
        io_dispatch_mgr__client_secure_channel_timeout(id);
        break;
    case SC_DISCONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_SC_DISCONNECTED scIdx=%" PRIu32, id);
        // id == connection Id ==> TMP: secure channel config idx
        // auxParam = status
        // => B model entry point to add
        // secure_channel_lost call !
        io_dispatch_mgr__secure_channel_lost(id);
        break;
    case SC_SERVICE_RCV_MSG:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_SC_SERVICE_RCV_MSG scIdx=%" PRIu32 " reqId=%" PRIuPTR, id, auxParam);

        // id ==  connection Id
        // params = message content (byte buffer)
        // auxParam == context (request id)
        assert(NULL != params);
        io_dispatch_mgr__receive_msg_buffer(id, (constants__t_byte_buffer_i) params,
                                            (constants__t_request_context_i) auxParam);
        // params is freed by services manager
        break;
    case SC_SND_FAILURE:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_SND_FAILURE scIdx=%" PRIu32 " reqId=%" PRIu32 " statusCode=%" PRIXPTR,
                               id, params == NULL ? 0 : *(uint32_t*) params, auxParam);

        if (NULL != params)
        {
            constants_statuscodes_bs__t_StatusCode_i statusCode;
            util_status_code__C_to_B((SOPC_StatusCode) auxParam, &statusCode);
            io_dispatch_mgr__snd_msg_failure(id, (constants__t_request_context_i) * (uint32_t*) params, statusCode);
            SOPC_Free(params);
        } // else: without request Id, it cannot be treated
        break;
    case SC_REQUEST_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_REQUEST_TIMEOUT scIdx=%" PRIu32 " reqHandle=%" PRIuPTR, id, auxParam);

        /* id = secure channel connection index,
           auxParam = request handle */
        assert(id <= constants__t_channel_i_max);
        assert(auxParam <= SOPC_MAX_PENDING_REQUESTS);
        io_dispatch_mgr__client_request_timeout(id, (uint32_t) auxParam);
        break;
    }
}

static void onServiceEvent(SOPC_EventHandler* handler, int32_t scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* epConfig = NULL;
    constants_statuscodes_bs__t_StatusCode_i sCode = constants_statuscodes_bs__e_sc_ok;
    SOPC_EncodeableType* encType = NULL;
    bool bres = false;
    void* msg = NULL;
    switch (event)
    {
    case SE_TO_SE_SC_ALL_DISCONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_SC_ALL_DISCONNECTED");
        // Call directly toolkit configuration callback
        SOPC_Internal_AllClientSecureChannelsDisconnected();
        break;

    case SE_TO_SE_ACTIVATE_ORPHANED_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_ACTIVATE_ORPHANED_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR,
                               id, auxParam);

        assert(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__internal_client_activate_orphaned_session(id, (constants__t_channel_config_idx_i) auxParam);
        break;
    case SE_TO_SE_CREATE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_CREATE_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR, id,
                               auxParam);
        assert(auxParam <= constants__t_channel_config_idx_i_max);
        io_dispatch_mgr__internal_client_create_session((constants__t_session_i) id,
                                                        (constants__t_channel_config_idx_i) auxParam);
        break;
    case SE_TO_SE_ACTIVATE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32, id);

        if (NULL != params)
        {
            io_dispatch_mgr__client_reactivate_session_new_user(id, (constants__t_user_token_i) params);
        }
        else
        {
            SOPC_Logger_TraceError("ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32 " user parameter is NULL",
                                   id);
            sCode = constants_statuscodes_bs__e_sc_bad_generic;
        }
        break;
    case SE_TO_SE_SERVER_DATA_CHANGED:
        /* Server side only:
           id = session id
           auxParam = (int32_t) session state
         */
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_SERVER_DATA_CHANGED session=%" PRIu32, id);

        assert(params != NULL);

        OpcUa_WriteValue* old_value = params;
        OpcUa_WriteValue* new_value = (void*) auxParam;
        assert(old_value != NULL);
        assert(new_value != NULL);

        /* Note: write values deallocation managed by B model */
        io_dispatch_mgr__internal_server_data_changed(old_value, new_value, &bres);

        if (bres == false)
        {
            SOPC_Logger_TraceError("ServicesMgr: SE_TO_SE_SERVER_DATA_CHANGED session=%" PRIu32 " treatment failed",
                                   id);
        }
        break;
    case SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO:
        /* Server side only:
           id = session id
           auxParam = (int32_t) session state
         */
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO session=%" PRIu32
                               " sessionState=%" PRIdPTR,
                               id, auxParam);

        io_dispatch_mgr__internal_server_inactive_session_prio_event((constants__t_session_i) id,
                                                                     (constants__t_sessionState) auxParam, &bres);

        if (bres == false)
        {
            SOPC_Logger_TraceError(
                "ServicesMgr: SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO session=%" PRIu32 " treatment failed", id);
        }
        break;
    case SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO:
        /* Server side only:
           id = session id
           params = (SOPC_Internal_AsyncSendMsgData*)
           auxParams = (constants_statuscodes_bs__t_StatusCode_i) service result code
         */
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32, id);

        SOPC_Internal_AsyncSendMsgData* msg_data = params;
        assert(msg_data != NULL);

        io_dispatch_mgr__internal_server_send_publish_response_prio_event(
            (constants__t_session_i) id, msg_data->requestHandle, msg_data->requestId, msg_data->msgToSend,
            (constants_statuscodes_bs__t_StatusCode_i) auxParam, &bres);
        SOPC_Free(msg_data);

        if (bres == false)
        {
            SOPC_Logger_TraceError(
                "ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32 " treatment failed", id);
        }
        break;
    case TIMER_SE_EVAL_SESSION_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: TIMER_SE_EVAL_SESSION_TIMEOUT session=%" PRIu32, id);
        io_dispatch_mgr__internal_server_evaluate_session_timeout((constants__t_session_i) id);
        break;
    case TIMER_SE_PUBLISH_CYCLE_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: TIMER_SE_PUBLISH_CYCLE_TIMEOUT subscription=%" PRIu32, id);
        /* Server side only: id = subscription id */
        io_dispatch_mgr__internal_server_subscription_publish_timeout((constants__t_subscription_i) id, &bres);
        if (bres == false)
        {
            SOPC_Logger_TraceError(
                "ServicesMgr: TIMER_SE_PUBLISH_CYCLE_TIMEOUT subscription=%" PRIu32 " treatment failed", id);
        }
        break;

    /* App to Services events */
    case APP_TO_SE_OPEN_ENDPOINT:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_OPEN_ENDPOINT epCfgIdx=%" PRIu32, id);

        // id ==  endpoint configuration index
        // => B model entry point to add
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
        if (NULL == epConfig)
        {
            status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, NULL, SOPC_STATUS_INVALID_PARAMETERS);
        }
        else
        {
            SOPC_SecureChannels_EnqueueEvent(EP_OPEN,
                                             id, // Server endpoint config idx
                                             NULL, 0);
            status = SOPC_STATUS_OK;
        }
        break;
    case APP_TO_SE_CLOSE_ENDPOINT:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_CLOSE_ENDPOINT epCfgIdx=%" PRIu32, id);

        // id ==  endpoint configuration index
        // => B model entry point to add
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
        if (NULL == epConfig)
        {
            status = SOPC_App_EnqueueComEvent(SE_CLOSED_ENDPOINT, id, NULL, SOPC_STATUS_INVALID_PARAMETERS);
        }
        else
        {
            SOPC_SecureChannels_EnqueueEvent(EP_CLOSE, id, NULL, 0);
            status = SOPC_STATUS_OK;
        }
        break;

    case APP_TO_SE_LOCAL_SERVICE_REQUEST:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_LOCAL_SERVICE_REQUEST epCfgIdx=%" PRIu32
                               " msgType=%s ctx=%" PRIuPTR,
                               id, SOPC_EncodeableType_GetName(encType), auxParam);

        // id =  endpoint configuration index
        // params = local service request
        // auxParam = user application session context
        assert(id <= INT32_MAX);

        io_dispatch_mgr__server_treat_local_service_request(id, params, auxParam, &sCode);
        if (constants_statuscodes_bs__e_sc_ok != sCode)
        {
            // Error case
            status = SOPC_Encodeable_Create(&OpcUa_ServiceFault_EncodeableType, &msg);
            if (SOPC_STATUS_OK == status && NULL != msg)
            {
                util_status_code__B_to_C(sCode, &((OpcUa_ServiceFault*) msg)->ResponseHeader.ServiceResult);
            }
            else
            {
                msg = NULL;
            }
            SOPC_App_EnqueueComEvent(SE_LOCAL_SERVICE_RESPONSE, id, msg, auxParam);
            SOPC_Logger_TraceWarning("ServicesMgr: APP_TO_SE_LOCAL_SERVICE_REQUEST failed epCfgIdx=%" PRIu32
                                     " msgType=%s ctx=%" PRIuPTR,
                                     id, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        break;
    case APP_TO_SE_ACTIVATE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_ACTIVATE_SESSION scCfgIdx=%" PRIu32 " ctx=%" PRIuPTR, id,
                               auxParam);

        // id == secure channel configuration
        // params = user authentication
        // auxParam = user application session context
        assert(id <= constants__t_channel_config_idx_i_max);
        assert(params != NULL);

        io_dispatch_mgr__client_activate_new_session(id, (constants__t_user_token_i) params, auxParam, &bres);

        if (bres == false)
        {
            SOPC_App_EnqueueComEvent(SE_SESSION_ACTIVATION_FAILURE,
                                     0,         // session id (not yet defined)
                                     NULL,      // user ?
                                     auxParam); // user application session context
            SOPC_Logger_TraceWarning(
                "ServicesMgr: APP_TO_SE_ACTIVATE_SESSION failed scCfgIdx=%" PRIu32 " ctx=%" PRIuPTR, id, auxParam);
        }
        break;
    case APP_TO_SE_SEND_SESSION_REQUEST:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST  session=%" PRIu32
                               " msgType=%s ctx=%" PRIuPTR,
                               id, SOPC_EncodeableType_GetName(encType), auxParam);

        // id == session id
        // params = request
        assert(id <= constants__t_session_i_max);

        io_dispatch_mgr__client_send_service_request(id, params, auxParam, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, util_status_code__B_to_return_status_C(sCode), encType,
                                     auxParam);
            status = SOPC_STATUS_NOK;

            SOPC_Logger_TraceWarning("ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST failed session=%" PRIu32
                                     " msgType=%s ctx=%" PRIuPTR,
                                     id, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        break;
    case APP_TO_SE_CLOSE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_CLOSE_SESSION  session=%" PRIu32, id);

        // id == session id
        assert(id <= constants__t_session_i_max);

        io_dispatch_mgr__client_send_close_session_request(id, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            SOPC_Logger_TraceError("ServicesMgr: APP_TO_SE_CLOSE_SESSION failed session=%" PRIu32, id);
        }
        break;
    case APP_TO_SE_SEND_DISCOVERY_REQUEST:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_SEND_DISCOVERY_REQUEST scCfgIdx=%" PRIu32
                               " msgType=%s ctx=%" PRIuPTR,
                               id, SOPC_EncodeableType_GetName(encType), auxParam);

        // id == endpoint connection config idx
        // params = request
        assert(id <= constants_bs__t_channel_config_idx_i_max);

        io_dispatch_mgr__client_send_discovery_request(id, params, auxParam, &sCode);
        if (sCode != constants_statuscodes_bs__e_sc_ok)
        {
            SOPC_App_EnqueueComEvent(SE_SND_REQUEST_FAILED, util_status_code__B_to_return_status_C(sCode), encType,
                                     auxParam);
            status = SOPC_STATUS_NOK;

            SOPC_Logger_TraceWarning("ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST failed session=%" PRIu32
                                     " msgType=%s ctx=%" PRIuPTR,
                                     id, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        break;
    case APP_TO_SE_CLOSE_ALL_CONNECTIONS:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_CLOSE_ALL_CONNECTIONS");

        io_dispatch_mgr__close_all_active_connections(&bres);
        if (bres == false)
        {
            // All connections considered closed: simulate new service event
            onServiceEvent(handler, SE_TO_SE_SC_ALL_DISCONNECTED, id, params, auxParam);
        }
        break;
    default:
        assert(false);
    }
}

void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    assert(servicesEventHandler != NULL);
    SOPC_EventHandler_Post(servicesEventHandler, (int32_t) seEvent, id, params, auxParam);
}

void SOPC_Services_Initialize(SOPC_SetListenerFunc setSecureChannelsListener)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    servicesLooper = SOPC_Looper_Create("Services");
    assert(servicesLooper != NULL);

    servicesEventHandler = SOPC_EventHandler_Create(servicesLooper, onServiceEvent);
    assert(servicesEventHandler != NULL);

    secureChannelsEventHandler = SOPC_EventHandler_Create(servicesLooper, onSecureChannelEvent);
    assert(secureChannelsEventHandler != NULL);

    // Init async close management flag
    status = Mutex_Initialization(&closeAllConnectionsSync.mutex);
    assert(status == SOPC_STATUS_OK);

    status = Condition_Init(&closeAllConnectionsSync.cond);
    assert(status == SOPC_STATUS_OK);

    setSecureChannelsListener(secureChannelsEventHandler);
}

void SOPC_Services_ToolkitConfigured()
{
    /* Init B model */
    INITIALISATION();
}

void SOPC_Services_PreClear()
{
    Mutex_Lock(&closeAllConnectionsSync.mutex);
    closeAllConnectionsSync.requestedFlag = true;
    // Do a synchronous connections closed (effective on client only)
    SOPC_EventHandler_Post(servicesEventHandler, APP_TO_SE_CLOSE_ALL_CONNECTIONS, 0, NULL, 0);
    while (closeAllConnectionsSync.allDisconnectedFlag == false)
    {
        Mutex_UnlockAndWaitCond(&closeAllConnectionsSync.cond, &closeAllConnectionsSync.mutex);
    }
    Mutex_Unlock(&closeAllConnectionsSync.mutex);
    Mutex_Clear(&closeAllConnectionsSync.mutex);
    Condition_Clear(&closeAllConnectionsSync.cond);
}

void SOPC_Services_Clear()
{
    io_dispatch_mgr__UNINITIALISATION();

    SOPC_Looper_Delete(servicesLooper);

    closeAllConnectionsSync.allDisconnectedFlag = false;
    closeAllConnectionsSync.requestedFlag = false;
}
