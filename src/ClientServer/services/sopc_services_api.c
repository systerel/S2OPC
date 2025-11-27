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

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_enums.h"
#include "sopc_helper_string.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
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
    SOPC_Mutex mutex;
    SOPC_Condition cond;
    bool allDisconnectedFlag;
    bool requestedFlag;
    bool clientOnlyFlag;
} closeAllConnectionsSync = {.allDisconnectedFlag = false, .requestedFlag = false, .clientOnlyFlag = false};

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

static void onSecureChannelEvent(SOPC_EventHandler* handler,
                                 int32_t event,
                                 uint32_t id,
                                 uintptr_t params,
                                 uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
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
    case SC_SERVICE_RCV_MSG:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SC_SC_SERVICE_RCV_MSG scIdx=%" PRIu32 " reqId/0=%" PRIuPTR, id, auxParam);

        // id ==  connection Id
        // params = message content (byte buffer)
        // auxParam == requestId (server) / 0 (client)
        SOPC_ASSERT(NULL != (void*) params);
        io_dispatch_mgr__receive_msg_buffer(id, (constants__t_byte_buffer_i) params,
                                            (constants__t_request_context_i) auxParam, &bres);
        if (!bres)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ServicesMgr: SC_SC_SERVICE_RCV_MSG scIdx=%" PRIu32 " reqId/0=%" PRIuPTR
                                   " received message considered invalid",
                                   id, auxParam);
        }
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
            sCode = constants_statuscodes_bs__e_sc_bad_generic;
        }
        break;
    case SE_TO_SE_SERVER_DATA_CHANGED:
        /* Server side only:
           params = (SOPC_Array*) array of SOPC_WriteDataChanged
         */
        SOPC_ASSERT((void*) params != NULL);

        dataOrNodeChangedArray = (SOPC_Array*) params;
        bres = true;
        length = SOPC_Array_Size(dataOrNodeChangedArray);
        for (size_t i = 0; i < length; i++)
        {
            writeDataChanged = (SOPC_WriteDataChanged*) SOPC_Array_Get_Ptr(dataOrNodeChangedArray, i);
            SOPC_ASSERT(writeDataChanged != NULL);
            old_value = writeDataChanged->oldValue;
            new_value = writeDataChanged->newValue;
            /* Note: write values deallocation managed by B model */
            io_dispatch_mgr__internal_server_data_changed(old_value, new_value, &lbres);
            bres = bres && lbres;
        }
        SOPC_Array_Delete(dataOrNodeChangedArray);
        if (bres == false)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ServicesMgr: SE_TO_SE_SERVER_DATA_CHANGED session=%" PRIu32 " treatment failed",
                                   id);
        }
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
           auxParams = (constants_statuscodes_bs__t_StatusCode_i) service result code
         */
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32, id);

        msg_data = (void*) params;
        SOPC_ASSERT(msg_data != NULL);

        io_dispatch_mgr__internal_server_send_publish_response_prio_event(
            (constants__t_session_i) id, msg_data->requestHandle, msg_data->requestId, msg_data->msgToSend,
            (constants_statuscodes_bs__t_StatusCode_i) auxParam, &bres);
        SOPC_Free(msg_data);

        if (bres == false)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ServicesMgr: SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO session=%" PRIu32 " treatment failed", id);
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
    case APP_TO_SE_UNINITIALIZE_SERVICES:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ServicesMgr: APP_TO_SE_UNINITIALIZE_SERVICES");
        io_dispatch_mgr__UNINITIALISATION();
        break;
    default:
        SOPC_ASSERT(false);
    }
}

void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    SOPC_ASSERT(servicesEventHandler != NULL);
    SOPC_EventHandler_Post(servicesEventHandler, (int32_t) seEvent, id, params, auxParam);
}

uint32_t SOPC_Services_Get_QueueSize(void)
{
    return SOPC_EventHandler_Get_QueueSize(servicesEventHandler);
}

void SOPC_Services_Initialize(SOPC_SetListenerFunc* setSecureChannelsListener)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    servicesLooper = SOPC_Looper_Create("Services");
    SOPC_ASSERT(servicesLooper != NULL);

    servicesEventHandler = SOPC_EventHandler_Create(servicesLooper, onServiceEvent);
    SOPC_ASSERT(servicesEventHandler != NULL);

    secureChannelsEventHandler = SOPC_EventHandler_Create(servicesLooper, onSecureChannelEvent);
    SOPC_ASSERT(secureChannelsEventHandler != NULL);

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

void SOPC_Services_Clear(void)
{
    SOPC_EventHandler_Post(servicesEventHandler, APP_TO_SE_UNINITIALIZE_SERVICES, 0, 0, 0);

    // Set to NULL handlers deallocated by SOPC_Looper_Delete call
    servicesEventHandler = NULL;
    secureChannelsEventHandler = NULL;
    SOPC_Looper_Delete(servicesLooper);
    servicesLooper = NULL;

    closeAllConnectionsSync.allDisconnectedFlag = false;
    closeAllConnectionsSync.clientOnlyFlag = false;
    closeAllConnectionsSync.requestedFlag = false;
    SOPC_Mutex_Clear(&closeAllConnectionsSync.mutex);
    SOPC_Condition_Clear(&closeAllConnectionsSync.cond);
}
