/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_secure_channels_api.h"
#include "sopc_toolkit_constants.h"

#include "sopc_logger.h"
#include "sopc_mutexes.h"
#include "sopc_services_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include "io_dispatch_mgr.h"
#include "service_mgr_bs.h"
#include "toolkit_header_init.h"
#include "util_b2c.h"

static SOPC_EventDispatcherManager* servicesEventDispatcherMgr = NULL;

static SOPC_EventDispatcherManager* applicationEventDispatcherMgr = NULL;
#ifdef __TRUSTINSOFT_NO_MTHREAD__
// TIS_ServicesEventDispatch and TIS_AppEventDispatch
void* SOPC_ThreadStartEventDispatcherManager(void* pEventMgr);
void TIS_ServicesEventDispatch (void) {
  SOPC_ThreadStartEventDispatcherManager (servicesEventDispatcherMgr);
}
void TIS_AppEventDispatch (void) {
  SOPC_ThreadStartEventDispatcherManager (applicationEventDispatcherMgr);
}
#endif

// Structure used to close all connections in a synchronous way
// (necessary on toolkit clear)
static struct
{
    Mutex mutex;
    Condition cond;
    bool allDisconnectedFlag;
    bool requestedFlag;
} closeAllConnectionsSync = {.requestedFlag = false, .allDisconnectedFlag = false};

SOPC_EventDispatcherManager* SOPC_Services_GetEventDispatcher()
{
    return servicesEventDispatcherMgr;
}

SOPC_EventDispatcherManager* SOPC_ApplicationCallback_GetEventDispatcher()
{
    return applicationEventDispatcherMgr;
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

void SOPC_ServicesEventDispatcher(int32_t scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
#ifdef __TRUSTINSOFT_DEBUG__
  printf ("TIS: SOPC_ServicesEventDispatcher (TIS_ServicesEventDispatch)\n");
#endif
    SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* epConfig = NULL;
    constants__t_StatusCode_i sCode = constants__e_sc_ok;
    SOPC_EncodeableType* encType = NULL;
    bool bres = false;
    void* msg = NULL;
    switch (event)
    {
    /* SC to Services events */
    case SC_TO_SE_EP_SC_CONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_EP_SC_CONNECTED epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32
                               " scIdx=%" PRIuPTR,
                               id, params == NULL ? 0 : *(uint32_t*) params, auxParam);

        // id ==  endpoint configuration index
        // params = channel configuration index POINTER
        // auxParam == connection Id
        if (auxParam <= INT32_MAX)
        {
            io_dispatch_mgr__server_channel_connected_event(id, *(uint32_t*) params, auxParam, &bres);
        }
        if (bres == false)
        {
            SOPC_Logger_TraceError("Services: channel state incoherent epCfgIdx=%" PRIu32 " scIdx=%" PRIuPTR, id,
                                   auxParam);

            SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT, auxParam, NULL, 0);
        }
        break;
    case SC_TO_SE_EP_CLOSED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_EP_CLOSED epCfgIdx=%" PRIu32 " returnStatus=%" PRIdPTR, id,
                               auxParam);
        // id == endpoint configuration index
        // params = NULL
        // auxParam == status
        // => B model entry point to add
        status = SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                                      SOPC_AppEvent_ComEvent_Create(SE_CLOSED_ENDPOINT), id, NULL,
                                                      auxParam, "Endpoint is closed !");
        break;
    case SC_TO_SE_SC_CONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_SC_CONNECTED scIdx=%" PRIu32 " scCfgIdx=%" PRIuPTR, id, auxParam);
        // id == connection Id
        // auxParam == secure channel configuration index
        // => B model entry point to add
        if (auxParam <= INT32_MAX)
        {
            io_dispatch_mgr__client_channel_connected_event(auxParam, id);
        }
        break;
    case SC_TO_SE_SC_CONNECTION_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_SC_CONNECTION_TIMEOUT scCfgIdx=%" PRIu32, id);

        // id == secure channel configuration index
        // => B model entry point to add
        io_dispatch_mgr__client_secure_channel_timeout(id);
        break;
    case SC_TO_SE_SC_DISCONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_SC_DISCONNECTED scIdx=%" PRIu32, id);
        // id == connection Id ==> TMP: secure channel config idx
        // auxParam = status
        // => B model entry point to add
        // secure_channel_lost call !
        io_dispatch_mgr__secure_channel_lost((constants__t_channel_i) id);
        break;
    case SE_TO_SE_SC_ALL_DISCONNECTED:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_SC_ALL_DISCONNECTED");
        // Call directly toolkit configuration callback
        SOPC_Internal_AllClientSecureChannelsDisconnected();
        break;

    case SE_TO_SE_ACTIVATE_ORPHANED_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_ACTIVATE_ORPHANED_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR,
                               id, auxParam);

        if (auxParam <= INT32_MAX)
        {
            io_dispatch_mgr__internal_client_activate_orphaned_session((constants__t_session_i) id,
                                                                       (constants__t_channel_config_idx_i) auxParam);
        }
        break;
    case SE_TO_SE_ACTIVATE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32, id);

        if (NULL != params)
        {
            io_dispatch_mgr__client_reactivate_session_new_user((constants__t_session_i) id,
                                                                *(constants__t_user_i*) params);
            free(params);
        }
        else
        {
            SOPC_Logger_TraceError("ServicesMgr: SE_TO_SE_ACTIVATE_SESSION session=%" PRIu32 " user parameter is NULL",
                                   id);
            sCode = constants__e_sc_bad_generic;
        }
        break;
    case SE_TO_SE_CREATE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_CREATE_SESSION session=%" PRIu32 " scCfgIdx=%" PRIuPTR, id,
                               auxParam);
        if (auxParam <= INT32_MAX)
        {
            io_dispatch_mgr__internal_client_create_session((constants__t_session_i) id,
                                                            (constants__t_channel_config_idx_i) auxParam);
        }
        break;

    case TIMER_SE_EVAL_SESSION_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: SE_TO_SE_CREATE_SESSION session=%" PRIu32, id);

        io_dispatch_mgr__internal_server_evaluate_session_timeout((constants__t_session_i) id);
        break;
    case SC_TO_SE_SC_SERVICE_RCV_MSG:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_SC_SERVICE_RCV_MSG scIdx=%" PRIu32 " reqId=%" PRIuPTR, id,
                               auxParam);

        // id ==  connection Id
        // params = message content (byte buffer)
        // auxParam == context (request id)
        assert(NULL != params && INT32_MAX >= auxParam);
        io_dispatch_mgr__receive_msg_buffer((constants__t_channel_i) id, (constants__t_byte_buffer_i) params,
                                            (constants__t_request_context_i) auxParam);
        // params is freed by services manager
        break;

    case SC_TO_SE_SND_FAILURE:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_SND_FAILURE scIdx=%" PRIu32 " reqId=%" PRIu32
                               " statusCode=%" PRIXPTR,
                               id, params == NULL ? 0 : *(uint32_t*) params, auxParam);

        if (NULL != params)
        {
            util_status_code__C_to_B(auxParam, &sCode);
            io_dispatch_mgr__snd_msg_failure((constants__t_channel_i) id,
                                             (constants__t_request_handle_i) * (uint32_t*) params, sCode);
            free(params);
        } // else: without request Id, it cannot be treated
        break;
    case SC_TO_SE_REQUEST_TIMEOUT:
        SOPC_Logger_TraceDebug("ServicesMgr: SC_TO_SE_REQUEST_TIMEOUT scIdx=%" PRIu32 " reqHandle=%" PRIuPTR, id,
                               auxParam);

        /* id = secure channel connection index,
           auxParam = request handle */
        io_dispatch_mgr__internal_client_request_timeout(id, auxParam);
        break;

    /* App to Services events */
    case APP_TO_SE_OPEN_ENDPOINT:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_OPEN_ENDPOINT epCfgIdx=%" PRIu32, id);

        // id ==  endpoint configuration index
        // => B model entry point to add
        epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
        if (NULL == epConfig)
        {
            status = SOPC_EventDispatcherManager_AddEvent(
                applicationEventDispatcherMgr, SOPC_AppEvent_ComEvent_Create(SE_CLOSED_ENDPOINT), id, NULL,
                SOPC_STATUS_INVALID_PARAMETERS, "Endpoint configuration is invalid !");
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
            status = SOPC_EventDispatcherManager_AddEvent(
                applicationEventDispatcherMgr, SOPC_AppEvent_ComEvent_Create(SE_CLOSED_ENDPOINT), id, NULL,
                SOPC_STATUS_INVALID_PARAMETERS, "Endpoint configuration is invalid !");
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
        io_dispatch_mgr__server_treat_local_service_request(id, params, auxParam, &sCode);
        if (constants__e_sc_ok != sCode)
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
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_LOCAL_SERVICE_RESPONSE), id, msg,
                                            auxParam);
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
        io_dispatch_mgr__client_activate_new_session(id, *(uint32_t*) params, auxParam, &bres);
        if (bres == false)
        {
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_SESSION_ACTIVATION_FAILURE),
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
        io_dispatch_mgr__client_send_service_request(id, params, auxParam, &sCode);
        if (sCode != constants__e_sc_ok)
        {
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_SND_REQUEST_FAILED),
                                            util_status_code__B_to_return_status_C(sCode), encType, auxParam);
            status = SOPC_STATUS_NOK;

            SOPC_Logger_TraceWarning("ServicesMgr: APP_TO_SE_SEND_SESSION_REQUEST failed session=%" PRIu32
                                     " msgType=%s ctx=%" PRIuPTR,
                                     id, SOPC_EncodeableType_GetName(encType), auxParam);
        }
        break;
    case APP_TO_SE_CLOSE_SESSION:
        SOPC_Logger_TraceDebug("ServicesMgr: APP_TO_SE_CLOSE_SESSION  session=%" PRIu32, id);

        // id == session id
        io_dispatch_mgr__client_send_close_session_request(id, &sCode);
        if (sCode != constants__e_sc_ok)
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
        io_dispatch_mgr__client_send_discovery_request(id, params, auxParam, &sCode);
        if (sCode != constants__e_sc_ok)
        {
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_SND_REQUEST_FAILED),
                                            util_status_code__B_to_return_status_C(sCode), encType, auxParam);
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
            SOPC_ServicesEventDispatcher(SE_TO_SE_SC_ALL_DISCONNECTED, id, params, auxParam);
        }
        break;
    default:
        assert(false);
    }
}

void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    if (servicesEventDispatcherMgr != NULL)
    {
        SOPC_EventDispatcherManager_AddEvent(servicesEventDispatcherMgr, seEvent, id, params, auxParam, NULL);
    }
}

void SOPC_ServicesToApp_EnqueueEvent(SOPC_App_Com_Event appEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    if (applicationEventDispatcherMgr != NULL)
    {
        SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr, appEvent, id, params, auxParam, NULL);
    }
}

void SOPC_Services_Initialize()
{
    servicesEventDispatcherMgr =
        SOPC_EventDispatcherManager_CreateAndStart(SOPC_ServicesEventDispatcher, "Services event dispatcher manager");
    applicationEventDispatcherMgr = SOPC_EventDispatcherManager_CreateAndStart(
        SOPC_Internal_ApplicationEventDispatcher, "(Services) Application event dispatcher manager");
    // Init async close management flag
    Mutex_Initialization(&closeAllConnectionsSync.mutex);
    Condition_Init(&closeAllConnectionsSync.cond);
}

void SOPC_Services_ToolkitConfigured()
{
    /* Init B model */
    INITIALISATION();
}

#ifdef __TRUSTINSOFT_NO_MTHREAD__
// use spec for SOPC_Services_PreClear in order to avoid infinite loop
//@ assigns closeAllConnectionsSync \from \nothing;
#endif
void SOPC_Services_PreClear()
{
    Mutex_Lock(&closeAllConnectionsSync.mutex);
    closeAllConnectionsSync.requestedFlag = true;
    // Do a synchronous connections closed (effective on client only)
    SOPC_EventDispatcherManager_AddEvent(servicesEventDispatcherMgr, APP_TO_SE_CLOSE_ALL_CONNECTIONS, 0, NULL, 0,
                                         "Services: Close all channel !");
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
    address_space_bs__UNINITIALISATION();
    service_mgr_bs__UNINITIALISATION();

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_EventDispatcherManager_StopAndDelete(&servicesEventDispatcherMgr);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError("Services event mgr: error status when stopping '%d'", status);
    }
    status = SOPC_EventDispatcherManager_StopAndDelete(&applicationEventDispatcherMgr);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError("Application event mgr: error status when stopping '%d'", status);
    }
}
