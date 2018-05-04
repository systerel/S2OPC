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
#include <string.h>

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
struct SOPC_StaMac_ReqCtx
{
    uint32_t uid;     /* Unique request identifier */
    uintptr_t appCtx; /* Application context, chosen outside of the state machine */
};

struct SOPC_StaMac_Machine
{
    SOPC_StaMac_State state;
    uint32_t iscConfig;                       /* Toolkit scConfig ID */
    uint32_t iCliId;                          /* LibSub connection ID, used by the callback */
    SOPC_LibSub_DataChangeCbk cbkDataChanged; /* Callback when subscribed data changed */
    uintptr_t iSessionCtx;                    /* Toolkit Session Context, used to identify session events */
    uint32_t iSessionID;                      /* OPC UA Session ID */
    SOPC_SLinkedList* pListReqCtx;            /* List of yet-to-be-answered requests,
                                               * id is unique request identifier, value is a SOPC_StaMac_ReqCtx */
    double fPublishInterval;                  /* The publish interval, in ms */
    uint32_t iSubscriptionID;                 /* OPC UA subscription ID, non 0 when subscription is created */
    SOPC_SLinkedList* pListMonIt;             /* List of monitored items, where the appCtx is the list value,
                                               * and the id is the uint32_t OPC UA monitored item ID */
    uint32_t nTokenTarget;                    /* Target number of available tokens */
    uint32_t nTokenUsable;                    /* Tokens available to the server
                                               * (PublishRequest_sent - PublishResponse_sent) */
    bool bAckSubscr;                          /* Indicates whether an acknowledgement should be sent
                                               * in the next PublishRequest */
    uint32_t iAckSeqNum;                      /* The sequence number to acknowledge after a PublishResponse */
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
                                     uint32_t iCliId,
                                     SOPC_LibSub_DataChangeCbk cbkDataChanged,
                                     double fPublishInterval,
                                     uint32_t iTokenTarget,
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
        pSM->iCliId = iCliId;
        pSM->cbkDataChanged = cbkDataChanged;
        pSM->iSessionCtx = 0;
        pSM->iSessionID = 0;
        pSM->pListReqCtx = SOPC_SLinkedList_Create(0);
        pSM->fPublishInterval = fPublishInterval;
        pSM->iSubscriptionID = 0;
        pSM->pListMonIt = SOPC_SLinkedList_Create(0);
        pSM->nTokenTarget = iTokenTarget;
        pSM->nTokenUsable = 0;
        pSM->bAckSubscr = false;
        pSM->iAckSeqNum = 0;
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

SOPC_ReturnStatus SOPC_StaMac_CreateMonitoredItem(SOPC_StaMac_Machine* pSM,
                                                  const char* szNodeId,
                                                  uint32_t iAttrId,
                                                  uintptr_t* pAppCtx,
                                                  uint32_t* pCliHndl)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NodeId* pNid = NULL;
    size_t szLen = 0;
    void* pReq = NULL;
    uint32_t iCliHndl = 0;

    if (NULL == pSM || NULL == szNodeId || NULL == pCliHndl)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && stActivated != pSM->state)
    {
        status = SOPC_STATUS_INVALID_STATE;
        /* TODO: log */
        printf("# Error: creating monitored item, the machine should be in the stActivated state (is in %i).\n",
               pSM->state);
    }
    if (SOPC_STATUS_OK == status && !SOPC_StaMac_HasSubscription(pSM))
    {
        status = SOPC_STATUS_INVALID_STATE;
        printf("# Error: the machine shall have a created subscription.\n");
    }
    if (SOPC_STATUS_OK == status && UINT32_MAX == nSentReqs)
    {
        status = SOPC_STATUS_INVALID_STATE;
        printf("# Error: creating monitored item, too much sent requests.\n");
    }

    /* Create the NodeId */
    if (SOPC_STATUS_OK == status)
    {
        szLen = strlen(szNodeId);
        if (INT32_MAX < szLen)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
            printf("# Error: creating monitored item, szNodeId is too long.\n");
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pNid = SOPC_NodeId_FromCString(szNodeId, (int32_t) szLen);
        if (NULL == pNid)
        {
            status = SOPC_STATUS_NOK;
            printf("# Error: creating monitored item, could not convert szNodeId to NodeId.\n");
        }
    }

    /* Create the CreateMonitoredItemRequest */
    if (SOPC_STATUS_OK == status)
    {
        ++nSentReqs;
        iCliHndl = nSentReqs;
        status = Helpers_NewCreateMonitoredItemsRequest(pNid, iAttrId, pSM->iSubscriptionID, MONIT_TIMESTAMPS_TO_RETURN,
                                                        iCliHndl, MONIT_QSIZE, &pReq);
    }

    /* Send it */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_SendRequest(pSM, pReq, iCliHndl);
    }

    /* Update the machine, the *pAppCtx, and *pCliHndl */
    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stCreatingMonIt;
        if (NULL != pAppCtx)
        {
            *pAppCtx = iCliHndl;
        }
        *pCliHndl = iCliHndl;
    }

    /* Partial mallocs */
    if (SOPC_STATUS_OK != status && NULL != pNid)
    {
        free(pNid);
        pNid = NULL;
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

bool SOPC_StaMac_HasSubscription(SOPC_StaMac_Machine* pSM)
{
    return NULL != pSM && 0 != pSM->iSubscriptionID;
}

bool SOPC_StaMac_HasMonItByAppCtx(SOPC_StaMac_Machine* pSM, uintptr_t appCtx)
{
    bool bHasMonIt = false;
    SOPC_SLinkedListIterator pIter = NULL;

    if (NULL != pSM || NULL != pSM->pListMonIt)
    {
        pIter = SOPC_SLinkedList_GetIterator(pSM->pListMonIt);
    }

    while (!bHasMonIt && NULL != pIter)
    {
        if (SOPC_SLinkedList_Next(&pIter) == (void*) appCtx)
        {
            bHasMonIt = true;
        }
    }

    return bHasMonIt;
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
    uintptr_t intAppCtx = 0; /* Internal appCtx, the one wrapped in the (ReqCtx*)appCtx */
    void* pRequest = NULL;
    int32_t i = 0;
    OpcUa_CreateMonitoredItemsResponse* pMonItResp = NULL;
    SOPC_EncodeableType* pEncType = NULL;
    OpcUa_PublishResponse* pPubResp = NULL;
    OpcUa_NotificationMessage* pNotifMsg = NULL;
    OpcUa_DataChangeNotification* pDataNotif = NULL;
    OpcUa_MonitoredItemNotification* pMonItNotif = NULL;
    SOPC_LibSub_Value* plsVal = NULL;

    bProcess = StaMac_IsEventTargeted(pSM, &intAppCtx, event, arg, pParam, appCtx);

    if (bProcess)
    {
        if (NULL != pAppCtx)
        {
            *pAppCtx = intAppCtx;
        }

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
                pSM->nTokenUsable = 0;
                pSM->bAckSubscr = false;
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
                /* There should be an EncodeableType pointer in the first field of the message struct */
                pEncType = *(SOPC_EncodeableType**) pParam;
                if (&OpcUa_PublishResponse_EncodeableType == pEncType)
                {
                    printf("# Info: PublishResponse received.\n");
                    /* TODO: move this to an external function */
                    pPubResp = (OpcUa_PublishResponse*) pParam;
                    /* Take note to acknowledge later. There is no ack with KeepAlive. */
                    /* TODO: this limits the benefits of having multiple pending PublishRequest, maybe
                     * it would be more appropriate to have a list of SeqNumbsToAck... */
                    assert(!pSM->bAckSubscr);
                    if (0 < pPubResp->NoOfAvailableSequenceNumbers)
                    {
                        pSM->bAckSubscr = true;
                        /* Only take the last one when more than 1 is available */
                        pSM->iAckSeqNum =
                            pPubResp->AvailableSequenceNumbers[pPubResp->NoOfAvailableSequenceNumbers - 1];
                    }
                    pSM->nTokenUsable -= 1;
                    /* Traverse the notifications and calls the callback */
                    pNotifMsg = &pPubResp->NotificationMessage;
                    /* For now, only handles at most a NotificationData */
                    assert(1 >= pNotifMsg->NoOfNotificationData);
                    if (0 < pNotifMsg->NoOfNotificationData)
                    {
                        assert(&OpcUa_DataChangeNotification_EncodeableType ==
                               pNotifMsg->NotificationData[0].Body.Object.ObjType);
                        pDataNotif = (OpcUa_DataChangeNotification*) pNotifMsg->NotificationData[0].Body.Object.Value;
                        for (i = 0; i < pDataNotif->NoOfMonitoredItems; ++i)
                        {
                            pMonItNotif = &pDataNotif->MonitoredItems[i];
                            status = Helpers_NewValueFromDataValue(&pMonItNotif->Value, &plsVal);
                            pSM->cbkDataChanged(pSM->iCliId, pMonItNotif->ClientHandle, plsVal);
                        }
                    }
                    /* TODO: verify the results[] which contains a status for each Ack */
                }
                else
                {
                    printf("# Info: Response received.\n");
                }
                break;
            case SE_SND_REQUEST_FAILED:
                pSM->state = stError;
                printf("# Error: Send request 0x%" PRIxPTR " failed.\n", intAppCtx);
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
                printf("# Info: Subscription created.\n");
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
            switch (event)
            {
            case SE_RCV_SESSION_RESPONSE:
                /* There should be only one result element */
                pMonItResp = (OpcUa_CreateMonitoredItemsResponse*) pParam;
                assert(NULL != pMonItResp);
                for (i = 0; SOPC_STATUS_OK == status && i < pMonItResp->NoOfResults; ++i)
                {
                    /* TODO: verify revised values?? */
                    if (0 != pMonItResp->Results[i].StatusCode) /* OpcUa_Good does not exist... */
                    {
                        status = SOPC_STATUS_NOK;
                    }
                    else
                    {
                        if (SOPC_SLinkedList_Append(pSM->pListMonIt, pMonItResp->Results[i].MonitoredItemId,
                                                    (void*) intAppCtx) != (void*) intAppCtx)
                        {
                            status = SOPC_STATUS_NOK;
                        }
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        printf("# Info: MonitoredItem created.\n");
                    }
                }
                if (SOPC_STATUS_OK == status)
                {
                    pSM->state = stActivated;
                }
                else
                {
                    pSM->state = stError;
                }
                break;
            case SE_SND_REQUEST_FAILED:
                pSM->state = stError;
                printf("# Error: Send create monitored items request failed.\n");
                break;
            default:
                pSM->state = stError;
                printf("# Error: In state stCreatingMonIt, unexpected event %i.\n", event);
                break;
            }
            break;
        case stCreatingPubReq:
            /* TODO: remove this non existing state */
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
            /* Then add tokens, but wait for at least a monitored item */
            else if (pSM->nTokenUsable < pSM->nTokenTarget)
            {
                while (SOPC_STATUS_OK == status && pSM->nTokenUsable < pSM->nTokenTarget)
                {
                    /* Send a PublishRequest */
                    printf("# Info: Adding publish token.\n");
                    status =
                        Helpers_NewPublishRequest(pSM->bAckSubscr, pSM->iSubscriptionID, pSM->iAckSeqNum, &pRequest);
                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_StaMac_SendRequest(pSM, pRequest, 0);
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        /* This is the reason nTokenUsable and nTokenTarget are uint32_t: uint16_t arithmetics
                         * would raise a warning, as 1 cannot be interpreted as a uint16_t... */
                        pSM->nTokenUsable += 1;
                        pSM->bAckSubscr = false;
                    }
                    else
                    {
                        pSM->state = stError;
                    }
                }
            }
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
    (void) arg;
    (void) pParam;
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
        /* Depending on the event, check either by request ctx or session ctx. */
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
                    if (SOPC_SLinkedList_RemoveFromId(pSM->pListReqCtx, ((SOPC_StaMac_ReqCtx*) appCtx)->uid) == NULL)
                    {
                        printf("# Warning: failed to pop the request from the pListReqCtx\n");
                    }
                    free((void*) appCtx);
                    appCtx = 0;
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
