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

#include <string.h>

#include "sopc_toolkit_async_api.h"

#include "sopc_encodeable.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_services_api.h"
#include "sopc_services_api_internal.h"
#include "sopc_types.h"

void SOPC_ToolkitServer_AsyncOpenEndpoint(SOPC_EndpointConfigIdx endpointConfigIdx)
{
    // TODO: check valid config and return bool
    SOPC_Services_EnqueueEvent(APP_TO_SE_OPEN_ENDPOINT, endpointConfigIdx, (uintptr_t) NULL, 0);
}

void SOPC_ToolkitServer_AsyncCloseEndpoint(SOPC_EndpointConfigIdx endpointConfigIdx)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_ENDPOINT, endpointConfigIdx, (uintptr_t) NULL, 0);
}

void SOPC_ToolkitServer_AsyncLocalServiceRequest(SOPC_EndpointConfigIdx endpointConfigIdx,
                                                 void* requestStruct,
                                                 uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_LOCAL_SERVICE_REQUEST, endpointConfigIdx, (uintptr_t) requestStruct,
                               requestContext);
}

bool SOPC_ToolkitClient_AsyncActivateSession(SOPC_EndpointConnectionConfigIdx endpointConnectionIdx,
                                             const char* sessionName,
                                             uintptr_t sessionContext,
                                             SOPC_ExtensionObject* userToken)
{
    SOPC_Internal_SessionAppContext* sessionAppContext = SOPC_Calloc(1, sizeof(*sessionAppContext));
    if (NULL == sessionAppContext)
    {
        return false;
    }
    if (NULL != sessionName)
    {
        size_t len = strlen(sessionName);
        sessionAppContext->sessionName = SOPC_Calloc(len, sizeof(*sessionAppContext->sessionName));
        if (NULL != sessionAppContext->sessionName)
        {
            sessionAppContext->sessionName = strncpy(sessionAppContext->sessionName, sessionName, len - 1);
        }
        else
        {
            SOPC_Free(sessionAppContext);
            return false;
        }
    }
    sessionAppContext->userSessionContext = sessionContext;
    SOPC_Services_EnqueueEvent(APP_TO_SE_ACTIVATE_SESSION, endpointConnectionIdx, (uintptr_t) userToken,
                               (uintptr_t) sessionAppContext);
    return true;
}

SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_Anonymous(
    SOPC_EndpointConnectionConfigIdx endpointConnectionIdx,
    const char* sessionName,
    uintptr_t sessionContext,
    const char* policyId)
{
    if (NULL == policyId || strlen(policyId) == 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ExtensionObject* user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_AnonymousIdentityToken* token = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == user)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_Encodeable_CreateExtension(user, &OpcUa_AnonymousIdentityToken_EncodeableType, (void**) &token);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_InitializeFromCString(&token->PolicyId, policyId);
    }

    if (SOPC_STATUS_OK == status)
    {
        bool res = SOPC_ToolkitClient_AsyncActivateSession(endpointConnectionIdx, sessionName, sessionContext, user);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create anonymous UserIdentityToken.");
        SOPC_ExtensionObject_Clear(user);
        SOPC_Free(user);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
    SOPC_EndpointConnectionConfigIdx endpointConnectionIdx,
    const char* sessionName,
    uintptr_t sessionContext,
    const char* policyId,
    const char* username,
    const uint8_t* password,
    int32_t length_password)
{
    if (NULL == policyId || strlen(policyId) == 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ExtensionObject* user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_UserNameIdentityToken* token = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == user)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_Encodeable_CreateExtension(user, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&token->UserName);
        SOPC_ByteString_Initialize(&token->Password);
        SOPC_String_Initialize(&token->EncryptionAlgorithm);
        status = SOPC_String_InitializeFromCString(&token->PolicyId, policyId);
    }
    if (SOPC_STATUS_OK == status && NULL != username)
    {
        status = SOPC_String_InitializeFromCString(&token->UserName, username);
    }
    if (SOPC_STATUS_OK == status && NULL != password && length_password > 0)
    {
        status = SOPC_ByteString_CopyFromBytes(&token->Password, password, length_password);
    }

    if (SOPC_STATUS_OK == status)
    {
        bool res = SOPC_ToolkitClient_AsyncActivateSession(endpointConnectionIdx, sessionName, sessionContext, user);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create username UserIdentityToken.");
        SOPC_ExtensionObject_Clear(user);
        SOPC_Free(user);
    }

    return status;
}

void SOPC_ToolkitClient_AsyncSendRequestOnSession(SOPC_SessionId sessionId,
                                                  void* requestStruct,
                                                  uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_SEND_SESSION_REQUEST, sessionId, (uintptr_t) requestStruct, requestContext);
}

void SOPC_ToolkitClient_AsyncCloseSession(SOPC_SessionId sessionId)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_SESSION, sessionId, (uintptr_t) NULL, 0);
}

void SOPC_ToolkitClient_AsyncSendDiscoveryRequest(SOPC_EndpointConnectionConfigIdx endpointConnectionIdx,
                                                  void* discoveryReqStruct,
                                                  uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_SEND_DISCOVERY_REQUEST, endpointConnectionIdx, (uintptr_t) discoveryReqStruct,
                               requestContext);
}
