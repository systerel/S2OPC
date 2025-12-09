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

/** \file
 *
 * \brief The state machine of the subscribing client. See state_machine.h
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_encodeabletype.h"
#include "sopc_hash.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_user_app_itf.h"

#include "libs2opc_client_internal.h"
#include "state_machine.h"

/* =========
 * Internals
 * =========
 */

static const uintptr_t DICT_TOMBSTONE = UINTPTR_MAX;

/* Structures */
struct SOPC_StaMac_Machine
{
    SOPC_Mutex mutex;
    SOPC_StaMac_State state;
    SOPC_StatusCode connectionStatus;
    uint32_t iscConfig; /* Toolkit scConfig ID */
    SOPC_ReverseEndpointConfigIdx
        reverseConfigIdx; /* Reverse configuration index > 0 if reverse connection mechanism shall be used */
    uint32_t iCliId;      /* Connection ID, used by the callback. It shall be unique. */

    SOPC_StaMacNotification_Fct* pCbkNotification; /* Callback when subscription notification occurs */

    SOPC_StaMac_EventCbk* pCbkGenericEvent; /* Callback when received event that is out of the StaMac scope */
    uintptr_t iSessionCtx;                  /* Toolkit Session Context, used to identify session events */
    uint32_t iSessionID;                    /* S2OPC Session ID */
    SOPC_SLinkedList* pListReqCtx;          /* List of yet-to-be-answered requests,
                                             * id is unique request identifier, value is a SOPC_StaMac_ReqCtx */
    SOPC_SLinkedList* pListSubscriptions; /* OPC UA subscription. SubcriptionId (= key) <-> subscriptionCtx (= value) */
    bool bSubscriptionInProgress;         /* Flag active when a subscription is in creation progress */
    bool bSubscriptionInProgressCreated;  /* Flag active when the subscription in progress has been created */
    uint32_t latestSubscriptionId;        /* The id of latest created subscription */
    uint32_t nMonItClientHandle;          /* Latest client handle generated for monitored items,
                                           * used as unique identifier */
    uint32_t nTokenTarget;                /* Target number of available tokens */
    bool tooManyTokenRcvd;                /* Flag set when server returns a too many token service fault,
                                              wait for next response without fault. */
    uint32_t nTokenUsable;                /* Tokens available to the server
                                           * (PublishRequest_sent - PublishResponse_sent) */
    const char* szPolicyId;               /* Zero-terminated user identity policy id */
    const char* szUsername;               /* Zero-terminated username */
    const char* szPassword;               /* Zero-terminated password */
    SOPC_SerializedCertificate*
        pUserCertX509;                      /* X509 serialized certificate for X509IdentiyToken (DER or PEM format) */
    SOPC_SerializedAsymmetricKey* pUserKey; /* Serialized private key for X509IdentiyToken (DER or PEM format) */
    SOPC_SLinkedList* dataIdToNodeIdList;   /* A list of data ids to node ids */
    uintptr_t userContext;                  /* A state machine user defined context */
};

typedef struct
{
    double fPublishInterval;          /* The publish interval, in ms */
    uint32_t iCntMaxKeepAlive;        /* Number of skipped response before sending an empty KeepAlive */
    uint32_t iCntLifetime;            /* Number of deprived publish cycles before subscription deletion */
    uintptr_t subscriptionAppCtx;     /* Application context of the subscription (new API only) */
    bool bAckSubscr;                  /* Indicates whether an acknowledgment should be sent
                                       * in the next PublishRequest */
    SOPC_SLinkedList* pListAckSeqNum; /* List of sequence numbers (uint32_t) to acknowledge after a PublishResponse */
    SOPC_SLinkedList* pListMonIt;     /* List of monitored items creation request successful, where the
                                         SOPC_CreateMonitoredItem_Ctx is the listed value */
    SOPC_SLinkedList* pListDelMonIt;  /* List of monitored items deletion request successful, where the
                                         SOPC_DeleteMonitoredItem_Ctx is the listed value */
    SOPC_Dict* miCliHandleToUserAppCtxDict; /* A dictionary of monitored client handles to user app contexts
                                               (new API only) */
    SOPC_Dict* miIdToCliHandleDict;         /* A dictionary of ids to client handles (new API only) */
} subscriptionCtx;

/* Internal functions */
static bool LockedStaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                                         uintptr_t* pAppCtx,
                                         SOPC_StaMac_RequestScope* pRequestScope,
                                         SOPC_StaMac_RequestType* pRequestType,
                                         SOPC_App_Com_Event event,
                                         uint32_t arg,
                                         void* pParam,
                                         uintptr_t appCtx);

static bool LockedStaMac_GiveAuthorization_stActivating(SOPC_StaMac_Machine* pSM,
                                                        SOPC_App_Com_Event event,
                                                        SOPC_EncodeableType* pEncType);
static bool LockedStaMac_GiveAuthorization_stClosing(SOPC_StaMac_Machine* pSM,
                                                     SOPC_App_Com_Event event,
                                                     SOPC_EncodeableType* pEncType);
static bool LockedStaMac_GiveAuthorization_stActivated(SOPC_StaMac_Machine* pSM,
                                                       SOPC_App_Com_Event event,
                                                       SOPC_EncodeableType* pEncType);
static bool LockedStaMac_GiveAuthorization_stCreatingSubscr(SOPC_StaMac_Machine* pSM,
                                                            SOPC_App_Com_Event event,
                                                            SOPC_EncodeableType* pEncType);
static bool LockedStaMac_GiveAuthorization_stCreatingMonIt(SOPC_StaMac_Machine* pSM,
                                                           SOPC_App_Com_Event event,
                                                           SOPC_EncodeableType* pEncType);
static bool LockedStaMac_GiveAuthorization_stDeletingSubscr(SOPC_StaMac_Machine* pSM,
                                                            SOPC_App_Com_Event event,
                                                            SOPC_EncodeableType* pEncType);

static void LockedStaMac_ProcessMsg_ActivateSessionResponse(SOPC_StaMac_Machine* pSM,
                                                            uint32_t arg,
                                                            void* pParam,
                                                            uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_CloseSessionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_CloseChannelResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_PublishResponse(SOPC_StaMac_Machine* pSM,
                                                    uint32_t arg,
                                                    void* pParam,
                                                    uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_CreateSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                               uint32_t arg,
                                                               void* pParam,
                                                               uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_CreateMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                                 uint32_t arg,
                                                                 void* pParam,
                                                                 uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_DeleteMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                                 uint32_t arg,
                                                                 void* pParam,
                                                                 uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_DeleteSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                               uint32_t arg,
                                                               void* pParam,
                                                               uintptr_t appCtx);
static void LockedStaMac_ProcessMsg_ServiceFault(SOPC_StaMac_Machine* pSM,
                                                 uint32_t arg,
                                                 void* pParam,
                                                 SOPC_StaMac_RequestType reqType);
static void LockedStaMac_ProcessEvent_SendRequestFailed(SOPC_StaMac_Machine* pSM,
                                                        uint32_t arg,
                                                        void* pParam,
                                                        uintptr_t appCtx);
static void LockedStaMac_ProcessEvent_stError(SOPC_StaMac_Machine* pSM,
                                              SOPC_App_Com_Event event,
                                              uint32_t arg,
                                              void* pParam,
                                              uintptr_t appCtx);

static void LockedStaMac_PostProcessActions(SOPC_StaMac_Machine* pSM, SOPC_StaMac_State oldState);

/* ==================
 * API implementation
 * ==================
 */
static uint64_t uintptr_hash(const uintptr_t data)
{
    return SOPC_DJBHash((const uint8_t*) &data, sizeof(uintptr_t));
}

static bool direct_equal(const uintptr_t a, const uintptr_t b)
{
    return a == b;
}

SOPC_ReturnStatus SOPC_StaMac_Create(uint32_t iscConfig,
                                     SOPC_ReverseEndpointConfigIdx reverseConfigIdx,
                                     uint32_t iCliId,
                                     const char* szPolicyId,
                                     const char* szUsername,
                                     const char* szPassword,
                                     const SOPC_SerializedCertificate* pUserCertX509,
                                     const SOPC_SerializedAsymmetricKey* pUserKey,
                                     uint32_t iTokenTarget,
                                     SOPC_StaMac_EventCbk* pCbkGenericEvent,
                                     uintptr_t userContext,
                                     SOPC_StaMac_Machine** ppSM)
{
    SOPC_StaMac_Machine* pSM = SOPC_Calloc(1, sizeof(SOPC_StaMac_Machine));

    if (NULL == pSM)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&pSM->mutex);
    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stInit;
        pSM->connectionStatus = SOPC_GoodGenericStatus;
        pSM->iscConfig = iscConfig;
        pSM->reverseConfigIdx = reverseConfigIdx;
        pSM->iCliId = iCliId;
        pSM->iSessionCtx = 0;
        pSM->iSessionID = 0;
        pSM->pListReqCtx = SOPC_SLinkedList_Create(0);
        pSM->pListSubscriptions = SOPC_SLinkedList_Create(0);
        pSM->bSubscriptionInProgress = false;
        pSM->bSubscriptionInProgressCreated = false;
        pSM->latestSubscriptionId = 0;
        pSM->nMonItClientHandle = 0;
        pSM->nTokenTarget = iTokenTarget;
        pSM->tooManyTokenRcvd = false;
        pSM->nTokenUsable = 0;
        pSM->pCbkGenericEvent = pCbkGenericEvent;
        pSM->szPolicyId = NULL;
        pSM->szUsername = NULL;
        pSM->szPassword = NULL;
        pSM->pUserCertX509 = NULL;
        pSM->pUserKey = NULL;
        pSM->dataIdToNodeIdList = SOPC_SLinkedList_Create(0);

        pSM->userContext = userContext;
        if (NULL != szPolicyId)
        {
            pSM->szPolicyId = SOPC_Malloc(strlen(szPolicyId) + 1);
            if (NULL == pSM->szPolicyId)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (NULL != szUsername)
        {
            pSM->szUsername = SOPC_Malloc(strlen(szUsername) + 1);
            if (NULL == pSM->szUsername)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (NULL != szPassword)
        {
            pSM->szPassword = SOPC_Malloc(strlen(szPassword) + 1);
            if (NULL == pSM->szPassword)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (NULL != pUserCertX509)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(pUserCertX509->data, pUserCertX509->length,
                                                                         &pSM->pUserCertX509);
        }
        if (NULL != pUserKey)
        {
            const SOPC_ExposedBuffer* key = SOPC_SecretBuffer_Expose(pUserKey);
            pSM->pUserKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, SOPC_SecretBuffer_GetLength(pUserKey));
            SOPC_SecretBuffer_Unexpose(key, pUserKey);
        }
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        if (NULL != pSM->szPolicyId && NULL != szPolicyId)
        {
            strcpy((char*) pSM->szPolicyId, szPolicyId);
        }
        if (NULL != pSM->szUsername && NULL != szUsername)
        {
            strcpy((char*) pSM->szUsername, szUsername);
        }
        if (NULL != pSM->szPassword && NULL != szPassword)
        {
            strcpy((char*) pSM->szPassword, szPassword);
        }
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }

    if (SOPC_STATUS_OK == status &&
        (NULL == pSM->pListReqCtx || NULL == pSM->pListSubscriptions || NULL == pSM->dataIdToNodeIdList))
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Handles the value to the caller */
    if (SOPC_STATUS_OK == status)
    {
        *ppSM = pSM;
    }
    else
    {
        SOPC_StaMac_Delete(&pSM);
    }

    return status;
}

static void free_subscriptionCtx(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    subscriptionCtx* subCtx = (subscriptionCtx*) val;
    if (NULL != subCtx)
    {
        SOPC_SLinkedList_Delete(subCtx->pListMonIt);
        subCtx->pListMonIt = NULL;
        SOPC_SLinkedList_Delete(subCtx->pListDelMonIt);
        subCtx->pListDelMonIt = NULL;
        SOPC_SLinkedList_Delete(subCtx->pListAckSeqNum);
        subCtx->pListAckSeqNum = NULL;
        SOPC_Dict_Delete(subCtx->miCliHandleToUserAppCtxDict);
        subCtx->miCliHandleToUserAppCtxDict = NULL;
        SOPC_Dict_Delete(subCtx->miIdToCliHandleDict);
        subCtx->miIdToCliHandleDict = NULL;
        SOPC_Free(subCtx);
    }
}

void SOPC_StaMac_Delete(SOPC_StaMac_Machine** ppSM)
{
    if (NULL != ppSM && NULL != *ppSM)
    {
        SOPC_StaMac_Machine* pSM = *ppSM;
        SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_SLinkedList_Apply(pSM->pListReqCtx, SOPC_SLinkedList_EltGenericFree);
        SOPC_SLinkedList_Delete(pSM->pListReqCtx);
        pSM->pListReqCtx = NULL;
        SOPC_SLinkedList_Apply(pSM->pListSubscriptions, free_subscriptionCtx);
        SOPC_SLinkedList_Delete(pSM->pListSubscriptions);
        pSM->pListSubscriptions = NULL;
        status = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Mutex_Clear(&pSM->mutex);
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_Free((void*) pSM->szPolicyId);
        SOPC_Free((void*) pSM->szUsername);
        SOPC_Free((void*) pSM->szPassword);
        SOPC_SLinkedList_Apply(pSM->dataIdToNodeIdList, SOPC_SLinkedList_EltGenericFree);
        SOPC_SLinkedList_Delete(pSM->dataIdToNodeIdList);
        pSM->dataIdToNodeIdList = NULL;
        SOPC_KeyManager_SerializedCertificate_Delete(pSM->pUserCertX509);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSM->pUserKey);

        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_Free(pSM);
        *ppSM = NULL;
    }
}

SOPC_ReturnStatus SOPC_StaMac_StartSession(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (pSM->state != stInit)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The state machine shall be in stInit state to start a session.");
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Sends the request */
    if (SOPC_STATUS_OK == status)
    {
        // Session is strongly linked to the connection since only 1 can be activated on it
        // and connection ID is globally unique.
        pSM->iSessionCtx = pSM->iCliId;
        SOPC_EndpointConnectionCfg endpointConnectionCfg = {.reverseEndpointConfigIdx = pSM->reverseConfigIdx,
                                                            .secureChannelConfigIdx = pSM->iscConfig};
        if (NULL != pSM->szUsername)
        {
            status = SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
                endpointConnectionCfg, NULL, (uintptr_t) pSM->iSessionCtx, pSM->szPolicyId, pSM->szUsername,
                (const uint8_t*) pSM->szPassword, pSM->szPassword != NULL ? (int32_t) strlen(pSM->szPassword) : 0);
        }
        else if (NULL != pSM->pUserCertX509 && NULL != pSM->pUserKey)
        {
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ToolkitClient_AsyncActivateSession_Certificate(
                    endpointConnectionCfg, NULL, (uintptr_t) pSM->iSessionCtx, pSM->szPolicyId, pSM->pUserCertX509,
                    pSM->pUserKey);
                pSM->pUserKey = NULL; // now owned by toolkit
            }
        }
        else if (NULL == pSM->pUserCertX509 && NULL == pSM->pUserKey)
        {
            status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL,
                                                                       (uintptr_t) pSM->iSessionCtx, pSM->szPolicyId);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Unable to identify the type of token to start a session.");
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stActivating;
    }
    else if (NULL != pSM)
    {
        pSM->state = stError;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_StopSession(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (!SOPC_StaMac_IsConnected(pSM))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "StopSession on a disconnected machine.");
        pSM->state = stError;
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncCloseSession(pSM->iSessionID);
        pSM->state = stClosingSession;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

static void SOPC_StaMacInternal_CloseChannel(SOPC_StaMac_Machine* pSM)
{
    SOPC_EndpointConnectionCfg endpointConnectionCfg = SOPC_EndpointConnectionCfg_CreateClassic(pSM->iscConfig);
    SOPC_ToolkitClient_AsyncCloseChannel(endpointConnectionCfg, (uintptr_t) pSM->iCliId);
    pSM->state = stClosingChannel;
}

void SOPC_StaMac_CloseChannel(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bool isSessionConnected = SOPC_StaMac_IsConnected(pSM);
    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!isSessionConnected)
    {
        SOPC_StaMacInternal_CloseChannel(pSM);
    }
}

void SOPC_StaMac_StopsWithError(SOPC_StaMac_Machine* pSM, SOPC_StatusCode errorStatus)
{
    if (NULL == pSM)
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (pSM->state != stError && SOPC_IsGoodStatus(pSM->connectionStatus))
    {
        pSM->connectionStatus = errorStatus;
    }
    bool isConnected = SOPC_StaMac_IsConnected(pSM);
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    if (isConnected)
    {
        SOPC_StaMac_StopSession(pSM);
    }
    else
    {
        SOPC_StaMac_CloseChannel(pSM);
    }
}

SOPC_StatusCode SOPC_StaMac_GetConnectionStatus(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return OpcUa_BadInvalidState;
    }
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_StatusCode connectionStatus = pSM->connectionStatus;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    return connectionStatus;
}

SOPC_ReturnStatus SOPC_StaMac_SendRequest(SOPC_StaMac_Machine* pSM,
                                          void* requestStruct,
                                          uintptr_t appCtx,
                                          SOPC_StaMac_RequestScope requestScope,
                                          SOPC_StaMac_RequestType requestType)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_ReqCtx* pReqCtx = NULL;

    if (NULL == pSM || !SOPC_StaMac_IsConnected(pSM))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ASSERT(SOPC_REQUEST_SCOPE_STATE_MACHINE == requestScope || SOPC_REQUEST_SCOPE_APPLICATION == requestScope);

    pReqCtx = SOPC_Malloc(sizeof(SOPC_StaMac_ReqCtx));
    if (NULL == pReqCtx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Adds it to the list of yet-to-be-answered requests */
    pReqCtx->pSM = pSM;
    pReqCtx->appCtx = appCtx;
    pReqCtx->requestScope = requestScope;
    pReqCtx->requestType = requestType;
    if ((SOPC_StaMac_ReqCtx*) SOPC_SLinkedList_Append(pSM->pListReqCtx, 0, (uintptr_t) pReqCtx) != pReqCtx)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Sends the request */
    if (SOPC_STATUS_OK == status)
    {
        // We will use pReqCtx address as context to recognize it when received.
        // It is acceptable because we keep it in pSM->pListReqCtx and deallocate it only when removed from list
        SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, requestStruct, (uintptr_t) pReqCtx);
    }

    if (SOPC_STATUS_OK != status)
    {
        pSM->state = stError;
        SOPC_Free(pReqCtx);
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewCreateSubscription(SOPC_StaMac_Machine* pSM,
                                                    OpcUa_CreateSubscriptionRequest* req,
                                                    uintptr_t userAppContext)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool requestSentToServices = false;

    if (NULL == pSM || NULL == req || &OpcUa_CreateSubscriptionRequest_EncodeableType != req->encodeableType)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        if (!pSM->bSubscriptionInProgress && stActivated == pSM->state)
        {
            pSM->bSubscriptionInProgress = true;
            pSM->bSubscriptionInProgressCreated = false;
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Creating subscription using provided request.");
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_StaMac_SendRequest(pSM, req, userAppContext, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                                 SOPC_REQUEST_TYPE_SUBSCRIPTION);
                requestSentToServices = (SOPC_STATUS_OK == status);
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
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (!requestSentToServices && NULL != req)
    {
        SOPC_EncodeableObject_Delete(req->encodeableType, (void**) &req);
    }

    return status;
}

uintptr_t SOPC_StaMac_GetSubscriptionCtx(SOPC_StaMac_Machine* pSM, uint32_t subscriptionId)
{
    uintptr_t result = 0;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
    if (NULL != subCtx)
    {
        result = subCtx->subscriptionAppCtx;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               subscriptionId);
    }
    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return result;
}

SOPC_ReturnStatus SOPC_StaMac_GetSubscriptionRevisedParams(SOPC_StaMac_Machine* pSM,
                                                           uint32_t subscriptionId,
                                                           double* revisedPublishingInterval,
                                                           uint32_t* revisedLifetimeCount,
                                                           uint32_t* revisedMaxKeepAliveCount)
{
    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
    if (NULL != subCtx)
    {
        if (NULL != revisedPublishingInterval)
        {
            *revisedPublishingInterval = subCtx->fPublishInterval;
        }
        if (NULL != revisedLifetimeCount)
        {
            *revisedLifetimeCount = subCtx->iCntLifetime;
        }
        if (NULL != revisedMaxKeepAliveCount)
        {
            *revisedMaxKeepAliveCount = subCtx->iCntMaxKeepAlive;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               subscriptionId);
        status = SOPC_STATUS_INVALID_STATE;
    }
    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_StaMac_SetSubscriptionNbTokens(SOPC_StaMac_Machine* pSM, uint32_t nbTokens)
{
    if (NULL == pSM || 0 == nbTokens)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    pSM->nTokenTarget = nbTokens;
    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus Helpers_NewDeleteSubscriptionRequest(uint32_t subscriptionId, void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_DeleteSubscriptionsRequest* pReq = NULL;

    if (NULL == ppRequest)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Create(&OpcUa_DeleteSubscriptionsRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        uint32_t* pSubscriptionId = (uint32_t*) SOPC_Malloc(sizeof(uint32_t));
        if (NULL == pSubscriptionId)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            *pSubscriptionId = subscriptionId;
            pReq->NoOfSubscriptionIds = 1;
            pReq->SubscriptionIds = pSubscriptionId;
            *ppRequest = (void*) pReq;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_DeleteSubscription(SOPC_StaMac_Machine* pSM, uint32_t subscriptionId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pRequest = NULL;

    if (SOPC_StaMac_HasSubscriptionId(pSM, subscriptionId) && stActivated == pSM->state)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Deleting subscription.");

        if (SOPC_STATUS_OK == status)
        {
            status = Helpers_NewDeleteSubscriptionRequest(subscriptionId, &pRequest);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, pRequest, (uintptr_t) subscriptionId,
                                             SOPC_REQUEST_SCOPE_STATE_MACHINE, SOPC_REQUEST_TYPE_SUBSCRIPTION);
        }
        if (SOPC_STATUS_OK == status)
        {
            pSM->state = stDeletingSubscr;
        }
        else
        {
            pSM->state = stError;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewConfigureNotificationCallback(SOPC_StaMac_Machine* pSM,
                                                               SOPC_StaMacNotification_Fct* pNotificationCb)
{
    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL != pNotificationCb && NULL != pSM->pCbkNotification)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        pSM->pCbkNotification = pNotificationCb;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewCreateMonitoredItems(SOPC_StaMac_Machine* pSM,
                                                      uint32_t subscriptionId,
                                                      OpcUa_CreateMonitoredItemsRequest* req,
                                                      const uintptr_t* userAppCtxArray,
                                                      SOPC_CreateMonitoredItems_Ctx* pAppCtx)
{
    bool requestSentToServices = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    subscriptionCtx* subCtx = NULL;
    if (NULL == pSM || NULL == req || 0 >= req->NoOfItemsToCreate || NULL == pAppCtx)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        subCtx = (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
        if (NULL == subCtx)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                                   subscriptionId);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == status && !SOPC_StaMac_HasSubscriptionId(pSM, subscriptionId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "the machine shall have a created subscription to create monitored items.");
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        uint32_t nElems = (uint32_t) req->NoOfItemsToCreate;

        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (stActivated != pSM->state)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "creating monitored item, the machine should be in the stActivated state (is in %i).", pSM->state);
            status = SOPC_STATUS_INVALID_STATE;
        }

        /* Prepare context to keep a copy of request */
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_EncodeableObject_Create(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pAppCtx->req);
        }

        /* Fill the unique client handle parameters an record the user context associated to the MI  */
        if (SOPC_STATUS_OK == status)
        {
            for (uint32_t i = 0; i < nElems; ++i)
            {
                bool alreadyUsedCliHandle = false;
                uint32_t cpt = 0;
                if (SOPC_STATUS_OK == status && pSM->nMonItClientHandle < UINT32_MAX)
                {
                    do
                    {
                        cpt++;
                        pSM->nMonItClientHandle++;
                        uintptr_t result = SOPC_Dict_Get(subCtx->miCliHandleToUserAppCtxDict,
                                                         (uintptr_t) pSM->nMonItClientHandle, &alreadyUsedCliHandle);
                        SOPC_UNUSED_RESULT(result);
                        if (!alreadyUsedCliHandle && DICT_TOMBSTONE == (uintptr_t) cpt)
                        {
                            alreadyUsedCliHandle = true; // Tombstone value
                        }
                    } while (cpt < UINT32_MAX && alreadyUsedCliHandle); // Ensure that a free handle id is found

                    if (!alreadyUsedCliHandle)
                    {
                        const uintptr_t userCtx = (userAppCtxArray != NULL ? userAppCtxArray[i] : 0);
                        bool result = SOPC_Dict_Insert(subCtx->miCliHandleToUserAppCtxDict,
                                                       (uintptr_t) pSM->nMonItClientHandle, userCtx);
                        if (!result)
                        {
                            status = SOPC_STATUS_OUT_OF_MEMORY;
                        }
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        req->ItemsToCreate[i].RequestedParameters.ClientHandle = pSM->nMonItClientHandle;
                    }
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                pAppCtx->outCtxId = pSM->nMonItClientHandle; // latest handle as context
                req->SubscriptionId = subscriptionId;
                status = SOPC_EncodeableObject_Copy(&OpcUa_CreateMonitoredItemsRequest_EncodeableType,
                                                    (void*) pAppCtx->req, (void*) req);
            }
        }

        /* Send it */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, req, (uintptr_t) pAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                             SOPC_REQUEST_TYPE_SUBSCRIPTION);
            requestSentToServices = (SOPC_STATUS_OK == status);
        }
        else
        {
            SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pAppCtx->req);
        }

        if (SOPC_STATUS_OK == status)
        {
            pSM->state = stCreatingMonIt;
        }

        mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (!requestSentToServices && NULL != req)
    {
        SOPC_EncodeableObject_Delete(req->encodeableType, (void**) &req);
    }

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewDeleteMonitoredItems(SOPC_StaMac_Machine* pSM,
                                                      uint32_t subscriptionId,
                                                      OpcUa_DeleteMonitoredItemsRequest* req,
                                                      SOPC_DeleteMonitoredItems_Ctx* outAppCtx)
{
    bool requestSentToServices = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    subscriptionCtx* subCtx = NULL;

    if (NULL == pSM || NULL == req || 0 >= req->NoOfMonitoredItemIds || NULL == outAppCtx)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        subCtx = (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
        if (NULL == subCtx)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                                   subscriptionId);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == status && !SOPC_StaMac_HasSubscriptionId(pSM, subscriptionId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "the machine shall have a created subscription to create monitored items.");
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (stActivated != pSM->state)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "deleting monitored item, the machine should be in the stActivated state (is in %i).", pSM->state);
            status = SOPC_STATUS_INVALID_STATE;
        }

        /* Prepare context to keep a copy of request */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Create(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
                                                  (void**) &outAppCtx->req);
        }

        uintptr_t clientHandle = 0;
        bool findClientHandle = false;
        if (SOPC_STATUS_OK == status)
        {
            for (uint32_t i = 0; !findClientHandle && i < (uint32_t) req->NoOfMonitoredItemIds; i++)
            {
                clientHandle =
                    SOPC_Dict_Get(subCtx->miIdToCliHandleDict, (uintptr_t) req->MonitoredItemIds[i], &findClientHandle);
            }
            if (!findClientHandle)
            {
                // No valid monitored item id found
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }

        /* Fill the subscription id and copy the request */
        if (SOPC_STATUS_OK == status)
        {
            outAppCtx->outCtxId = clientHandle; // first valid client handle
            req->SubscriptionId = subscriptionId;
            status = SOPC_EncodeableObject_Copy(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
                                                (void*) outAppCtx->req, (void*) req);
        }

        /* Send it */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, req, (uintptr_t) outAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                             SOPC_REQUEST_TYPE_SUBSCRIPTION);
            requestSentToServices = (SOPC_STATUS_OK == status);
        }
        else
        {
            SOPC_EncodeableObject_Delete(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &outAppCtx->req);
        }

        if (SOPC_STATUS_OK == status)
        {
            pSM->state = stDeletingMonIt;
        }

        mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (!requestSentToServices && NULL != req)
    {
        SOPC_EncodeableObject_Delete(req->encodeableType, (void**) &req);
    }

    return status;
}

bool SOPC_StaMac_IsConnectable(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool return_code = (stInit == pSM->state);
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

bool SOPC_StaMac_IsConnected(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    bool bConnected = false;
    switch (pSM->state)
    {
    case stActivated:
    case stCreatingSubscr:
    case stCreatingMonIt:
    case stDeletingSubscr:
    case stClosingSession:
    case stClosingChannel:
        bConnected = true;
        break;
    default:
        break;
    }

    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return bConnected;
}

bool SOPC_StaMac_IsError(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool return_code = (stError == pSM->state);
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

bool SOPC_StaMac_IsSubscriptionInProgress(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool return_code = pSM->bSubscriptionInProgress;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

bool SOPC_StaMac_IsSubscriptionInProgressCreated(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool return_code = pSM->bSubscriptionInProgressCreated;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

uint32_t SOPC_StaMac_GetLatestSubscriptionId(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return 0;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    uint32_t lastSubId = pSM->latestSubscriptionId;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return lastSubId;
}

bool SOPC_StaMac_HasSubscriptionId(SOPC_StaMac_Machine* pSM, uint32_t subscriptionId)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return (NULL != subCtx ? true : false);
}

bool SOPC_StaMac_HasAnySubscription(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    return (0 < SOPC_SLinkedList_GetLength(pSM->pListSubscriptions));
}

bool SOPC_StaMac_PopMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_CreateMonitoredItems_Ctx* pAppCtx)
{
    if (NULL == pSM || NULL == pSM->pListSubscriptions)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool bHasMonIt = false;
    SOPC_SLinkedListIterator pIterSubs = SOPC_SLinkedList_GetIterator(pSM->pListSubscriptions);
    while (!bHasMonIt && NULL != pIterSubs)
    {
        subscriptionCtx* subCtx = (subscriptionCtx*) SOPC_SLinkedList_Next(&pIterSubs);
        if (NULL != subCtx)
        {
            SOPC_SLinkedListIterator pIterMIs = SOPC_SLinkedList_GetIterator(subCtx->pListMonIt);
            while (!bHasMonIt && NULL != pIterMIs)
            {
                if (SOPC_SLinkedList_Next(&pIterMIs) == pAppCtx->outCtxId)
                {
                    bHasMonIt = true;
                }
            }
            if (bHasMonIt)
            {
                // Remove context from list since retrieved by client
                SOPC_SLinkedList_RemoveFromValuePtr(subCtx->pListMonIt, pAppCtx->outCtxId);
            }
        }
    }

    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    return bHasMonIt;
}

bool SOPC_StaMac_PopDeleteMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_DeleteMonitoredItems_Ctx* pAppCtx)
{
    if (NULL == pSM || NULL == pSM->pListSubscriptions)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool bHasMonIt = false;
    SOPC_SLinkedListIterator pIterSubs = SOPC_SLinkedList_GetIterator(pSM->pListSubscriptions);
    while (!bHasMonIt && NULL != pIterSubs)
    {
        subscriptionCtx* subCtx = (subscriptionCtx*) SOPC_SLinkedList_Next(&pIterSubs);
        if (NULL != subCtx)
        {
            SOPC_SLinkedListIterator pIterMIs = SOPC_SLinkedList_GetIterator(subCtx->pListDelMonIt);
            while (!bHasMonIt && NULL != pIterMIs)
            {
                if (SOPC_SLinkedList_Next(&pIterMIs) == pAppCtx->outCtxId)
                {
                    bHasMonIt = true;
                }
            }
            if (bHasMonIt)
            {
                // Remove context from list since retrieved by client
                SOPC_SLinkedList_RemoveFromValuePtr(subCtx->pListDelMonIt, pAppCtx->outCtxId);
            }
        }
    }

    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    return bHasMonIt;
}

uintptr_t SOPC_StaMac_GetUserContext(SOPC_StaMac_Machine* pSM)
{
    SOPC_ASSERT(NULL != pSM);
    return pSM->userContext;
}

void SOPC_StaMac_SetUserContext(SOPC_StaMac_Machine* pSM, uintptr_t userContext)
{
    SOPC_ASSERT(NULL != pSM);
    pSM->userContext = userContext;
}

static bool LockedStaMac_GiveAuthorization_stActivating(SOPC_StaMac_Machine* pSM,
                                                        SOPC_App_Com_Event event,
                                                        SOPC_EncodeableType* pEncType)
{
    bool authorization = false;
    SOPC_UNUSED_ARG(pSM);
    SOPC_UNUSED_ARG(pEncType);

    switch (event)
    {
    case SE_ACTIVATED_SESSION:
        authorization = true;
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stClosing(SOPC_StaMac_Machine* pSM,
                                                     SOPC_App_Com_Event event,
                                                     SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);
    SOPC_UNUSED_ARG(pEncType);

    switch (event)
    {
    case SE_CLOSED_SESSION:
        authorization = stClosingSession == pSM->state;
        break;
    case SE_CLOSED_CHANNEL:
        authorization = stClosingChannel == pSM->state;
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stActivated(SOPC_StaMac_Machine* pSM,
                                                       SOPC_App_Com_Event event,
                                                       SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_RCV_SESSION_RESPONSE:
        if (&OpcUa_PublishResponse_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stCreatingSubscr(SOPC_StaMac_Machine* pSM,
                                                            SOPC_App_Com_Event event,
                                                            SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_RCV_SESSION_RESPONSE:
        if (&OpcUa_CreateSubscriptionResponse_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stCreatingMonIt(SOPC_StaMac_Machine* pSM,
                                                           SOPC_App_Com_Event event,
                                                           SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_RCV_SESSION_RESPONSE:
        if (&OpcUa_CreateMonitoredItemsResponse_EncodeableType == pEncType ||
            &OpcUa_PublishResponse_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stDeletingMonIt(SOPC_StaMac_Machine* pSM,
                                                           SOPC_App_Com_Event event,
                                                           SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_RCV_SESSION_RESPONSE:
        if (&OpcUa_DeleteMonitoredItemsResponse_EncodeableType == pEncType ||
            &OpcUa_PublishResponse_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static bool LockedStaMac_GiveAuthorization_stDeletingSubscr(SOPC_StaMac_Machine* pSM,
                                                            SOPC_App_Com_Event event,
                                                            SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_RCV_SESSION_RESPONSE:
        if (&OpcUa_DeleteSubscriptionsResponse_EncodeableType == pEncType ||
            &OpcUa_PublishResponse_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static bool StaMac_GiveAuthorization_SendRequestFailed(SOPC_StaMac_Machine* pSM,
                                                       SOPC_App_Com_Event event,
                                                       SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_SND_REQUEST_FAILED:
        // We only treat a send request failed event if it concerns a publish request
        if (&OpcUa_PublishRequest_EncodeableType == pEncType)
        {
            authorization = true;
        }
        break;
    default:
        break;
    }

    return authorization;
}

static const char* SOPC_ClientAppComEvent_ToString(SOPC_App_Com_Event event)
{
    switch (event)
    {
    case SE_REVERSE_ENDPOINT_CLOSED:
        return "SE_REVERSE_ENDPOINT_CLOSED";
    case SE_SESSION_ACTIVATION_FAILURE:
        return "SE_SESSION_ACTIVATION_FAILURE";
    case SE_ACTIVATED_SESSION:
        return "SE_ACTIVATED_SESSION";
    case SE_SESSION_REACTIVATING:
        return "SE_SESSION_REACTIVATING";
    case SE_RCV_SESSION_RESPONSE:
        return "SE_RCV_SESSION_RESPONSE";
    case SE_CLOSED_SESSION:
        return "SE_CLOSED_SESSION";
    case SE_CLOSED_CHANNEL:
        return "SE_CLOSED_CHANNEL";
    case SE_RCV_DISCOVERY_RESPONSE:
        return "SE_RCV_DISCOVERY_RESPONSE";
    case SE_SND_REQUEST_FAILED:
        return "SE_SND_REQUEST_FAILED";
    default:
        return "UNKNOWN EVENT VALUE";
    }
}

static const char* SOPC_StaMacState_ToString(SOPC_StaMac_State state)
{
    switch (state)
    {
    case stError:
        return "stError";
    case stInit:
        return "stInit";
    case stActivating:
        return "stActivating";
    case stActivated:
        return "stActivated";
    case stCreatingSubscr:
        return "stCreatingSubscr";
    case stCreatingMonIt:
        return "stCreatingMonIt";
    case stDeletingMonIt:
        return "stDeletingMonIt";
    case stDeletingSubscr:
        return "stDeletingSubscr";
    case stClosingSession:
        return "stClosingSession";
    case stClosingChannel:
        return "stClosingChannel";
    default:
        return "UNKNOWN STATE VALUE";
    }
}

bool SOPC_StaMac_EventDispatcher(SOPC_StaMac_Machine* pSM,
                                 uintptr_t* pAppCtx,
                                 SOPC_App_Com_Event event,
                                 uint32_t arg,
                                 void* pParam,
                                 uintptr_t toolkitCtx)
{
    bool bProcess = false;
    uintptr_t appCtx = 0; /* Internal appCtx, the one wrapped in the (ReqCtx*)toolkitCtx */
    SOPC_StaMac_State oldState = stError;
    SOPC_StaMac_RequestScope requestScope = SOPC_REQUEST_SCOPE_STATE_MACHINE;
    SOPC_StaMac_RequestType requestType = SOPC_REQUEST_TYPE_UNKNOWN;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bool unlockedMutex = false;
    bProcess = LockedStaMac_IsEventTargeted(pSM, &appCtx, &requestScope, &requestType, event, arg, pParam, toolkitCtx);

    if (bProcess)
    {
        oldState = pSM->state;
        if (NULL != pAppCtx)
        {
            *pAppCtx = appCtx;
        }

        /* Treat an event depending on the state of the machine */
        if (SOPC_REQUEST_SCOPE_STATE_MACHINE == requestScope)
        {
            bool processingAuthorization = false;
            /* Get message type */
            SOPC_EncodeableType* pEncType = NULL;

            /* cast only when pParam is a message */
            if (NULL != pParam && (SE_RCV_SESSION_RESPONSE == event || SE_RCV_DISCOVERY_RESPONSE == event ||
                                   SE_SND_REQUEST_FAILED == event || SE_LOCAL_SERVICE_RESPONSE == event))
            {
                if (SE_SND_REQUEST_FAILED == event)
                {
                    pEncType = (SOPC_EncodeableType*) pParam;
                }
                else
                {
                    pEncType = *(SOPC_EncodeableType**) pParam;
                }
            }
            /* Give authorization according to state (switch)*/
            switch (pSM->state)
            {
            /* Session states */
            case stActivating:
                processingAuthorization = LockedStaMac_GiveAuthorization_stActivating(pSM, event, pEncType);
                if (!processingAuthorization && SE_SESSION_ACTIVATION_FAILURE == event)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Session activation failed with status code 0x%08" PRIX32,
                                           (SOPC_StatusCode)(uintptr_t) pParam);
                }
                break;
            case stClosingSession:
            case stClosingChannel:
                processingAuthorization = LockedStaMac_GiveAuthorization_stClosing(pSM, event, pEncType);
                break;
            /* Main state */
            case stActivated:
                processingAuthorization = LockedStaMac_GiveAuthorization_stActivated(pSM, event, pEncType);
                break;
            /* Creating* states */
            case stCreatingSubscr:
                processingAuthorization = LockedStaMac_GiveAuthorization_stCreatingSubscr(pSM, event, pEncType);
                break;
            case stCreatingMonIt:
                processingAuthorization = LockedStaMac_GiveAuthorization_stCreatingMonIt(pSM, event, pEncType);
                break;
            /* Deleting states */
            case stDeletingMonIt:
                processingAuthorization = LockedStaMac_GiveAuthorization_stDeletingMonIt(pSM, event, pEncType);
                break;
            case stDeletingSubscr:
                processingAuthorization = LockedStaMac_GiveAuthorization_stDeletingSubscr(pSM, event, pEncType);
                break;
            /* Invalid states */
            case stError:
                LockedStaMac_ProcessEvent_stError(pSM, event, arg, pParam, appCtx);
                processingAuthorization = false;
                break;
            case stInit:
            default:
                processingAuthorization = false;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Dispatching in unknown state %s, event %s.",
                                       SOPC_StaMacState_ToString(pSM->state), SOPC_ClientAppComEvent_ToString(event));
                break;
            }

            /* always authorize processing of service faults */
            if (&OpcUa_ServiceFault_EncodeableType == pEncType)
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Received ServiceFault");
                processingAuthorization = true;
            }

            /* Authorize processing of send request failed if it is a timeout of PublishRequest */
            if (SE_SND_REQUEST_FAILED == event)
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Dispatching event SE_SND_REQUEST_FAILED");
                processingAuthorization = StaMac_GiveAuthorization_SendRequestFailed(pSM, event, pEncType);
            }

            /* Process message if authorization has been given, else go to stError */
            if (processingAuthorization)
            {
                if (SE_ACTIVATED_SESSION == event)
                {
                    LockedStaMac_ProcessMsg_ActivateSessionResponse(pSM, arg, pParam, appCtx);
                }
                else if (SE_CLOSED_SESSION == event)
                {
                    LockedStaMac_ProcessMsg_CloseSessionResponse(pSM, arg, pParam, appCtx);
                }
                else if (SE_CLOSED_CHANNEL == event)
                {
                    LockedStaMac_ProcessMsg_CloseChannelResponse(pSM, arg, pParam, appCtx);
                }
                else if (SE_RCV_SESSION_RESPONSE == event)
                {
                    if (&OpcUa_PublishResponse_EncodeableType == pEncType)
                    {
                        LockedStaMac_ProcessMsg_PublishResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_CreateMonitoredItemsResponse_EncodeableType == pEncType)
                    {
                        LockedStaMac_ProcessMsg_CreateMonitoredItemsResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_DeleteMonitoredItemsResponse_EncodeableType == pEncType)
                    {
                        LockedStaMac_ProcessMsg_DeleteMonitoredItemsResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_CreateSubscriptionResponse_EncodeableType == pEncType)
                    {
                        LockedStaMac_ProcessMsg_CreateSubscriptionResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_DeleteSubscriptionsResponse_EncodeableType == pEncType)
                    {
                        LockedStaMac_ProcessMsg_DeleteSubscriptionResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_ServiceFault_EncodeableType == pEncType)
                    {
                        /* give appCtx, and not internal app context, to know more about the service fault */
                        LockedStaMac_ProcessMsg_ServiceFault(pSM, arg, pParam, requestType);
                    }
                    else
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Received unknown message in event %s",
                                               SOPC_ClientAppComEvent_ToString(event));
                        pSM->state = stError;
                    }
                }
                else if (SE_SND_REQUEST_FAILED == event)
                {
                    // Use same processing as service fault: it concerns only publish request
                    LockedStaMac_ProcessEvent_SendRequestFailed(pSM, arg, pParam, appCtx);
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Received unexpected event %d", event);
                    pSM->state = stError;
                }
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Received unexpected message or event '%s' in state '%s', switching to error state",
                    SOPC_ClientAppComEvent_ToString(event), SOPC_StaMacState_ToString(pSM->state));
                pSM->state = stError;
            }
        }
        /* Forward the event to the generic event callback if it is not an error */
        else
        {
            SOPC_ASSERT(SOPC_REQUEST_SCOPE_APPLICATION == requestScope);
            uint32_t cliId = pSM->iCliId;
            if (SE_SND_REQUEST_FAILED == event)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "Applicative message could not be sent, closing the connection.");

                if (NULL != pSM->pCbkGenericEvent)
                {
                    // Release mutex to avoid possible deadlock in user callback
                    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
                    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                    unlockedMutex = true;
                    (*pSM->pCbkGenericEvent)(cliId, SOPC_StaMac_ApplicativeEvent_SendFailed, arg, NULL, appCtx);
                }
            }
            else
            {
                if (NULL != pSM->pCbkGenericEvent)
                {
                    // Release mutex to avoid possible deadlock in user callback
                    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
                    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                    unlockedMutex = true;
                    (*pSM->pCbkGenericEvent)(cliId, SOPC_StaMac_ApplicativeEvent_Response, SOPC_STATUS_OK, pParam,
                                             appCtx);
                }
            }
        }

        LockedStaMac_PostProcessActions(pSM, oldState);
        // Note: bProcess now only reflects if the connection callback in client layer will be called or not,
        //       i.e. in case of connection or disconnection events.
        //       (LockedStaMac_IsEventTargeted already filtered unexpected events)
        bProcess = (event == SE_ACTIVATED_SESSION || event == SE_CLOSED_CHANNEL);
    }

    if (!unlockedMutex)
    {
        mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    return bProcess; // true if it shall be processed by client layer
}

/* ========================
 * Internals implementation
 * ========================
 */

/**
 * \brief Tells whether the event is targeted to this machine.
 *
 * It also unwraps the appCtx if it is a SOPC_StaMac_ReqCtx*, and set *pAppCtx to pReqCtx->appCtx.
 * Machine's mutex shall already be locked by the caller.
 *
 * When returns true, the event is targeted to this machine, the pAppCtx is set if not NULL,
 * as well as the request scope.
 */
static bool LockedStaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                                         uintptr_t* pAppCtx,
                                         SOPC_StaMac_RequestScope* pRequestScope,
                                         SOPC_StaMac_RequestType* pRequestType,
                                         SOPC_App_Com_Event event,
                                         uint32_t arg,
                                         void* pParam,
                                         uintptr_t toolkitCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(pParam);

    bool bProcess = true;
    SOPC_StaMac_ReqCtx* reqCtx = NULL;

    if (NULL == pSM)
    {
        return false;
    }

    /* As long as the appCtx is not surely a SOPC_StaMac_ReqCtx*, it is not possible to dereference it.
     * The appCtx is a uintptr_t, so it cannot be set as the id of the SOPC_SLinkedList,
     * and searched for easily. */

    /* Depending on the event, check either by request ctx or session ctx. */
    switch (event)
    {
    /* appCtx is request context */
    case SE_RCV_SESSION_RESPONSE:
    case SE_RCV_DISCOVERY_RESPONSE:
    case SE_SND_REQUEST_FAILED:
        bProcess = false;
        // We ensure context address pointer comparison is correct
        // (memory has not been reallocated since we enqueued in pSM->pListReqCtx)
        // by removing from pSM->pListReqCtx and deallocating memory at same time.
        reqCtx = (void*) SOPC_SLinkedList_RemoveFromValuePtr(pSM->pListReqCtx, toolkitCtx);
        if (NULL != reqCtx)
        {
            bProcess = true;
            /* A response with a known pReqCtx shall free it, and return the appCtx to the caller */
            if (NULL != pAppCtx)
            {
                *pAppCtx = reqCtx->appCtx;
            }
            if (NULL != pRequestScope)
            {
                *pRequestScope = reqCtx->requestScope;
            }
            if (NULL != pRequestType)
            {
                *pRequestType = reqCtx->requestType;
            }
            SOPC_Free(reqCtx);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to pop the request from the pListReqCtx.");
        }
        break;
    /* appCtx is session context */
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_ACTIVATED_SESSION:
    case SE_SESSION_REACTIVATING:
    case SE_CLOSED_SESSION:
        if (pSM->iSessionCtx != toolkitCtx)
        {
            bProcess = false;
        }
        else
        {
            // Update connection error status if not already set
            if (SOPC_IsGoodStatus(pSM->connectionStatus))
            {
                if (SE_SESSION_ACTIVATION_FAILURE == event)
                {
                    pSM->connectionStatus = (SOPC_StatusCode)(uintptr_t) pParam;
                }
                else if (SE_SESSION_REACTIVATING == event)
                {
                    pSM->connectionStatus = OpcUa_BadWouldBlock;
                }
                else if (SE_CLOSED_SESSION == event)
                {
                    pSM->connectionStatus = (SOPC_StatusCode)(uintptr_t) pParam;
                }
            }
        }
        break;
    case SE_CLOSED_CHANNEL:
        if (pSM->iCliId != toolkitCtx)
        {
            bProcess = false;
        }
        break;
    case SE_REVERSE_ENDPOINT_CLOSED:
        // Not managed in state machine
        bProcess = false;
        break;
    default:
        bProcess = false;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Unexpected event received by a machine.");
        break;
    }

    return bProcess;
}

static void LockedStaMac_ProcessMsg_ActivateSessionResponse(SOPC_StaMac_Machine* pSM,
                                                            uint32_t arg,
                                                            void* pParam,
                                                            uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(pParam);
    SOPC_UNUSED_ARG(appCtx);

    pSM->state = stActivated;
    pSM->iSessionID = arg;
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Session activated.");
}

static void LockedStaMac_ProcessMsg_CloseSessionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(pParam);
    SOPC_UNUSED_ARG(appCtx);

    /* Change machine state to closing channel */
    pSM->state = stClosingChannel;
}

static void LockedStaMac_ProcessMsg_CloseChannelResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(pParam);
    SOPC_UNUSED_ARG(appCtx);

    /* Put the machine in a correct closed state, events may still be received */
    pSM->state = stError;
}

static void LockedStaMac_ProcessMsg_PubResp_NotifData(SOPC_StaMac_Machine* pSM,
                                                      OpcUa_PublishResponse* pPubResp,
                                                      OpcUa_DataChangeNotification* pDataNotif)
{
    uintptr_t* newAPImonitoredItemCtxArray = NULL;
    OpcUa_MonitoredItemNotification* pMonItNotif = NULL;
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pPubResp->SubscriptionId);
    if (NULL == subCtx)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               pPubResp->SubscriptionId);
        return;
    }

    if (NULL != pSM->pCbkNotification && pDataNotif->NoOfMonitoredItems > 0)
    {
        newAPImonitoredItemCtxArray = SOPC_Calloc((size_t) pDataNotif->NoOfMonitoredItems, sizeof(uintptr_t));
    }
    for (int32_t i = 0; i < pDataNotif->NoOfMonitoredItems; ++i)
    {
        pMonItNotif = &pDataNotif->MonitoredItems[i];
        if (NULL != pSM->pCbkNotification) // new API behavior
        {
            // Retrieve user context associated to each MI and set it in dedicated array (same index as MI)
            if (NULL != newAPImonitoredItemCtxArray)
            {
                bool found = false;
                newAPImonitoredItemCtxArray[i] =
                    SOPC_Dict_Get(subCtx->miCliHandleToUserAppCtxDict, (uintptr_t) pMonItNotif->ClientHandle, &found);
                if (!found)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Unexpected monitored item client handle not found.");
                }
            }
        }
    }

    if (NULL != pSM->pCbkNotification)
    {
        if (pDataNotif->NoOfMonitoredItems < 0)
        {
            pDataNotif->NoOfMonitoredItems = 0;
        }

        // Callback shall not be called with locked mutex to avoid possible deadlock
        SOPC_ReturnStatus status = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        pSM->pCbkNotification(subCtx->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult,
                              &OpcUa_DataChangeNotification_EncodeableType, (uint32_t) pDataNotif->NoOfMonitoredItems,
                              pDataNotif, newAPImonitoredItemCtxArray);

        SOPC_Free(newAPImonitoredItemCtxArray);

        // Restore lock on state machine
        status = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
}

static void LockedStaMac_ProcessMsg_PubResp_EventNotifList(SOPC_StaMac_Machine* pSM,
                                                           OpcUa_PublishResponse* pPubResp,
                                                           OpcUa_EventNotificationList* pEventNotif)
{
    uintptr_t* newAPImonitoredItemCtxArray = NULL;
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pPubResp->SubscriptionId);
    if (NULL == subCtx)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               pPubResp->SubscriptionId);
        return;
    }
    if (NULL != pSM->pCbkNotification && pEventNotif->NoOfEvents > 0)
    {
        newAPImonitoredItemCtxArray = SOPC_Calloc((size_t) pEventNotif->NoOfEvents, sizeof(uintptr_t));
    }
    // Retrieve user context associated to each MI and set it in dedicated array (same index as MI)
    for (int32_t i = 0; NULL != newAPImonitoredItemCtxArray && i < pEventNotif->NoOfEvents; ++i)
    {
        bool found = false;

        uintptr_t userAppCtx =
            SOPC_Dict_Get(subCtx->miCliHandleToUserAppCtxDict, (uintptr_t) pEventNotif->Events[i].ClientHandle, &found);
        newAPImonitoredItemCtxArray[i] = userAppCtx;
        if (!found)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Unexpected monitored item client handle not found.");
        }
    }

    if (NULL != pSM->pCbkNotification)
    {
        if (pEventNotif->NoOfEvents < 0)
        {
            pEventNotif->NoOfEvents = 0;
        }

        // Callback shall not be called with locked mutex to avoid possible deadlock
        SOPC_ReturnStatus status = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        pSM->pCbkNotification(subCtx->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult,
                              &OpcUa_EventNotificationList_EncodeableType, (uint32_t) pEventNotif->NoOfEvents,
                              pEventNotif, newAPImonitoredItemCtxArray);

        SOPC_Free(newAPImonitoredItemCtxArray);

        // Restore lock on state machine
        status = SOPC_Mutex_Lock(&pSM->mutex);
    }
}

static void LockedStaMac_ProcessMsg_PubResp_StatusChangeNotif(SOPC_StaMac_Machine* pSM,
                                                              OpcUa_PublishResponse* pPubResp,
                                                              OpcUa_StatusChangeNotification* pStatusNotif)
{
    const subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pPubResp->SubscriptionId);
    if (NULL == subCtx)
    {
        return;
    }

    if (NULL != pSM->pCbkNotification)
    {
        // Callback shall not be called with locked mutex to avoid possible deadlock
        SOPC_ReturnStatus status = SOPC_Mutex_Unlock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        pSM->pCbkNotification(subCtx->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult,
                              &OpcUa_StatusChangeNotification_EncodeableType, 0, pStatusNotif, NULL);

        // Restore lock on state machine
        status = SOPC_Mutex_Lock(&pSM->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
}

static void LockedStaMac_TreatTooManyPublishRequests(SOPC_StaMac_Machine* pSM)
{
    // Adapt the target to avoid sending too many requests
    if (!pSM->tooManyTokenRcvd && pSM->nTokenTarget > 1)
    {
        pSM->nTokenTarget--;
    }
    // Inhibit sending until next PublishResponse is received
    pSM->tooManyTokenRcvd = true;
}

static void LockedStaMac_ProcessMsg_PublishResponse(SOPC_StaMac_Machine* pSM,
                                                    uint32_t arg,
                                                    void* pParam,
                                                    uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx); // Nothing in context

    OpcUa_PublishResponse* pPubResp = NULL;
    OpcUa_NotificationMessage* pNotifMsg = NULL;
    OpcUa_DataChangeNotification* pDataNotif = NULL;

    /* There should be an EncodeableType pointer in the first field of the message struct */
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "PublishResponse received.");

    pPubResp = (OpcUa_PublishResponse*) pParam;
    /* Take note to acknowledge in next PublishRequest (which should be sent just after this function call).
       note: there is no ack with KeepAlive. */
    /* TODO: in the future we should manage need for republish and thus using the actual SN received instead of
     * available ones. For now it allows the server to avoid keeping them for nothing since no republish will never be
     * requested by us. */
    subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pPubResp->SubscriptionId);
    pNotifMsg = &pPubResp->NotificationMessage;
    /* TODO: manage available sequence numbers (AvailableSequenceNumbers) once we manage re-connection / republish */
    if (NULL != subCtx)
    {
        subCtx->bAckSubscr = true;
        SOPC_SLinkedList_Append(subCtx->pListAckSeqNum, 0, (uintptr_t) pNotifMsg->SequenceNumber);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               pPubResp->SubscriptionId);
    }
    if (pSM->nTokenUsable > 0) // Ensure we do not underflow
    {
        pSM->nTokenUsable -= 1;
    }
    else
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Unexpected number of PublishResponse received.");
    }

    /* Traverse the notifications and calls the callback */
    /* For now, only handles at most a NotificationData */
    SOPC_ASSERT(2 >= pNotifMsg->NoOfNotificationData);
    if (0 < pNotifMsg->NoOfNotificationData && SOPC_IsGoodStatus(pPubResp->ResponseHeader.ServiceResult))
    {
        for (int32_t iNotif = 0; iNotif < pNotifMsg->NoOfNotificationData; iNotif++)
        {
            if (&OpcUa_DataChangeNotification_EncodeableType == pNotifMsg->NotificationData[iNotif].Body.Object.ObjType)
            {
                SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == pNotifMsg->NotificationData[iNotif].Encoding);

                pDataNotif = (OpcUa_DataChangeNotification*) pNotifMsg->NotificationData[iNotif].Body.Object.Value;
                LockedStaMac_ProcessMsg_PubResp_NotifData(pSM, pPubResp, pDataNotif);
            }
            else if (&OpcUa_EventNotificationList_EncodeableType ==
                         pNotifMsg->NotificationData[iNotif].Body.Object.ObjType &&
                     NULL != pSM->pCbkNotification)
            {
                SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == pNotifMsg->NotificationData[iNotif].Encoding);

                // New API compatible only
                OpcUa_EventNotificationList* pEventNotif =
                    (OpcUa_EventNotificationList*) pNotifMsg->NotificationData[iNotif].Body.Object.Value;
                LockedStaMac_ProcessMsg_PubResp_EventNotifList(pSM, pPubResp, pEventNotif);
            }
            else if (&OpcUa_StatusChangeNotification_EncodeableType ==
                         pNotifMsg->NotificationData[iNotif].Body.Object.ObjType &&
                     NULL != pSM->pCbkNotification)
            {
                SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == pNotifMsg->NotificationData[iNotif].Encoding);

                // New API compatible only
                OpcUa_StatusChangeNotification* pStatusChangeNotif =
                    (OpcUa_StatusChangeNotification*) pNotifMsg->NotificationData[iNotif].Body.Object.Value;
                LockedStaMac_ProcessMsg_PubResp_StatusChangeNotif(pSM, pPubResp, pStatusChangeNotif);
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Unexpected notification type received %s.",
                                       pNotifMsg->NotificationData[iNotif].Body.Object.ObjType->TypeName);
            }
        }
    }
    /* TODO: verify the results[] which contains a status for each Ack */

    if (OpcUa_BadTooManyPublishRequests == pPubResp->ResponseHeader.ServiceResult)
    {
        LockedStaMac_TreatTooManyPublishRequests(pSM);
    }
    else
    {
        pSM->tooManyTokenRcvd = false;
    }
}

static void LockedStaMac_ProcessMsg_CreateSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                               uint32_t arg,
                                                               void* pParam,
                                                               uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx);

    SOPC_ASSERT(pSM->bSubscriptionInProgress);
    OpcUa_CreateSubscriptionResponse* resp = (OpcUa_CreateSubscriptionResponse*) pParam;

    // Create sub related objects
    subscriptionCtx* subCtx = SOPC_Malloc(sizeof(subscriptionCtx));
    if (NULL != subCtx)
    {
        subCtx->fPublishInterval = resp->RevisedPublishingInterval;
        subCtx->iCntLifetime = resp->RevisedLifetimeCount;
        subCtx->iCntMaxKeepAlive = resp->RevisedMaxKeepAliveCount;
        subCtx->subscriptionAppCtx = appCtx;
        subCtx->pListMonIt = SOPC_SLinkedList_Create(0);
        subCtx->pListDelMonIt = SOPC_SLinkedList_Create(0);
        subCtx->pListAckSeqNum = SOPC_SLinkedList_Create(0);
        subCtx->miCliHandleToUserAppCtxDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
        SOPC_Dict_SetTombstoneKey(subCtx->miCliHandleToUserAppCtxDict, DICT_TOMBSTONE); // Necessary for remove
        subCtx->miIdToCliHandleDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
        SOPC_Dict_SetTombstoneKey(subCtx->miIdToCliHandleDict, DICT_TOMBSTONE); // Necessary for remove
        subCtx->bAckSubscr = false;

        if (NULL == subCtx->pListMonIt || NULL == subCtx->pListDelMonIt || NULL == subCtx->pListAckSeqNum ||
            NULL == subCtx->miCliHandleToUserAppCtxDict || NULL == subCtx->miIdToCliHandleDict)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Subscription %" PRIu32
                                   "could not be created due "
                                   "to memory allocation problem.",
                                   resp->SubscriptionId);
            free_subscriptionCtx(0, (uintptr_t) subCtx);
        }
        else
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Subscription %" PRIu32 " created.",
                                  resp->SubscriptionId);
            // Add the sub to the state machine
            SOPC_SLinkedList_Append(pSM->pListSubscriptions, resp->SubscriptionId, (uintptr_t) subCtx);
            pSM->latestSubscriptionId = resp->SubscriptionId;
            pSM->bSubscriptionInProgressCreated = true;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription %" PRIu32
                               " could not be added to config due "
                               "to memory allocation problem.",
                               resp->SubscriptionId);
    }

    pSM->bSubscriptionInProgress = false;
    pSM->state = stActivated;
}

static void LockedStaMac_ProcessMsg_DeleteSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                               uint32_t arg,
                                                               void* pParam,
                                                               uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    uint32_t subscriptionId = (uint32_t) appCtx;

    if (1 != ((OpcUa_DeleteSubscriptionsResponse*) pParam)->NoOfResults)
    {
        /* we should have deleted only one subscription */
        pSM->state = stError;
    }
    else if (0 != ((OpcUa_DeleteSubscriptionsResponse*) pParam)->Results[0])
    {
        /* delete subscription went wrong */
        pSM->state = stError;
    }

    // Clear the value (= subCtx)
    subscriptionCtx* subCtx = (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, subscriptionId);
    if (NULL != subCtx)
    {
        SOPC_SLinkedList_Delete(subCtx->pListMonIt);
        subCtx->pListMonIt = NULL;
        SOPC_SLinkedList_Delete(subCtx->pListDelMonIt);
        subCtx->pListDelMonIt = NULL;
        SOPC_SLinkedList_Delete(subCtx->pListAckSeqNum);
        subCtx->pListAckSeqNum = NULL;
        SOPC_Dict_Delete(subCtx->miCliHandleToUserAppCtxDict);
        subCtx->miCliHandleToUserAppCtxDict = NULL;
        SOPC_Dict_Delete(subCtx->miIdToCliHandleDict);
        subCtx->miIdToCliHandleDict = NULL;
        SOPC_Free(subCtx);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               subscriptionId);
    }
    // Remove the key (= subscriptionId)
    SOPC_SLinkedList_RemoveFromId(pSM->pListSubscriptions, subscriptionId);

    // Reset values that will not be used anymore if no more subscription available
    if (0 == SOPC_SLinkedList_GetLength(pSM->pListSubscriptions))
    {
        SOPC_SLinkedList_Apply(pSM->dataIdToNodeIdList, SOPC_SLinkedList_EltGenericFree);
        SOPC_SLinkedList_Clear(pSM->dataIdToNodeIdList);
        pSM->nTokenUsable = 0;
        pSM->tooManyTokenRcvd = false;
    }

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Subscription deleted.");
    pSM->state = stActivated;
}

static void LockedStaMac_ProcessMsg_CreateMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                                 uint32_t arg,
                                                                 void* pParam,
                                                                 uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);

    int32_t i = 0;
    OpcUa_CreateMonitoredItemsResponse* pMonItResp = NULL;

    pMonItResp = (OpcUa_CreateMonitoredItemsResponse*) pParam;
    SOPC_CreateMonitoredItems_Ctx* MIappCtx = (SOPC_CreateMonitoredItems_Ctx*) appCtx;
    SOPC_ASSERT(NULL != pMonItResp);
    OpcUa_CreateMonitoredItemsRequest* pMonItReq = MIappCtx->req;

    // Retrieve subCtx
    subscriptionCtx* subCtx = NULL;
    if (NULL != pMonItReq)
    {
        subCtx = (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pMonItReq->SubscriptionId);
        if (NULL != subCtx)
        {
            for (i = 0; i < pMonItResp->NoOfResults; ++i)
            {
                if (SOPC_IsGoodStatus(pMonItResp->Results[i].StatusCode))
                {
                    bool result = SOPC_Dict_Insert(
                        subCtx->miIdToCliHandleDict, (uintptr_t) pMonItResp->Results[i].MonitoredItemId,
                        (uintptr_t) pMonItReq->ItemsToCreate[i].RequestedParameters.ClientHandle);
                    if (!result)
                    {
                        pMonItResp->Results[i].StatusCode = OpcUa_BadInternalError;
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "Internal error creating monitored item with index '%" PRIi32 ".", i);
                    }
                }
            }
            if (pMonItResp->NoOfResults > 0)
            {
                bool result = SOPC_SLinkedList_Append(subCtx->pListMonIt, pMonItResp->Results[0].MonitoredItemId,
                                                      MIappCtx->outCtxId);
                if (!result)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Internal error creating monitored item result context");
                }
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                                   pMonItReq->SubscriptionId);
        }
    }

    OpcUa_CreateMonitoredItemsResponse* resp = ((SOPC_CreateMonitoredItems_Ctx*) appCtx)->Results;
    if (NULL != resp)
    {
        // Transfer response data into app context
        *resp = *pMonItResp;
        SOPC_EncodeableObject_Initialize(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, pMonItResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pMonItReq);
    pSM->state = stActivated;
}

static void LockedStaMac_ProcessMsg_DeleteMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                                 uint32_t arg,
                                                                 void* pParam,
                                                                 uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);

    int32_t i = 0;
    OpcUa_DeleteMonitoredItemsResponse* pMonItResp = NULL;

    pMonItResp = (OpcUa_DeleteMonitoredItemsResponse*) pParam;
    SOPC_DeleteMonitoredItems_Ctx* MIappCtx = (SOPC_DeleteMonitoredItems_Ctx*) appCtx;
    SOPC_ASSERT(NULL != pMonItResp);
    OpcUa_DeleteMonitoredItemsRequest* pMonItReq = MIappCtx->req;

    // Retrieve subCtx
    subscriptionCtx* subCtx =
        (subscriptionCtx*) SOPC_SLinkedList_FindFromId(pSM->pListSubscriptions, pMonItReq->SubscriptionId);
    if (NULL != subCtx)
    {
        for (i = 0; i < pMonItResp->NoOfResults; ++i)
        {
            if (SOPC_IsGoodStatus(pMonItResp->Results[i]))
            {
                bool found = false;
                uintptr_t miCliHandle =
                    SOPC_Dict_Get(subCtx->miIdToCliHandleDict, (uintptr_t) pMonItReq->MonitoredItemIds[i], &found);
                if (found)
                {
                    // Remove internal context associated
                    SOPC_Dict_Remove(subCtx->miIdToCliHandleDict, (uintptr_t) pMonItReq->MonitoredItemIds[i]);
                    SOPC_Dict_Remove(subCtx->miCliHandleToUserAppCtxDict, miCliHandle);
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Internal error finding monitored item id %" PRIu32,
                                           pMonItReq->MonitoredItemIds[i]);
                }
            }
        }
        if (pMonItResp->NoOfResults > 0)
        {
            bool result =
                SOPC_SLinkedList_Append(subCtx->pListDelMonIt, pMonItReq->MonitoredItemIds[0], MIappCtx->outCtxId);
            if (!result)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Internal error creating delete monitored item result context");
            }
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription with SubscriptionId %" PRIu32 "could not be found in configuration.",
                               pMonItReq->SubscriptionId);
    }

    OpcUa_DeleteMonitoredItemsResponse* resp = ((SOPC_DeleteMonitoredItems_Ctx*) appCtx)->Results;
    if (NULL != resp)
    {
        // Transfer response data into app context
        *resp = *pMonItResp;
        SOPC_EncodeableObject_Initialize(&OpcUa_DeleteMonitoredItemsResponse_EncodeableType, pMonItResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &pMonItReq);
    pSM->state = stActivated;
}

static void LockedStaMac_ProcessMsg_ServiceFault(SOPC_StaMac_Machine* pSM,
                                                 uint32_t arg,
                                                 void* pParam,
                                                 SOPC_StaMac_RequestType reqType)
{
    SOPC_UNUSED_ARG(arg);
    OpcUa_ServiceFault* servFault = (OpcUa_ServiceFault*) pParam;
    switch (reqType)
    {
    case SOPC_REQUEST_TYPE_PUBLISH:
        /* ServiceFault on PublishRequest is allowed */
        if (pSM->nTokenUsable > 0) // Ensure we do not underflow
        {
            pSM->nTokenUsable -= 1;

            if (OpcUa_BadTooManyPublishRequests == servFault->ResponseHeader.ServiceResult)
            {
                LockedStaMac_TreatTooManyPublishRequests(pSM);
            }
            else
            {
                // Remove flag when service result is not OpcUa_BadTooManyPublishRequests (might be timeout, etc.)
                pSM->tooManyTokenRcvd = false;
            }
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Unexpected number of PublishResponse received.");
        }
        break;
    default:
        /* else go into error mode */
        pSM->state = stError;
        break;
    }
}

static void LockedStaMac_ProcessEvent_SendRequestFailed(SOPC_StaMac_Machine* pSM,
                                                        uint32_t arg,
                                                        void* pParam,
                                                        uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx);

    if (NULL == pParam)
    {
        pSM->state = stError;
    }
    else
    {
        SOPC_EncodeableType* pEncType = (SOPC_EncodeableType*) pParam;

        /* We only process PublishRequest send failed for timeout reason
         * (checked in StaMac_GiveAuthorization_SendRequestFailed) */
        if (&OpcUa_PublishRequest_EncodeableType == pEncType)
        {
            if (pSM->nTokenUsable > 0) // Ensure we do not underflow
            {
                pSM->nTokenUsable -= 1;
            }
            else
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "Unexpected number of PublishResponse received.");
            }
        }
        else
        {
            /* else go into error mode */
            pSM->state = stError;
        }
    }
}

static void LockedStaMac_ProcessEvent_stError(SOPC_StaMac_Machine* pSM,
                                              SOPC_App_Com_Event event,
                                              uint32_t arg,
                                              void* pParam,
                                              uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(pSM);
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx);

    switch (event)
    {
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_CLOSED_SESSION:
    case SE_CLOSED_CHANNEL:
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "Received post-closed closed event or activation failure, ignored.");
        break;
    case SE_SND_REQUEST_FAILED:
        if (NULL != pParam)
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                  "Received post-closed send request failure for message of type %s.",
                                  ((SOPC_EncodeableType*) pParam)->TypeName);
        }
        else
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                  "Received post-closed send request failure for message of unknown type");
        }
        break;
    case SE_SESSION_REACTIVATING:
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "Reactivating event received, but closed connection are considered lost.");
        break;
    default:
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Receiving unexpected event %i in closed state, ignored.", event);
        break;
    }
}
static SOPC_ReturnStatus Helpers_NewPublishRequest(SOPC_StaMac_Machine* pSM, void** ppRequest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_PublishRequest* pReq = NULL;
    OpcUa_SubscriptionAcknowledgement* subAckArray = NULL;
    uint32_t sizeOfSubAckArray = 0;

    if (NULL == ppRequest)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Create(&OpcUa_PublishRequest_EncodeableType, (void**) &pReq);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Iterate on subs in order to get the size of the array SubscriptionAcknowledgement[]
        SOPC_SLinkedListIterator itOnSubs = SOPC_SLinkedList_GetIterator(pSM->pListSubscriptions);
        const subscriptionCtx* subCtx = (subscriptionCtx*) SOPC_SLinkedList_Next(&itOnSubs);
        while (NULL != subCtx)
        {
            sizeOfSubAckArray += SOPC_SLinkedList_GetLength(subCtx->pListAckSeqNum);
            subCtx = (subscriptionCtx*) SOPC_SLinkedList_Next(&itOnSubs);
        }

        if (0 < sizeOfSubAckArray)
        {
            subAckArray = SOPC_Calloc((size_t) sizeOfSubAckArray, sizeof(OpcUa_SubscriptionAcknowledgement));
            if (NULL != subAckArray)
            {
                pReq->NoOfSubscriptionAcknowledgements = (int32_t) sizeOfSubAckArray;
                pReq->SubscriptionAcknowledgements = subAckArray;

                // Fill the array SubscriptionAcknowledgement[]
                // Iterate on subs
                uint32_t subAckArrayIndex = 0;
                uint32_t subId = 0;
                itOnSubs = SOPC_SLinkedList_GetIterator(pSM->pListSubscriptions);
                subCtx = (subscriptionCtx*) SOPC_SLinkedList_NextWithId(&itOnSubs, &subId);
                while (NULL != subCtx)
                {
                    // Pop the elements of the subs's ack sequence numbers list
                    uint32_t ackSeqNum = (uint32_t) SOPC_SLinkedList_PopHead(subCtx->pListAckSeqNum);
                    while (0 != ackSeqNum)
                    {
                        SOPC_ASSERT(subAckArrayIndex < sizeOfSubAckArray);
                        OpcUa_SubscriptionAcknowledgement_Initialize(&subAckArray[subAckArrayIndex]);
                        subAckArray[subAckArrayIndex].SubscriptionId = subId;
                        subAckArray[subAckArrayIndex].SequenceNumber = ackSeqNum;
                        subAckArrayIndex++;
                        ackSeqNum = (uint32_t) SOPC_SLinkedList_PopHead(subCtx->pListAckSeqNum);
                    }
                    subCtx = (subscriptionCtx*) SOPC_SLinkedList_NextWithId(&itOnSubs, &subId);
                }
            }
        }
        else
        {
            pReq->NoOfSubscriptionAcknowledgements = 0;
            pReq->SubscriptionAcknowledgements = NULL;
        }
        *ppRequest = pReq;
    }

    return status;
}

static void set_bAckSubscrp_to_0(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    ((subscriptionCtx*) val)->bAckSubscr = false;
}

/**
 * \brief Do the post process actions: create a subscription, keep the target number of PublishRequest, ...
 *
 * Machine's mutex shall already be locked by the caller.
 */
static void LockedStaMac_PostProcessActions(SOPC_StaMac_Machine* pSM, SOPC_StaMac_State oldState)
{
    SOPC_ASSERT(NULL != pSM);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pRequest = NULL;
    uint32_t NoOfSubs = SOPC_SLinkedList_GetLength(pSM->pListSubscriptions);
    switch (pSM->state)
    {
    /* Mostly when stActivated is reached */
    case stActivated:
    case stCreatingMonIt:
        /* add tokens, but wait for at least a monitored item */
        if (0 < NoOfSubs && pSM->nTokenUsable < pSM->nTokenTarget)
        {
            while (SOPC_STATUS_OK == status && pSM->nTokenUsable < pSM->nTokenTarget && !pSM->tooManyTokenRcvd)
            {
                /* Send a PublishRequest */
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Adding publish token.");

                // Build request with all subscriptions
                status = Helpers_NewPublishRequest(pSM, &pRequest);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_StaMac_SendRequest(pSM, pRequest, 0, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                                     SOPC_REQUEST_TYPE_PUBLISH);
                }
                if (SOPC_STATUS_OK == status)
                {
                    pSM->nTokenUsable += 1;
                    SOPC_SLinkedList_Apply(pSM->pListSubscriptions, &set_bAckSubscrp_to_0);
                }
                else
                {
                    pSM->state = stError;
                }
            }
        }
        break;
    case stClosingChannel:
        if (oldState != stClosingChannel)
        {
            SOPC_StaMacInternal_CloseChannel(pSM);
        }
        break;
    default:
        break;
    }

    /* If state machine is still in error state:
     *  try to send a close session / channel if the state machine was connected before the error */
    if (stError == pSM->state)
    {
        // If session was activated and not closed try to close it
        if (stError != oldState && stClosingChannel != oldState && stClosingSession != oldState &&
            stActivating != oldState && stInit != oldState)
        {
            pSM->state = oldState;
            if (SOPC_StaMac_IsConnected(pSM))
            {
                status = SOPC_StaMac_StopSession(pSM);
                if (SOPC_STATUS_OK != status)
                {
                    pSM->state = stError;
                }
                else
                {
                    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                          "Closing the connection because of the previous error.");
                }
            }
            else
            {
                pSM->state = stError;
            }
        }
        // If session is not closing and channel was not closed try to close it
        if (stError == pSM->state && stError != oldState && stClosingChannel != oldState && stInit != oldState)
        {
            SOPC_StaMacInternal_CloseChannel(pSM);
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                  "Closing the connection because of the previous error.");
        }
    }
}
