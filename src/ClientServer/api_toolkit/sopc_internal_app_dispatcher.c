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

#include "app_cb_call_context_internal.h"
#include "sopc_encodeable.h"
#include "sopc_encodeabletype.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

SOPC_ComEvent_Fct* appEventCallback = NULL;
SOPC_AddressSpaceNotif_Fct* appAddressSpaceNotificationCallback = NULL;

SOPC_Looper* appLooper = NULL;
SOPC_EventHandler* appComEventHandler = NULL;
SOPC_EventHandler* appAddressSpaceNotificationHandler = NULL;

static void onComEvent(SOPC_EventHandler* handler, int32_t event, uint32_t id, uintptr_t uintParams, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    void* params = (void*) uintParams; // Always used as a pointer content here
    SOPC_App_Com_Event comEvent = (SOPC_App_Com_Event) event;
    SOPC_EncodeableType* encType = NULL;

    switch (comEvent)
    {
    case SE_SESSION_ACTIVATION_FAILURE:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_SESSION_ACTIVATION_FAILURE session=%" PRIu32 " context=%" PRIuPTR, id,
                               auxParam);
        break;
    case SE_ACTIVATED_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_ACTIVATED_SESSION session=%" PRIu32 " context=%" PRIuPTR, id, auxParam);
        break;
    case SE_SESSION_REACTIVATING:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_SESSION_REACTIVATING session=%" PRIu32 " context=%" PRIuPTR, id, auxParam);
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_RCV_SESSION_RESPONSE  session=%" PRIu32 " msgTyp=%s context=%" PRIuPTR, id,
                               SOPC_EncodeableType_GetName(encType), auxParam);
        break;
    case SE_CLOSED_SESSION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_CLOSED_SESSION session=%" PRIu32 " context=%" PRIuPTR, id, auxParam);
        break;
    case SE_RCV_DISCOVERY_RESPONSE:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_RCV_DISCOVERY_RESPONSE msgTyp=%s context=%" PRIuPTR,
                               SOPC_EncodeableType_GetName(encType), auxParam);
        break;
    case SE_SND_REQUEST_FAILED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_SND_REQUEST_FAILED retStatus=%" PRIu32 " msgTyp=%s context=%" PRIuPTR, id,
                               SOPC_EncodeableType_GetName((SOPC_EncodeableType*) params), auxParam);
        break;
    case SE_CLOSED_ENDPOINT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_CLOSED_ENDPOINT idx=%" PRIu32 " retStatus=%" PRIuPTR, id, auxParam);
        break;
    case SE_LOCAL_SERVICE_RESPONSE:
        if (params != NULL)
        {
            encType = *(SOPC_EncodeableType**) params;
        }
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "App: SE_LOCAL_SERVICE_RESPONSE  idx=%" PRIu32 " msgTyp=%s context=%" PRIuPTR, id,
                               SOPC_EncodeableType_GetName(encType), auxParam);
        break;
    default:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "App: UNKOWN EVENT");
        break;
    }

    if (NULL != appEventCallback)
    {
        if (comEvent == SE_ACTIVATED_SESSION)
        {
            appEventCallback(comEvent,
                             id, // session id
                             NULL,
                             auxParam); // session context
        }
        else
        {
            appEventCallback(comEvent, id,
                             params,    // see event definition of params
                             auxParam); // application context
        }
    }

    if (NULL != encType && NULL != params)
    {
        // Message to deallocate => if not application shall deallocate !
        SOPC_Encodeable_Delete(encType, &params);
    }
}

static void onAddressSpaceNotification(SOPC_EventHandler* handler,
                                       int32_t event,
                                       uint32_t id,
                                       uintptr_t uintParams,
                                       uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_UNUSED_ARG(id);
    void* params = (void*) uintParams; // Always used as a pointer content here
    OpcUa_WriteValue* wv = NULL;
    SOPC_CallContext* cc = (SOPC_CallContext*) auxParam;
    uintptr_t initialAuxParam = 0;

    if (NULL != cc)
    {
        initialAuxParam = cc->auxParam;
    }

    SOPC_App_AddSpace_Event asEvent = (SOPC_App_AddSpace_Event) event;

    switch (asEvent)
    {
    case AS_WRITE_EVENT:
    {
        wv = (OpcUa_WriteValue*) params;
        if (wv != NULL)
        {
            char* nodeId = SOPC_NodeId_ToCString(&wv->NodeId);

            if (nodeId != NULL)
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "App: AS_WRITE_EVENT on NodeId: %s, AttributeId: %" PRIu32
                                       ", Write status: %" PRIX32,
                                       nodeId, wv->AttributeId, (SOPC_StatusCode) initialAuxParam);
                SOPC_Free(nodeId);
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "App: AS_WRITE_EVENT (WriteValue or NodeId string invalid)");
            }
        }

        if (NULL != appAddressSpaceNotificationCallback)
        {
            appAddressSpaceNotificationCallback(cc, asEvent, params, (SOPC_StatusCode) initialAuxParam);
        }

        if (NULL != params)
        {
            OpcUa_WriteValue_Clear((OpcUa_WriteValue*) params);
            SOPC_Free(params);
        }
        break;
    }
    default:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "App: UNKOWN AS EVENT");
        break;
    }

    SOPC_CallContext_Free(cc);
}

void SOPC_App_Initialize(void)
{
    appLooper = SOPC_Looper_Create("Application");
    assert(appLooper != NULL);

    appComEventHandler = SOPC_EventHandler_Create(appLooper, onComEvent);
    assert(appComEventHandler != NULL);

    appAddressSpaceNotificationHandler = SOPC_EventHandler_Create(appLooper, onAddressSpaceNotification);
    assert(appAddressSpaceNotificationHandler != NULL);
}

void SOPC_App_Clear(void)
{
    SOPC_Looper_Delete(appLooper);
    appLooper = NULL;
    appComEventHandler = NULL;
    appAddressSpaceNotificationHandler = NULL;
}

static uintptr_t getCurrentContextCopyWithAuxParams(uintptr_t auxParam)
{
    SOPC_CallContext* newCC = SOPC_CallContext_Copy(SOPC_CallContext_GetCurrent());
    if (NULL != newCC)
    {
        newCC->auxParam = auxParam;
    }
    return (uintptr_t) newCC;
}

SOPC_ReturnStatus SOPC_App_EnqueueComEvent(SOPC_App_Com_Event event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    return SOPC_EventHandler_Post(appComEventHandler, (int32_t) event, id, params, auxParam);
}

SOPC_ReturnStatus SOPC_App_EnqueueAddressSpaceNotification(SOPC_App_AddSpace_Event event,
                                                           uint32_t id,
                                                           uintptr_t params,
                                                           uintptr_t auxParam)
{
    return SOPC_EventHandler_Post(appAddressSpaceNotificationHandler, (int32_t) event, id, params,
                                  getCurrentContextCopyWithAuxParams(auxParam));
}
