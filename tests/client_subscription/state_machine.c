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

/** \file
 *
 * \brief The state machine of the subscribing client. See state_machine.h
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_user_app_itf.h"

#include "state_machine.h"
#include "toolkit_helpers.h"

/* =========
 * Internals
 * =========
 */

/* Structures */
enum SOPC_StaMac_State
{
    stError,
    stInit,
    stActivating,
    stActivated,
    stCreatingSubscr,
    stCreatingMonIt,
    stCreatingPubReq,
    stClosing
};

struct SOPC_StaMac_ReqCtx
{
    uint32_t uid;     /* Unique request identifier */
    uintptr_t appCtx; /* Application context, chosen outside of the state machine */
};

struct SOPC_StaMac_Machine
{
    SOPC_StaMac_State state;
    uint32_t iscConfig;                       /* Toolkit scConfig ID */
    SOPC_LibSub_DataChangeCbk cbkDataChanged; /* Callback when subscribed data changed */
    uintptr_t iSessionCtx;                    /* Toolkit Session Context, used to identify session events */
    uint32_t iSessionID;                      /* OPC UA Session ID */
    SOPC_SLinkedList* pListReqCtx;            /* List of yet-to-be-answered requests,
                                               * id is unique request identifier, value is a SOPC_StaMac_ReqCtx */
    double fPublishInterval;                  /* The publish interval, in ms */
    uint32_t iSubscriptionID;                 /* OPC UA subscription ID, non 0 when subscription is created */
    SOPC_SLinkedList* pListMonIt;             /* List of monitored items, where the appCtx is the list id,
                                               * and the value is the uint32_t OPC UA monitored item ID */
};

/* Global variables */
static uint32_t nSentReqs = 0; /* Number of sent request, used to uniquely associate a response to its request. */

/* Internal functions */
bool StaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                            uintptr_t* pAppCtx,
                            SOPC_App_Com_Event event,
                            uint32_t arg,
                            void* pParam,
                            uintptr_t appCtx);

/* ==================
 * API implementation
 * ==================
 */

SOPC_ReturnStatus SOPC_StaMac_Create(uint32_t iscConfig,
                                     SOPC_LibSub_DataChangeCbk cbkDataChanged,
                                     double fPublishInterval,
                                     SOPC_StaMac_Machine** ppSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    pSM = malloc(sizeof(SOPC_StaMac_Machine));
    if (NULL == pSM)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stInit;
        pSM->iscConfig = iscConfig;
        pSM->cbkDataChanged = cbkDataChanged;
        pSM->iSessionCtx = 0;
        pSM->iSessionID = 0;
        pSM->pListReqCtx = SOPC_SLinkedList_Create(0);
        pSM->fPublishInterval = fPublishInterval;
        pSM->iSubscriptionID = 0;
        pSM->pListMonIt = SOPC_SLinkedList_Create(0);
    }

    if (SOPC_STATUS_OK == status && (NULL == pSM->pListReqCtx || NULL == pSM->pListMonIt))
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Handles the value to the caller */
    if (SOPC_STATUS_OK == status)
    {
        *ppSM = pSM;
    }

    return status;
}

void SOPC_StaMac_Delete(SOPC_StaMac_Machine** ppSM)
{
    SOPC_StaMac_Machine* pSM = NULL;

    if (NULL != ppSM && NULL != *ppSM)
    {
        pSM = *ppSM;
        if (NULL != pSM->pListReqCtx)
        {
            SOPC_SLinkedList_Delete(pSM->pListReqCtx);
            pSM->pListReqCtx = NULL;
        }
        if (NULL != pSM->pListMonIt)
        {
            SOPC_SLinkedList_Delete(pSM->pListMonIt);
            pSM->pListMonIt = NULL;
        }
        free(pSM);
        *ppSM = NULL;
    }
}

SOPC_ReturnStatus SOPC_StaMac_StartSession(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && UINT32_MAX == nSentReqs)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status && pSM->state != stInit)
    {
        status = SOPC_STATUS_INVALID_STATE;
        /* TODO: log */
        printf("# Error: The state machine shall be in stInit state to start a session.\n");
    }

    /* Sends the request */
    if (SOPC_STATUS_OK == status)
    {
        ++nSentReqs;
        pSM->iSessionCtx = nSentReqs; /* Record the session context, must be reset when connection is closed. */
        SOPC_ToolkitClient_AsyncActivateSession(pSM->iscConfig, (uintptr_t) pSM->iSessionCtx);
        pSM->state = stActivating;
    }
    else if (NULL != pSM)
    {
        pSM->state = stError;
    }

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_StopSession(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!SOPC_StaMac_IsConnected(pSM))
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

SOPC_ReturnStatus SOPC_StaMac_SendRequest(SOPC_StaMac_Machine* pSM, void* requestStruct, uintptr_t appCtx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_ReqCtx* pReqCtx = NULL;

    if (NULL == pSM || (!SOPC_StaMac_IsConnected(pSM) && stClosing != pSM->state) || UINT32_MAX == nSentReqs)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Allocate a request context */
    if (SOPC_STATUS_OK == status)
    {
        pReqCtx = malloc(sizeof(SOPC_StaMac_ReqCtx));
        if (NULL == pReqCtx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Adds it to the list of yet-to-be-answered requests */
    if (SOPC_STATUS_OK == status)
    {
        ++nSentReqs;
        pReqCtx->uid = nSentReqs;
        pReqCtx->appCtx = appCtx;
        /* Asserts that there cannot be two requests with the same id when receiving a response */
        assert(SOPC_SLinkedList_FindFromId(pSM->pListReqCtx, pReqCtx->uid) == NULL);
        if (SOPC_SLinkedList_Append(pSM->pListReqCtx, pReqCtx->uid, (void*) pReqCtx) != pReqCtx)
        {
            status = SOPC_STATUS_NOK;
            free(pReqCtx);
        }
    }

    /* Sends the request */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, requestStruct, (uintptr_t) pReqCtx);
    }

    if (SOPC_STATUS_OK != status && NULL != pSM)
    {
        pSM->state = stError;
    }

    return status;
}

bool SOPC_StaMac_IsConnectable(SOPC_StaMac_Machine* pSM)
{
    return NULL != pSM && stInit == pSM->state;
}

bool SOPC_StaMac_IsConnected(SOPC_StaMac_Machine* pSM)
{
    bool bConnected = false;

    if (NULL != pSM)
    {
        switch (pSM->state)
        {
        case stActivating:
        case stActivated:
        case stCreatingSubscr:
        case stCreatingMonIt:
        case stCreatingPubReq:
        case stClosing:
            bConnected = true;
            break;
        default:
            break;
        }
    }

    return bConnected;
}

bool SOPC_StaMac_IsError(SOPC_StaMac_Machine* pSM)
{
    return NULL != pSM && stError == pSM->state;
}

bool SOPC_StaMac_EventDispatcher(SOPC_StaMac_Machine* pSM,
                                 uintptr_t* pAppCtx,
                                 SOPC_App_Com_Event event,
                                 uint32_t arg,
                                 void* pParam,
                                 uintptr_t appCtx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bProcess = false;
    void* pRequest = NULL;

    bProcess = StaMac_IsEventTargeted(pSM, pAppCtx, event, arg, pParam, appCtx);

    if (bProcess)
    {
        /* Treat event, if needed */
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
        case stClosing:
            switch (event)
            {
            case SE_CLOSED_SESSION:
                pSM->state = stInit;
                /* Reset the machine */
                pSM->iSessionCtx = 0;
                pSM->iSessionID = 0;
                pSM->iSubscriptionID = 0;
                SOPC_SLinkedList_Clear(pSM->pListReqCtx);
                SOPC_SLinkedList_Clear(pSM->pListMonIt);
                break;
            default:
                /* This might be a response to a pending request, so this might not an error */
                printf("# Warning: Unexpected event in stClosing state, ignoring.\n");
                break;
            }
            break;
        /* Main state */
        case stActivated:
            switch (event)
            {
            case SE_RCV_SESSION_RESPONSE:
                printf("# Info: Response received.\n");
                break;
            case SE_SND_REQUEST_FAILED:
                pSM->state = stError;
                printf("# Error: Send request 0x%" PRIxPTR " failed.\n", appCtx);
                break;
            case SE_SESSION_REACTIVATING:
                printf("# Info: Session reactivated.\n");
                break;
            default:
                pSM->state = stError;
                printf("# Error: In state stActivated, unexpected event %i.\n", event);
                break;
            }
            break;
        /* Creating* states */
        case stCreatingSubscr:
            switch (event)
            {
            case SE_RCV_SESSION_RESPONSE:
                /* TODO: verify revised values?? */
                assert(pSM->iSubscriptionID == 0);
                pSM->iSubscriptionID = ((OpcUa_CreateSubscriptionResponse*) pParam)->SubscriptionId;
                printf("# Subscription created.\n");
                pSM->state = stActivated;
                break;
            case SE_SND_REQUEST_FAILED:
                pSM->state = stError;
                printf("# Error: Send subscription request failed.\n");
                break;
            default:
                pSM->state = stError;
                printf("# Error: In state stCreatingSubscr, unexpected event %i.\n", event);
                break;
            }
            break;
        case stCreatingMonIt:
            break;
        case stCreatingPubReq:
            break;
        /* Invalid states */
        case stInit:
            pSM->state = stError;
            printf("# Error: Event received in stInit state.\n");
            break;
        case stError:
            printf("# Warning: Receiving event in stError state, ignoring.\n");
            break;
        default:
            pSM->state = stError;
            printf("# Error: Dispatching in unknown state %i, event %i.\n", pSM->state, event);
            break;
        }

        /* Do other things, now that the event has been processed */
        switch (pSM->state)
        {
        /* Mostly when stActivated is reached */
        case stActivated:
            /* First, create the subscription */
            if (0 == pSM->iSubscriptionID)
            {
                /* Creates the subscription */
                /* The request is freed by the Toolkit */
                /* TODO: make all value configurable */
                /* TODO: log errors */
                printf("# Info: Creating subscription.\n");
                status = Helpers_NewCreateSubscriptionRequest(pSM->fPublishInterval, 1000, 30, &pRequest);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_StaMac_SendRequest(pSM, pRequest, 0);
                }
                if (SOPC_STATUS_OK == status)
                {
                    pSM->state = stCreatingSubscr;
                }
                else
                {
                    pSM->state = stError;
                }
            }
            /* Then add tokens */
            break;
        default:
            break;
        }
    }

    return bProcess;
}

/* ========================
 * Internals implementation
 * ========================
 */

/**
 * \brief Tells whether the event is targeted to this machine.
 *
 * It also unwraps the appCtx if it is a SOPC_StaMac_ReqCtx*, and set *pAppCtx to pReqCtx->appCtx.
 */
bool StaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                            uintptr_t* pAppCtx,
                            SOPC_App_Com_Event event,
                            uint32_t arg,
                            void* pParam,
                            uintptr_t appCtx)
{
    bool bProcess = true;
    SOPC_SLinkedListIterator pListIter = NULL;

    if (NULL == pSM)
    {
        bProcess = false;
    }

    /* As long as the appCtx is not surely a SOPC_StaMac_ReqCtx*, it is not possible to dereference it.
     * But it contains the uid of the request, which is the id of the pListReqCtx.
     * The appCtx is a uintptr_t, so it cannot be set as the id of the SOPC_SLinkedList,
     * and searched for easily. */
    if (bProcess)
    {
        /* Depending on the event, check either by request id or by session id. */
        switch (event)
        {
        /* appCtx is request context */
        case SE_RCV_SESSION_RESPONSE:
        case SE_RCV_DISCOVERY_RESPONSE:
        case SE_SND_REQUEST_FAILED:
            bProcess = false;
            pListIter = SOPC_SLinkedList_GetIterator(pSM->pListReqCtx);
            while (!bProcess && NULL != pListIter)
            {
                if ((uintptr_t) SOPC_SLinkedList_Next(&pListIter) == appCtx)
                {
                    bProcess = true;
                    /* A response with a known pReqCtx shall free it, and return the appCtx to the caller */
                    if (NULL != pAppCtx)
                    {
                        *pAppCtx = ((SOPC_StaMac_ReqCtx*) appCtx)->appCtx;
                    }
                    free((void*) appCtx);
                    if (SOPC_SLinkedList_RemoveFromId(pSM->pListReqCtx, ((SOPC_StaMac_ReqCtx*) appCtx)->uid) != NULL)
                    {
                        printf("# Warning: failed to pop the request from the pListReqCtx\n");
                    }
                }
            }
            break;
        /* appCtx is session context */
        case SE_SESSION_ACTIVATION_FAILURE:
        case SE_ACTIVATED_SESSION:
        case SE_SESSION_REACTIVATING:
        case SE_CLOSED_SESSION:
            if (pSM->iSessionCtx != appCtx)
            {
                bProcess = false;
            }
            break;
        default:
            printf("# Error: Unexpected event received by a machine.\n");
            break;
        }
    }

    return bProcess;
}
