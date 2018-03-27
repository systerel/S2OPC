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
#include <stdio.h>
#include <stdlib.h>

#include "sopc_encodeable.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

static uintptr_t nSMCreated = 0; /* Number of created machines, used for session context. */
static uintptr_t nDiscovery = 0; /* Number of sent discovery requests, used as UID for requestContext */
static uintptr_t nReqSent = 0;   /* Number of other requests sent through the wrapper, used as UID for requestContext */

StateMachine_Machine* StateMachine_Create(void)
{
    StateMachine_Machine* pSM = malloc(sizeof(StateMachine_Machine));
    StateMachine_RequestContext* pCtxSess = malloc(sizeof(StateMachine_RequestContext));

    if (NULL != pSM && NULL != pCtxSess)
    {
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
    }

    return pSM;
}

SOPC_ReturnStatus StateMachine_ConfigureMachine(StateMachine_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Add the SecureChannel configuration */
    if (SOPC_STATUS_OK == status)
    {
        pSM->pscConfig = Config_NewSCConfig(SECURITY_POLICY, SECURITY_MODE);
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
    else
    {
        pSM->state = stError;
    }

    return status;
}

SOPC_ReturnStatus StateMachine_StartSession(StateMachine_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (pSM->state != stConfigured)
    {
        status = SOPC_STATUS_NOK;
        printf("# Error: The state machine shall be in stConfigured state to start a session.\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncActivateSession(pSM->iscConfig, (uintptr_t) pSM->pCtxSession);
        pSM->state = stActivating;
    }
    else
    {
        pSM->state = stError;
    }

    return status;
}

SOPC_ReturnStatus StateMachine_StopSession(StateMachine_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!StateMachine_IsConnected(pSM))
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

    return status;
}

SOPC_ReturnStatus StateMachine_StartDiscovery(StateMachine_Machine* pSM)
{
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
        pSM->pCtxRequest = malloc(sizeof(StateMachine_RequestContext));
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
            free(pReq);
        }
        if (NULL != pSM->pCtxRequest)
        {
            free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    return status;
}

SOPC_ReturnStatus StateMachine_SendRequest(StateMachine_Machine* pSM, void* requestStruct, uintptr_t appCtx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM || NULL != pSM->pCtxSession || stActivated != pSM->state)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->pCtxSession = malloc(sizeof(StateMachine_RequestContext));
        status = NULL == pSM->pCtxSession;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Overflow will not cause a problem, as it shall not be possible to have UINTPTR_MAX pending requests */
        ++nReqSent;
        pSM->pCtxRequest->uid = nReqSent;
        pSM->pCtxRequest->appCtx = appCtx;
        SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, requestStruct, (uintptr_t) pSM->pCtxRequest);
    }

    if (SOPC_STATUS_OK != status && NULL != pSM)
    {
        pSM->state = stError;
        if (NULL != pSM->pCtxRequest)
        {
            free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    return status;
}

bool StateMachine_IsConnectable(StateMachine_Machine* pSM)
{
    return NULL != pSM && stConfigured == pSM->state;
}

bool StateMachine_IsConnected(StateMachine_Machine* pSM)
{
    bool bConnected = false;

    if (NULL != pSM)
    {
        switch (pSM->state)
        {
        case stActivating:
        case stActivated:
        case stClosing:
            bConnected = true;
            break;
        default:
            break;
        }
    }

    return bConnected;
}

bool StateMachine_IsDiscovering(StateMachine_Machine* pSM)
{
    return NULL != pSM && stDiscovering == pSM->state;
}

bool StateMachine_IsIdle(StateMachine_Machine* pSM)
{
    bool bIdle = false;

    if (NULL != pSM)
    {
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
    }

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

    if (NULL != pSM->pCtxSession)
    {
        free(pSM->pCtxSession);
        pSM->pCtxSession = NULL;
    }
    if (NULL != pSM->pCtxRequest)
    {
        free(pSM->pCtxRequest);
        pSM->pCtxRequest = NULL;
    }

    Config_DeleteSCConfig(&((*ppSM)->pscConfig));

    free(*ppSM);
    *ppSM = NULL;
}

bool StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  uintptr_t* pAppCtx,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx)
{
    (void) pParam;
    bool bProcess = true;

    if (NULL == pSM)
    {
        bProcess = false;
    }

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
            free(pSM->pCtxRequest);
            pSM->pCtxRequest = NULL;
        }
    }

    return bProcess;
}
