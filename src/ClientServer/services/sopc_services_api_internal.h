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

/**
 *  \file sopc_services_api_internal.h
 *
 *  \brief Event oriented API of the Services layer for internal use only (from Services layer).
 */

#ifndef SOPC_SERVICES_API_INTERNAL_H
#define SOPC_SERVICES_API_INTERNAL_H

#include "sopc_event.h"
#include "sopc_key_manager.h"
#include "sopc_services_api.h"
#include "sopc_user_app_itf.h"

typedef struct SOPC_Internal_AsyncSendMsgData
{
    uint32_t requestId;     // t_request_context
    uint32_t requestHandle; // t_request_handle
    void* msgToSend;        // OpcUa_<Msg> *
    uint32_t bStatusCode;   // constants_statuscodes_bs__t_StatusCode_i
} SOPC_Internal_AsyncSendMsgData;

typedef struct SOPC_Internal_SessionAppContext
{
    SOPC_ExtensionObject* userToken;
    char* sessionName;
    uintptr_t userSessionContext;
    SOPC_SerializedAsymmetricKey* userTokenKey;
} SOPC_Internal_SessionAppContext;

typedef struct SOPC_Internal_DiscoveryContext
{
    void* opcuaMessage;            /**< (OpcUa_<MessageStruct>*) OPC UA message payload structure (header ignored)*/
    uintptr_t discoveryAppContext; /**< User application request context */
} SOPC_Internal_DiscoveryContext;

typedef struct SOPC_Internal_EventContext
{
    SOPC_NodeId notifierNodeId;
    SOPC_Event* event;
    SOPC_SessionId optSessionId;
    uint32_t optSubscriptionId;
    uint32_t optMonitoredItemId;
} SOPC_Internal_EventContext;

#endif /* SOPC_SERVICES_API_INTERNAL_H */
