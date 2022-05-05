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
#include <stdlib.h>
#include <string.h>

#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

static uint32_t nSMCreated = 0; /* Number of created machines, used for session context. */
static uint32_t nDiscovery = 0; /* Number of sent discovery requests, used as UID for requestContext */
static uint32_t nReqSent = 0;   /* Number of other requests sent through the wrapper, used as UID for requestContext */

StateMachine_Machine* StateMachine_Create(void)
{
    StateMachine_Machine* pSM = SOPC_Malloc(sizeof(StateMachine_Machine));
    StateMachine_RequestContext* pCtxSess = SOPC_Malloc(sizeof(StateMachine_RequestContext));

    if (pSM == NULL || pCtxSess == NULL || Mutex_Initialization(&pSM->mutex) != SOPC_STATUS_OK)
    {
        SOPC_Free(pSM);
        SOPC_Free(pCtxSess);
        return NULL;
    }

    /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX opened sessions */
    ++nSMCreated;
    pCtxSess->uid = nSMCreated;
    pCtxSess->appCtx = 0;
    pSM->state = stInit;
    pSM->pscConfig = NULL;
    pSM->iscConfig = 0;
    pSM->pCtxSession = pCtxSess;
    pSM->iSessionID = 0;
    pSM->pCtxRequest = NULL;

    return pSM;
}

static SOPC_ReturnStatus StateMachine_InternalConfigureMachine(StateMachine_Machine* pSM,
                                                               const char* reqSecuPolicyUri,
                                                               OpcUa_MessageSecurityMode msgSecurityMode)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Add the SecureChannel configuration */
    if (SOPC_STATUS_OK == status)
    {
        pSM->pscConfig = Config_NewSCConfig(reqSecuPolicyUri, msgSecurityMode);

        if (NULL == pSM->pscConfig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->iscConfig = SOPC_ToolkitClient_AddSecureChannelConfig(pSM->pscConfig);
        if (0 == pSM->iscConfig)
        {
            status = SOPC_STATUS_NOK;
            printf("# Error: AddSecureChannelConfig failed.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stConfigured;
    }
    else if (pSM != NULL)
    {
        pSM->state = stError;
    }

    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus StateMachine_ConfigureMachine(StateMachine_Machine* pSM, bool sign, bool encrypt)
{
    if (sign && encrypt)
    {
        return StateMachine_InternalConfigureMachine(pSM, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                                     OpcUa_MessageSecurityMode_SignAndEncrypt);
    }
    else if (sign)
    {
        return StateMachine_InternalConfigureMachine(pSM, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                                     OpcUa_MessageSecurityMode_Sign);
    }
    else
    {
        return StateMachine_InternalConfigureMachine(pSM, SOPC_SecurityPolicy_None_URI, OpcUa_MessageSecurityMode_None);
    }
}

typedef struct
{
    bool anonymous;
    const char* policyId;
    const char* username;
    const uint8_t* password;
    int32_t length;
} activation_type;

static SOPC_ReturnStatus ActivateSession(StateMachine_Machine* pSM, activation_type activation_parameters)
{
    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    if (pSM->state != stConfigured)
    {
        printf("# Error: The state machine shall be in stConfigured state to start a session.\n");
        pSM->state = stError;
        mutStatus = Mutex_Unlock(&pSM->mutex);
        assert(SOPC_STATUS_OK == mutStatus);
        return SOPC_STATUS_NOK;
    }

    if (activation_parameters.anonymous)
    {
        SOPC_ToolkitClient_AsyncActivateSession_Anonymous(pSM->iscConfig, SESSION_NAME, (uintptr_t) pSM->pCtxSession,
                                                          activation_parameters.policyId);
    }
    else
    {
        SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
            pSM->iscConfig, SESSION_NAME, (uintptr_t) pSM->pCtxSession, activation_parameters.policyId,
            activation_parameters.username, activation_parameters.password, activation_parameters.length);
    }

    pSM->state = stActivating;
    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus StateMachine_StartSession_Anonymous(StateMachine_Machine* pSM, const char* policyId)
{
    activation_type activ = {.anonymous = true, .policyId = policyId};
    return ActivateSession(pSM, activ);
}

SOPC_ReturnStatus StateMachine_StartSession_UsernamePassword(StateMachine_Machine* pSM,
                                                             const char* policyId,
                                                             const char* username,
                                                             const char* password)
{
    activation_type activ = {.anonymous = false,
                             .policyId = policyId,
                             .username = username,
                             .password = (const uint8_t*) password,
                             .length = (int32_t) strlen(password)};
    return ActivateSession(pSM, activ);
}

static bool is_connected_unlocked(StateMachine_Machine* pSM)
{
    switch (pSM->state)
    {
    case stActivating:
    case stActivated:
    case stClosing:
        return true;
    default:
        return false;
    }
}

SOPC_ReturnStatus StateMachine_StopSession(StateMachine_Machine* pSM)
{
    if (pSM == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!is_connected_unlocked(pSM))
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncCloseSession(pSM->iSessionID);
        pSM->state = stClosing;
    }
    else
    {
        printf("# Error: StopSession on a disconnected machine.\n");
        pSM->state = stError;
    }

    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus StateMachine_StartDiscovery(StateMachine_Machine* pSM)
{
    if (pSM == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_GetEndpointsRequest* pReq = NULL;

    if (pSM->state != stConfigured)
    {
        status = SOPC_STATUS_NOK;
        printf("# Error: The state machine shall be in stConfigured state to send a discovery request.\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_GetEndpointsRequest_EncodeableType, (void**) &pReq);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_AttachFromCstring(&pReq->EndpointUrl, ENDPOINT_URL);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Could not create the GetEndpointsRequest.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX pending discoveries */
        ++nDiscovery;
        pSM->pCtxRequest = SOPC_Malloc(sizeof(StateMachine_RequestContext));
        if (NULL == pSM->pCtxRequest)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            pSM->pCtxRequest->uid = nDiscovery;
            pSM->pCtxRequest->appCtx = 0;
            SOPC_ToolkitClient_AsyncSendDiscoveryRequest(pSM->iscConfig, pReq, (uintptr_t) pSM->pCtxRequest);
            pSM->state = stDiscovering;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        pSM->state = stError;
        if (NULL != pReq)
        {
            SOPC_Free(pReq);
        }
        if (NULL != pSM->pCtxRequest)
        {
            SOPC_Free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus StateMachine_StartFindServers(StateMachine_Machine* pSM)
{
    if (pSM == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_FindServersRequest* pReq = NULL;

    if (pSM->state != stConfigured)
    {
        status = SOPC_STATUS_NOK;
        printf("# Error: The state machine shall be in stConfigured state to send a find servers request.\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_FindServersRequest_EncodeableType, (void**) &pReq);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_AttachFromCstring(&pReq->EndpointUrl, ENDPOINT_URL);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Could not create the FindServersRequest.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Leave the FindServersRequest's LocaleIds ans ServerUris empty */

        /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX pending discoveries */
        ++nDiscovery;
        pSM->pCtxRequest = SOPC_Malloc(sizeof(StateMachine_RequestContext));
        if (NULL == pSM->pCtxRequest)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            pSM->pCtxRequest->uid = nDiscovery;
            pSM->pCtxRequest->appCtx = 0;
            SOPC_ToolkitClient_AsyncSendDiscoveryRequest(pSM->iscConfig, pReq, (uintptr_t) pSM->pCtxRequest);
            pSM->state = stDiscovering;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        pSM->state = stError;
        if (NULL != pReq)
        {
            SOPC_Free(pReq);
        }
        if (NULL != pSM->pCtxRequest)
        {
            SOPC_Free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

static SOPC_ReturnStatus fillRegisterServerRequest(OpcUa_RegisteredServer* pServ,
                                                   SOPC_LocalizedText* serverName,
                                                   SOPC_String* discoveryURL)
{
    SOPC_ReturnStatus status;

    SOPC_LocalizedText_Initialize(serverName);

    bool fillRequest =
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->ServerUri, DEFAULT_APPLICATION_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->ProductUri, DEFAULT_PRODUCT_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->GatewayServerUri, GATEWAY_SERVER_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->SemaphoreFilePath, "")) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&serverName->defaultLocale, "Locale")) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&serverName->defaultText, "Text")) &&
        (SOPC_STATUS_OK == SOPC_String_InitializeFromCString(discoveryURL, "opc.tcp://test"));

    pServ->NoOfServerNames = 1;
    pServ->NoOfDiscoveryUrls = 1;
    pServ->IsOnline = true;
    pServ->ServerType = OpcUa_ApplicationType_Server;

    if (!fillRequest)
    {
        /* clear ressources */
        SOPC_LocalizedText_Clear(serverName);
        SOPC_Free(serverName);
        SOPC_String_Delete(discoveryURL);
        status = SOPC_STATUS_NOK;
    }
    else
    {
        pServ->ServerNames = serverName;
        pServ->DiscoveryUrls = discoveryURL;
        status = SOPC_STATUS_OK;
    }

    return status;
}

SOPC_ReturnStatus StateMachine_StartRegisterServer(StateMachine_Machine* pSM)
{
    OpcUa_RegisterServerRequest* pReq = NULL;
    SOPC_LocalizedText* serverName = NULL;
    SOPC_String* discoveryURL = NULL;

    if (pSM == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    if (pSM->state != stConfigured)
    {
        status = SOPC_STATUS_NOK;
        printf("# Error: The state machine shall be in stConfigured state to send a register servers request.\n");
    }
    else
    {
        serverName = SOPC_Calloc(1, sizeof(SOPC_LocalizedText));
        discoveryURL = SOPC_String_Create();
        status = SOPC_Encodeable_Create(&OpcUa_RegisterServerRequest_EncodeableType, (void**) &pReq);
    }

    if ((NULL == serverName) || (NULL == discoveryURL) || (SOPC_STATUS_OK != status))
    {
        SOPC_Free(serverName);
        SOPC_String_Delete(discoveryURL);
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = fillRegisterServerRequest(&pReq->Server, serverName, discoveryURL);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not create the RegisterServersRequest.\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Leave the FindServersRequest's LocaleIds and ServerUris empty */

        /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX pending discoveries */
        ++nDiscovery;
        pSM->pCtxRequest = SOPC_Malloc(sizeof(StateMachine_RequestContext));
        if (NULL == pSM->pCtxRequest)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            pSM->pCtxRequest->uid = nDiscovery;
            pSM->pCtxRequest->appCtx = 0;
            SOPC_ToolkitClient_AsyncSendDiscoveryRequest(pSM->iscConfig, pReq, (uintptr_t) pSM->pCtxRequest);
            pSM->state = stRegister;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        /* Free all remaining resources */
        pSM->state = stError;
        SOPC_Free(pSM->pCtxRequest);
        pSM->pCtxRequest = NULL;
        OpcUa_RegisterServerRequest_Clear(pReq);
        SOPC_Free(pReq);
    }

    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return status;
}

SOPC_ReturnStatus StateMachine_SendRequest(StateMachine_Machine* pSM, void* requestStruct, uintptr_t appCtx)
{
    if (pSM == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    StateMachine_RequestContext* ctx = SOPC_Calloc(1, sizeof(StateMachine_RequestContext));

    if (NULL != pSM->pCtxSession || stActivated != pSM->state || ctx == NULL)
    {
        pSM->state = stError;
        SOPC_Free(ctx);
        SOPC_Free(pSM->pCtxRequest);
        pSM->pCtxRequest = NULL;
        return SOPC_STATUS_NOK;
    }

    pSM->pCtxSession = ctx;

    /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX pending requests */
    ++nReqSent;
    pSM->pCtxRequest->uid = nReqSent;
    pSM->pCtxRequest->appCtx = appCtx;
    SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, requestStruct, (uintptr_t) pSM->pCtxRequest);

    mutStatus = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return SOPC_STATUS_OK;
}

bool StateMachine_IsConnectable(StateMachine_Machine* pSM)
{
    assert(pSM != NULL);

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);
    bool is_connectable = stConfigured == pSM->state;
    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return is_connectable;
}

bool StateMachine_IsConnected(StateMachine_Machine* pSM)
{
    assert(pSM != NULL);

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);
    bool bConnected = is_connected_unlocked(pSM);
    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return bConnected;
}

bool StateMachine_IsDiscovering(StateMachine_Machine* pSM)
{
    assert(pSM != NULL);

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);
    bool is_discovering = stDiscovering == pSM->state;
    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return is_discovering;
}

bool StateMachine_IsIdle(StateMachine_Machine* pSM)
{
    assert(pSM != NULL);

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    bool bIdle = false;

    switch (pSM->state)
    {
    case stInit:
    case stConfigured:
    case stError:
        bIdle = true;
        break;
    default:
        break;
    }

    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return bIdle;
}

void StateMachine_Delete(StateMachine_Machine** ppSM)
{
    StateMachine_Machine* pSM = NULL;

    if (NULL == ppSM || NULL == *ppSM)
    {
        return;
    }

    pSM = *ppSM;

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);
    SOPC_Free(pSM->pCtxSession);
    pSM->pCtxSession = NULL;
    SOPC_Free(pSM->pCtxRequest);
    pSM->pCtxRequest = NULL;
    Config_DeleteSCConfig(&(pSM->pscConfig));
    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);
    Mutex_Clear(&pSM->mutex);
    SOPC_Free(pSM);
    *ppSM = NULL;
}

bool StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  uintptr_t* pAppCtx,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx)
{
    /* avoid unused parameter compiler warning */
    SOPC_UNUSED_ARG(pParam);

    bool bProcess = true;

    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = Mutex_Lock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    /* Is this event targeted to the machine? */
    if (bProcess)
    {
        switch (event)
        {
        /* appCtx is session context */
        case SE_SESSION_ACTIVATION_FAILURE:
        case SE_ACTIVATED_SESSION:
        case SE_SESSION_REACTIVATING:
        case SE_CLOSED_SESSION:
            if ((uintptr_t) pSM->pCtxSession != appCtx)
            {
                bProcess = false;
            }
            break;
        /* arg is session id */
        case SE_RCV_SESSION_RESPONSE:
            if (pSM->iSessionID != arg)
            {
                bProcess = false;
            }
            break;
        /* appCtx is request context */
        case SE_RCV_DISCOVERY_RESPONSE:
        case SE_SND_REQUEST_FAILED:
            if ((uintptr_t) pSM->pCtxRequest != appCtx)
            {
                bProcess = false;
            }
            break;
        default:
            printf("# Error: Unexpected event received by a machine.\n");
            bProcess = false;
            break;
        }
    }

    /* Process the event, when it is targeted to this machine */
    if (bProcess)
    {
        switch (pSM->state)
        {
        /* Session states */
        case stActivating:
            switch (event)
            {
            case SE_ACTIVATED_SESSION:
                pSM->state = stActivated;
                pSM->iSessionID = arg;
                printf("# Info: Session activated.\n");
                break;
            case SE_SESSION_ACTIVATION_FAILURE:
                pSM->state = stError;
                printf("# Error: Failed session activation.\n");
                break;
            default:
                pSM->state = stError;
                printf("# Error: In state Activation, unexpected event %i.\n", event);
                break;
            }
            break;
        case stActivated:
            switch (event)
            {
            case SE_RCV_SESSION_RESPONSE:
                printf("# Info: Response received.\n");
                break;
            case SE_SND_REQUEST_FAILED:
                pSM->state = stError;
                printf("# Error: Send request 0x%" PRIxPTR " failed.\n", (uintptr_t) pSM->pCtxRequest);
                break;
            default:
                pSM->state = stError;
                printf("# Error: In state stActivated, unexpected event %i.\n", event);
                break;
            }
            break;
        case stClosing:
            switch (event)
            {
            case SE_CLOSED_SESSION:
                pSM->state = stConfigured;
                break;
            default:
                /* This might be a response to a pending request, so this might not an error */
                printf("# Warning: Unexpected event in stClosing state, ignoring.\n");
                break;
            }
            break;
        /* Discovery state */
        case stDiscovering:
        case stRegister:
            switch (event)
            {
            case SE_RCV_DISCOVERY_RESPONSE:
                /* This assert is ok because of the test that would otherwise set bProcess to false */
                assert((uintptr_t) pSM->pCtxRequest == appCtx);
                pSM->state = stConfigured;
                break;
            default:
                printf("# Error: Unexpected event %i in stDiscovering state.\n", event);
                pSM->state = stError;
                break;
            }
            break;
        /* Invalid states */
        case stInit:
            pSM->state = stError;
            printf("# Error: Event received in stInit state.\n");
            break;
        case stConfigured:
            pSM->state = stError;
            printf("# Error: Event received in stConfigured state.\n");
            break;
        case stError:
            printf("# Warning: Receiving event in stError state, ignoring.\n");
            break;
        default:
            pSM->state = stError;
            printf("# Error: Dispatching in unknown state %i, event %i.\n", pSM->state, event);
            break;
        }

        /* Whatever the state, a response with a known pCtxRequest shall free it,
         * and return the appCtx to the caller */
        if (NULL != pSM->pCtxRequest && (uintptr_t) pSM->pCtxRequest == appCtx)
        {
            if (NULL != pAppCtx)
            {
                *pAppCtx = pSM->pCtxRequest->appCtx;
            }
            SOPC_Free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    status = Mutex_Unlock(&pSM->mutex);
    assert(SOPC_STATUS_OK == status);

    return bProcess;
}
