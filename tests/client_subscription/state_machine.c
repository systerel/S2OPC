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

#include <stdio.h>
#include <stdlib.h>

#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_user_app_itf.h"

#include "state_machine.h"

static uint32_t nSentReqs = 0; /* Number of sent request, used to uniquely associate a response to its request. */

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
    SOPC_SLinkedList* pListCtxReqs;           /* List of yet-to-be-answered requests,
                                               * id is unique request identifier, value is a SOPC_StaMac_ReqCtx */
    double fPublishInterval;                  /* The publish interval, in ms */
    uint32_t iSubscriptionID;                 /* OPC UA subscription ID, non 0 when subscription is created */
    SOPC_SLinkedList* pListMonIt;             /* List of monitored items, where the appCtx is the list id,
                                               * and the value is the uint32_t OPC UA monitored item ID */
};

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
        pSM->pListCtxReqs = SOPC_SLinkedList_Create(0);
        pSM->fPublishInterval = fPublishInterval;
        pSM->iSubscriptionID = 0;
        pSM->pListMonIt = SOPC_SLinkedList_Create(0);
    }

    if (SOPC_STATUS_OK == status && (NULL == pSM->pListCtxReqs || NULL == pSM->pListMonIt))
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

SOPC_ReturnStatus SOPC_StaMac_StartSession(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_ReqCtx* pReqCtx = NULL;

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
        pReqCtx->appCtx = 0;
        if (SOPC_SLinkedList_Append(pSM->pListCtxReqs, pReqCtx->uid, (void*) pReqCtx) != pReqCtx)
        {
            status = SOPC_STATUS_NOK;
            free(pReqCtx);
        }
    }

    /* Sends the request */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncActivateSession(pSM->iscConfig, (uintptr_t) pReqCtx);
        pSM->state = stActivating;
    }

    if (SOPC_STATUS_OK != status && NULL != pSM)
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
        if (SOPC_SLinkedList_Append(pSM->pListCtxReqs, pReqCtx->uid, (void*) pReqCtx) != pReqCtx)
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
    return false;
}
