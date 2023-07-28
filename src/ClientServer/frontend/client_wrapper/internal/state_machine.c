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
#include "sopc_encodeable.h"
#include "sopc_hash.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_user_app_itf.h"

#include "libs2opc_client_internal.h"
#include "state_machine.h"
#include "toolkit_helpers.h"

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
    uint32_t iscConfig; /* Toolkit scConfig ID */
    SOPC_ReverseEndpointConfigIdx
        reverseConfigIdx; /* Reverse configuration index > 0 if reverse connection mechanism shall be used */
    uint32_t iCliId;      /* LibSub connection ID, used by the callback. It shall be unique. */

    /* Keeping three callbacks waiting the deprecated APIs to be removed for the first 2 cbs */
    SOPC_LibSub_DataChangeCbk* pCbkLibSubDataChanged;             /* Callback when subscribed data changed */
    SOPC_ClientHelper_DataChangeCbk* pCbkClientHelperDataChanged; /* Callback when subscribed data changed */
    SOPC_StaMacNotification_Fct* pCbkNotification;                /* Callback when subscription notification occurs */

    SOPC_LibSub_EventCbk* pCbkGenericEvent; /* Callback when received event that is out of the LibSub scope */
    uintptr_t iSessionCtx;                  /* Toolkit Session Context, used to identify session events */
    uint32_t iSessionID;                    /* S2OPC Session ID */
    SOPC_SLinkedList* pListReqCtx;          /* List of yet-to-be-answered requests,
                                             * id is unique request identifier, value is a SOPC_StaMac_ReqCtx */
    double fPublishInterval;                /* The publish interval, in ms */
    uint32_t iCntMaxKeepAlive;              /* Number of skipped response before sending an empty KeepAlive */
    uint32_t iCntLifetime;                  /* Number of deprived publish cycles before subscription deletion */
    uint32_t iSubscriptionID;               /* OPC UA subscription ID */
    bool bSubscriptionCreated;       /* Flag set when subscription created succesfully and subscription ID is set */
    uintptr_t subscriptionAppCtx;    /* Application context of the subscription (new API only)*/
    uint32_t nMonItClientHandle;     /* Latest client handle generated for monitored items,
                                      * used as unique identifier */
    SOPC_SLinkedList* pListMonIt;    /* List of monitored items creation request successful, where the
                                        SOPC_CreateMonitoredItem_Ctx    is the listed value */
    SOPC_SLinkedList* pListDelMonIt; /* List of monitored items deletion request successful, where the
                                        SOPC_DeleteMonitoredItem_Ctx is the listed value */

    uint32_t nTokenTarget;                     /* Target number of available tokens */
    uint32_t nTokenUsable;                     /* Tokens available to the server
                                                * (PublishRequest_sent - PublishResponse_sent) */
    bool bAckSubscr;                           /* Indicates whether an acknowledgement should be sent
                                                * in the next PublishRequest */
    uint32_t iAckSeqNum;                       /* The sequence number to acknowledge after a PublishResponse */
    const char* szPolicyId;                    /* See SOPC_LibSub_ConnectionCfg */
    const char* szUsername;                    /* See SOPC_LibSub_ConnectionCfg */
    const char* szPassword;                    /* See SOPC_LibSub_ConnectionCfg */
    SOPC_SerializedCertificate* pUserCertX509; /* X509 serialized certificate for X509IdentiyToken (DER format) */
    SOPC_SerializedAsymmetricKey* pUserKey;    /* Serialized private key for X509IdentiyToken (PEM format) */
    int64_t iTimeoutMs;                        /* See SOPC_LibSub_ConnectionCfg.timeout_ms */
    SOPC_SLinkedList* dataIdToNodeIdList;      /* A list of data ids to node ids */
    SOPC_Dict* miCliHandleToUserAppCtxDict;    /* A dictionary of monitored client handles to user app contexts
                                                        (new API only)*/
    SOPC_Dict* miIdToCliHandleDict;            /* A dictionary of ids to client handles (new API only)*/
    uintptr_t userContext;                     /* A state machine user defined context */
};

/* Internal functions */
static bool StaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                                   uintptr_t* pAppCtx,
                                   SOPC_StaMac_RequestScope* pRequestScope,
                                   SOPC_App_Com_Event event,
                                   uint32_t arg,
                                   void* pParam,
                                   uintptr_t appCtx);

static bool StaMac_GiveAuthorization_stActivating(SOPC_StaMac_Machine* pSM,
                                                  SOPC_App_Com_Event event,
                                                  SOPC_EncodeableType* pEncType);
static bool StaMac_GiveAuthorization_stClosing(SOPC_StaMac_Machine* pSM,
                                               SOPC_App_Com_Event event,
                                               SOPC_EncodeableType* pEncType);
static bool StaMac_GiveAuthorization_stActivated(SOPC_StaMac_Machine* pSM,
                                                 SOPC_App_Com_Event event,
                                                 SOPC_EncodeableType* pEncType);
static bool StaMac_GiveAuthorization_stCreatingSubscr(SOPC_StaMac_Machine* pSM,
                                                      SOPC_App_Com_Event event,
                                                      SOPC_EncodeableType* pEncType);
static bool StaMac_GiveAuthorization_stCreatingMonIt(SOPC_StaMac_Machine* pSM,
                                                     SOPC_App_Com_Event event,
                                                     SOPC_EncodeableType* pEncType);
static bool StaMac_GiveAuthorization_stDeletingSubscr(SOPC_StaMac_Machine* pSM,
                                                      SOPC_App_Com_Event event,
                                                      SOPC_EncodeableType* pEncType);

static void StaMac_ProcessMsg_ActivateSessionResponse(SOPC_StaMac_Machine* pSM,
                                                      uint32_t arg,
                                                      void* pParam,
                                                      uintptr_t appCtx);
static void StaMac_ProcessMsg_CloseSessionResponse(SOPC_StaMac_Machine* pSM,
                                                   uint32_t arg,
                                                   void* pParam,
                                                   uintptr_t appCtx);
static void StaMac_ProcessMsg_PublishResponse(SOPC_StaMac_Machine* pSM, uint32_t arg, void* pParam, uintptr_t appCtx);
static void StaMac_ProcessMsg_CreateSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx);
static void StaMac_ProcessMsg_CreateMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                           uint32_t arg,
                                                           void* pParam,
                                                           uintptr_t appCtx);
static void StaMac_ProcessMsg_DeleteMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
                                                           uint32_t arg,
                                                           void* pParam,
                                                           uintptr_t appCtx);
static void StaMac_ProcessMsg_DeleteSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx);
static void StaMac_ProcessMsg_ServiceFault(SOPC_StaMac_Machine* pSM, uint32_t arg, void* pParam, uintptr_t appCtx);
static void StaMac_ProcessEvent_SendRequestFailed(SOPC_StaMac_Machine* pSM,
                                                  uint32_t arg,
                                                  void* pParam,
                                                  uintptr_t appCtx);
static void StaMac_ProcessEvent_stError(SOPC_StaMac_Machine* pSM,
                                        SOPC_App_Com_Event event,
                                        uint32_t arg,
                                        void* pParam,
                                        uintptr_t appCtx);

static void StaMac_PostProcessActions(SOPC_StaMac_Machine* pSM, SOPC_StaMac_State oldState);

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
                                     SOPC_LibSub_DataChangeCbk* pCbkLibSubDataChanged,
                                     double fPublishInterval,
                                     uint32_t iCntMaxKeepAlive,
                                     uint32_t iCntLifetime,
                                     uint32_t iTokenTarget,
                                     int64_t iTimeoutMs,
                                     SOPC_LibSub_EventCbk* pCbkGenericEvent,
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
        pSM->iscConfig = iscConfig;
        pSM->reverseConfigIdx = reverseConfigIdx;
        pSM->iCliId = iCliId;
        pSM->pCbkLibSubDataChanged = pCbkLibSubDataChanged;
        pSM->pCbkClientHelperDataChanged = NULL;
        pSM->iSessionCtx = 0;
        pSM->iSessionID = 0;
        pSM->pListReqCtx = SOPC_SLinkedList_Create(0);
        pSM->fPublishInterval = fPublishInterval;
        pSM->iCntMaxKeepAlive = iCntMaxKeepAlive;
        pSM->iCntLifetime = iCntLifetime;
        pSM->iSubscriptionID = 0;
        pSM->bSubscriptionCreated = false;
        pSM->nMonItClientHandle = 0;
        pSM->pListMonIt = SOPC_SLinkedList_Create(0);
        pSM->pListDelMonIt = SOPC_SLinkedList_Create(0);
        pSM->nTokenTarget = iTokenTarget;
        pSM->nTokenUsable = 0;
        pSM->pCbkGenericEvent = pCbkGenericEvent;
        pSM->bAckSubscr = false;
        pSM->iAckSeqNum = 0;
        pSM->szPolicyId = NULL;
        pSM->szUsername = NULL;
        pSM->szPassword = NULL;
        pSM->pUserCertX509 = NULL;
        pSM->pUserKey = NULL;
        pSM->iTimeoutMs = iTimeoutMs;
        pSM->dataIdToNodeIdList = SOPC_SLinkedList_Create(0);
        pSM->miCliHandleToUserAppCtxDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
        SOPC_Dict_SetTombstoneKey(pSM->miCliHandleToUserAppCtxDict, DICT_TOMBSTONE); // Necessary for remove
        pSM->miIdToCliHandleDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
        SOPC_Dict_SetTombstoneKey(pSM->miIdToCliHandleDict, DICT_TOMBSTONE); // Necessary for remove

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

    if (SOPC_STATUS_OK == status && (NULL == pSM->pListReqCtx || NULL == pSM->pListMonIt ||
                                     NULL == pSM->pListDelMonIt || NULL == pSM->dataIdToNodeIdList ||
                                     NULL == pSM->miCliHandleToUserAppCtxDict || NULL == pSM->miIdToCliHandleDict))
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

SOPC_ReturnStatus SOPC_StaMac_ConfigureDataChangeCallback(SOPC_StaMac_Machine* pSM,
                                                          SOPC_ClientHelper_DataChangeCbk* pCbkClientHelper)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status)
    {
        if ((NULL != pSM->pCbkLibSubDataChanged && NULL != pCbkClientHelper) ||
            (NULL == pSM->pCbkLibSubDataChanged && NULL == pCbkClientHelper))
        {
            /* One and only one callback type should be set */
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    if (SOPC_STATUS_OK == status && NULL != pCbkClientHelper)
    {
        pSM->pCbkClientHelperDataChanged = pCbkClientHelper;
    }
    return status;
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
        SOPC_SLinkedList_Delete(pSM->pListMonIt);
        pSM->pListMonIt = NULL;
        SOPC_SLinkedList_Delete(pSM->pListDelMonIt);
        pSM->pListDelMonIt = NULL;
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
        SOPC_Dict_Delete(pSM->miCliHandleToUserAppCtxDict);
        pSM->miCliHandleToUserAppCtxDict = NULL;
        SOPC_Dict_Delete(pSM->miIdToCliHandleDict);
        pSM->miIdToCliHandleDict = NULL;
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
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "The state machine shall be in stInit state to start a session.");
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
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Unable to identify the type of token to start a session.");
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
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "StopSession on a disconnected machine.");
        pSM->state = stError;
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncCloseSession(pSM->iSessionID);
        pSM->state = stClosing;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
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

SOPC_ReturnStatus SOPC_StaMac_CreateSubscription(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pRequest = NULL;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (!pSM->bSubscriptionCreated && stActivated == pSM->state)
    {
        /* Creates the subscription */
        /* The request is freed by the Toolkit */
        /* TODO: make all value configurable */
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Creating subscription.");
        status = Helpers_NewCreateSubscriptionRequest(pSM->fPublishInterval, pSM->iCntMaxKeepAlive, pSM->iCntLifetime,
                                                      &pRequest);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, pRequest, 0, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                             SOPC_REQUEST_TYPE_SUBSCRIPTION);
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

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewCreateSubscription(SOPC_StaMac_Machine* pSM,
                                                    OpcUa_CreateSubscriptionRequest* req,
                                                    uintptr_t userAppContext)
{
    if (NULL == pSM || NULL == req || &OpcUa_CreateSubscriptionRequest_EncodeableType != req->encodeableType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (!pSM->bSubscriptionCreated && stActivated == pSM->state)
    {
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Creating subscription using provided request.");
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, req, userAppContext, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                             SOPC_REQUEST_TYPE_SUBSCRIPTION);
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
    return status;
}

uintptr_t SOPC_StaMac_GetSubscriptionCtx(SOPC_StaMac_Machine* pSM)
{
    uintptr_t result = 0;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (pSM->bSubscriptionCreated)
    {
        result = pSM->subscriptionAppCtx;
    }
    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return result;
}

SOPC_ReturnStatus SOPC_StaMac_GetSubscriptionRevisedParams(SOPC_StaMac_Machine* pSM,
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
    if (pSM->bSubscriptionCreated)
    {
        if (NULL != revisedPublishingInterval)
        {
            *revisedPublishingInterval = pSM->fPublishInterval;
        }
        if (NULL != revisedLifetimeCount)
        {
            *revisedLifetimeCount = pSM->iCntLifetime;
        }
        if (NULL != revisedMaxKeepAliveCount)
        {
            *revisedMaxKeepAliveCount = pSM->iCntMaxKeepAlive;
        }
    }
    else
    {
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

SOPC_ReturnStatus SOPC_StaMac_DeleteSubscription(SOPC_StaMac_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pRequest = NULL;

    if (SOPC_StaMac_HasSubscription(pSM) && stActivated == pSM->state)
    {
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Deleting subscription.");

        if (SOPC_STATUS_OK == status)
        {
            status = Helpers_NewDeleteSubscriptionRequest(pSM->iSubscriptionID, &pRequest);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, pRequest, pSM->subscriptionAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                             SOPC_REQUEST_TYPE_SUBSCRIPTION);
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

SOPC_ReturnStatus SOPC_StaMac_CreateMonitoredItem(SOPC_StaMac_Machine* pSM,
                                                  char const* const* lszNodeId,
                                                  const uint32_t* liAttrId,
                                                  int32_t nElems,
                                                  SOPC_CreateMonitoredItems_Ctx* pAppCtx,
                                                  uint32_t* lCliHndl)
{
    void* pReq = NULL;

    if (NULL == pSM || NULL == lszNodeId || NULL == liAttrId || NULL == pAppCtx || NULL == lCliHndl || 0 >= nElems)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!SOPC_StaMac_HasSubscription(pSM))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "the machine shall have a created subscription.");
        return SOPC_STATUS_INVALID_STATE;
    }

    /* Create the NodeIds from the strings */
    SOPC_NodeId** lpNid = SOPC_Calloc((size_t) nElems, sizeof(SOPC_NodeId*));
    if (NULL == lpNid)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (int i = 0; i < nElems; ++i)
    {
        size_t szLen = strlen(lszNodeId[i]);
        if (INT32_MAX < szLen)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "creating monitored item, NodeId string is too long.");
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        lpNid[i] = SOPC_NodeId_FromCString(lszNodeId[i], (int32_t) szLen);
        if (NULL == lpNid[i])
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "creating monitored item, could not convert \"%s\" to NodeId.",
                        lszNodeId[i]);
            status = SOPC_STATUS_NOK;
        }
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (stActivated != pSM->state)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                    "creating monitored item, the machine should be in the stActivated state (is in %i).", pSM->state);
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Create the CreateMonitoredItemRequest */
    if (SOPC_STATUS_OK == status)
    {
        for (int i = 0; i < nElems; ++i)
        {
            void* nodeId = SOPC_Calloc(1, sizeof(char) * (strlen(lszNodeId[i]) + 1));
            if (NULL == nodeId)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                void* result = NULL;
                if (pSM->nMonItClientHandle < UINT32_MAX)
                {
                    pSM->nMonItClientHandle++;
                    lCliHndl[i] = pSM->nMonItClientHandle;
                    strcpy(nodeId, lszNodeId[i]);
                    result = (void*) SOPC_SLinkedList_Append(pSM->dataIdToNodeIdList, pSM->nMonItClientHandle,
                                                             (uintptr_t) nodeId);
                }
                if (NULL == result)
                {
                    SOPC_Free(nodeId);
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            pAppCtx->req = NULL;
            pAppCtx->outCtxId = lCliHndl[0];
            status = Helpers_NewCreateMonitoredItemsRequest(lpNid, liAttrId, nElems, pSM->iSubscriptionID,
                                                            MONIT_TIMESTAMPS_TO_RETURN, lCliHndl, MONIT_QSIZE, &pReq);
        }
    }

    /* Send it */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_SendRequest(pSM, pReq, (uintptr_t) pAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                         SOPC_REQUEST_TYPE_SUBSCRIPTION);
    }

    /* Update the machine, the *pAppCtx, and *pCliHndl */
    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stCreatingMonIt;
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pReq);
    }

    for (int i = 0; NULL != lpNid && i < nElems; ++i)
    {
        SOPC_Free(lpNid[i]);
        lpNid[i] = NULL;
    }
    SOPC_Free(lpNid);
    lpNid = NULL;

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewConfigureNotificationCallback(SOPC_StaMac_Machine* pSM,
                                                               SOPC_StaMacNotification_Fct* pNotificationCb)
{
    if (NULL == pSM || NULL == pNotificationCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL != pSM->pCbkNotification || NULL != pSM->pCbkClientHelperDataChanged || NULL != pSM->pCbkLibSubDataChanged)
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
                                                      OpcUa_CreateMonitoredItemsRequest* req,
                                                      const uintptr_t* userAppCtxArray,
                                                      SOPC_CreateMonitoredItems_Ctx* pAppCtx)
{
    if (NULL == pSM || NULL == req || 0 >= req->NoOfItemsToCreate || NULL == pAppCtx)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!SOPC_StaMac_HasSubscription(pSM))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "the machine shall have a created subscription to create monitored items.");
        return SOPC_STATUS_INVALID_STATE;
    }

    uint32_t nElems = (uint32_t) req->NoOfItemsToCreate;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (stActivated != pSM->state)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                    "creating monitored item, the machine should be in the stActivated state (is in %i).", pSM->state);
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Prepare context to keep a copy of request */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pAppCtx->req);
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
                    uintptr_t result = SOPC_Dict_Get(pSM->miCliHandleToUserAppCtxDict,
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
                    bool result = SOPC_Dict_Insert(pSM->miCliHandleToUserAppCtxDict,
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
            req->SubscriptionId = pSM->iSubscriptionID;
            status = SOPC_EncodeableObject_Copy(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void*) pAppCtx->req,
                                                (void*) req);
        }
    }

    /* Send it */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_SendRequest(pSM, req, (uintptr_t) pAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                         SOPC_REQUEST_TYPE_SUBSCRIPTION);
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pAppCtx->req);
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stCreatingMonIt;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_StaMac_NewDeleteMonitoredItems(SOPC_StaMac_Machine* pSM,
                                                      OpcUa_DeleteMonitoredItemsRequest* req,
                                                      SOPC_DeleteMonitoredItems_Ctx* outAppCtx)
{
    if (NULL == pSM || NULL == req || 0 >= req->NoOfMonitoredItemIds || NULL == outAppCtx)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!SOPC_StaMac_HasSubscription(pSM))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "the machine shall have a created subscription to create monitored items.");
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (stActivated != pSM->state)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                    "deleting monitored item, the machine should be in the stActivated state (is in %i).", pSM->state);
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Prepare context to keep a copy of request */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &outAppCtx->req);
    }

    uintptr_t clientHandle = 0;
    bool findClientHandle = false;
    if (SOPC_STATUS_OK == status)
    {
        for (uint32_t i = 0; !findClientHandle && i < (uint32_t) req->NoOfMonitoredItemIds; i++)
        {
            clientHandle =
                SOPC_Dict_Get(pSM->miIdToCliHandleDict, (uintptr_t) req->MonitoredItemIds[i], &findClientHandle);
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
        req->SubscriptionId = pSM->iSubscriptionID;
        status = SOPC_EncodeableObject_Copy(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void*) outAppCtx->req,
                                            (void*) req);
    }

    /* Send it */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_SendRequest(pSM, req, (uintptr_t) outAppCtx, SOPC_REQUEST_SCOPE_STATE_MACHINE,
                                         SOPC_REQUEST_TYPE_SUBSCRIPTION);
    }
    else
    {
        SOPC_Encodeable_Delete(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &outAppCtx->req);
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stDeletingMonIt;
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

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
    case stClosing:
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

void SOPC_StaMac_SetError(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    pSM->state = stError;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
}

bool SOPC_StaMac_HasSubscription(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool return_code = pSM->bSubscriptionCreated;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

uint32_t SOPC_StaMac_HasSubscriptionId(SOPC_StaMac_Machine* pSM)
{
    if (NULL == pSM)
    {
        return 0;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    uint32_t return_code = pSM->iSubscriptionID;
    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return return_code;
}

bool SOPC_StaMac_PopMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_CreateMonitoredItems_Ctx* pAppCtx)
{
    if (NULL == pSM || NULL == pSM->pListMonIt)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool bHasMonIt = false;
    SOPC_SLinkedListIterator pIter = NULL;
    pIter = SOPC_SLinkedList_GetIterator(pSM->pListMonIt);

    while (!bHasMonIt && NULL != pIter)
    {
        if (SOPC_SLinkedList_Next(&pIter) == pAppCtx->outCtxId)
        {
            bHasMonIt = true;
        }
    }

    if (bHasMonIt)
    {
        // Remove context from list since retrieved by client
        SOPC_SLinkedList_RemoveFromValuePtr(pSM->pListMonIt, pAppCtx->outCtxId);
    }

    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return bHasMonIt;
}

bool SOPC_StaMac_PopDeleteMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_DeleteMonitoredItems_Ctx* pAppCtx)
{
    if (NULL == pSM || NULL == pSM->pListDelMonIt)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    bool bHasMonIt = false;
    SOPC_SLinkedListIterator pIter = NULL;
    pIter = SOPC_SLinkedList_GetIterator(pSM->pListDelMonIt);

    while (!bHasMonIt && NULL != pIter)
    {
        if (SOPC_SLinkedList_Next(&pIter) == pAppCtx->outCtxId)
        {
            bHasMonIt = true;
        }
    }

    if (bHasMonIt)
    {
        // Remove context from list since retrieved by client
        SOPC_SLinkedList_RemoveFromValuePtr(pSM->pListDelMonIt, pAppCtx->outCtxId);
    }

    status = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return bHasMonIt;
}

int64_t SOPC_StaMac_GetTimeout(SOPC_StaMac_Machine* pSM)
{
    SOPC_ASSERT(NULL != pSM);
    return pSM->iTimeoutMs;
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

static bool StaMac_GiveAuthorization_stActivating(SOPC_StaMac_Machine* pSM,
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

static bool StaMac_GiveAuthorization_stClosing(SOPC_StaMac_Machine* pSM,
                                               SOPC_App_Com_Event event,
                                               SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);
    SOPC_UNUSED_ARG(pEncType);

    switch (event)
    {
    case SE_CLOSED_SESSION:
        authorization = true;
        break;
    default:
        break;
    }

    return authorization;
}

static bool StaMac_GiveAuthorization_stActivated(SOPC_StaMac_Machine* pSM,
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

static bool StaMac_GiveAuthorization_stCreatingSubscr(SOPC_StaMac_Machine* pSM,
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

static bool StaMac_GiveAuthorization_stCreatingMonIt(SOPC_StaMac_Machine* pSM,
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

static bool StaMac_GiveAuthorization_stDeletingMonIt(SOPC_StaMac_Machine* pSM,
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

static bool StaMac_GiveAuthorization_stDeletingSubscr(SOPC_StaMac_Machine* pSM,
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
                                                       SOPC_ReturnStatus failureStatus,
                                                       SOPC_EncodeableType* pEncType)
{
    bool authorization = false;

    SOPC_UNUSED_ARG(pSM);

    switch (event)
    {
    case SE_SND_REQUEST_FAILED:
        // We only treat a send request failed event if it concerns a timed out publish request
        if (&OpcUa_PublishRequest_EncodeableType == pEncType && SOPC_STATUS_TIMEOUT == failureStatus)
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
    case stClosing:
        return "stClosing";
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

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bProcess = StaMac_IsEventTargeted(pSM, &appCtx, &requestScope, event, arg, pParam, toolkitCtx);

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
                processingAuthorization = StaMac_GiveAuthorization_stActivating(pSM, event, pEncType);
                if (!processingAuthorization && SE_SESSION_ACTIVATION_FAILURE == event)
                {
                    Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Session activation failed with status code 0x%X",
                                (SOPC_StatusCode)(uintptr_t) pParam);
                }
                break;
            case stClosing:
                processingAuthorization = StaMac_GiveAuthorization_stClosing(pSM, event, pEncType);
                break;
            /* Main state */
            case stActivated:
                processingAuthorization = StaMac_GiveAuthorization_stActivated(pSM, event, pEncType);
                break;
            /* Creating* states */
            case stCreatingSubscr:
                processingAuthorization = StaMac_GiveAuthorization_stCreatingSubscr(pSM, event, pEncType);
                break;
            case stCreatingMonIt:
                processingAuthorization = StaMac_GiveAuthorization_stCreatingMonIt(pSM, event, pEncType);
                break;
            /* Deleting states */
            case stDeletingMonIt:
                processingAuthorization = StaMac_GiveAuthorization_stDeletingMonIt(pSM, event, pEncType);
                break;
            case stDeletingSubscr:
                processingAuthorization = StaMac_GiveAuthorization_stDeletingSubscr(pSM, event, pEncType);
                break;
            /* Invalid states */
            case stError:
                StaMac_ProcessEvent_stError(pSM, event, arg, pParam, appCtx);
                processingAuthorization = false;
                break;
            case stInit:
            default:
                processingAuthorization = false;
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Dispatching in unknown state %s, event %s.",
                            SOPC_StaMacState_ToString(pSM->state), SOPC_ClientAppComEvent_ToString(event));
                break;
            }

            /* always authorize processing of service faults */
            if (&OpcUa_ServiceFault_EncodeableType == pEncType)
            {
                Helpers_Log(SOPC_LOG_LEVEL_INFO, "Received ServiceFault");
                processingAuthorization = true;
            }

            /* Authorize processing of send request failed if it is a timeout of PublishRequest */
            if (SE_SND_REQUEST_FAILED == event)
            {
                Helpers_Log(SOPC_LOG_LEVEL_INFO, "Dispatching event SE_SND_REQUEST_FAILED");
                processingAuthorization =
                    StaMac_GiveAuthorization_SendRequestFailed(pSM, event, (SOPC_ReturnStatus) arg, pEncType);
            }

            /* Process message if authorization has been given, else go to stError */
            if (processingAuthorization)
            {
                if (SE_ACTIVATED_SESSION == event)
                {
                    StaMac_ProcessMsg_ActivateSessionResponse(pSM, arg, pParam, appCtx);
                }
                else if (SE_CLOSED_SESSION == event)
                {
                    StaMac_ProcessMsg_CloseSessionResponse(pSM, arg, pParam, appCtx);
                }
                else if (SE_RCV_SESSION_RESPONSE == event)
                {
                    if (&OpcUa_PublishResponse_EncodeableType == pEncType)
                    {
                        StaMac_ProcessMsg_PublishResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_CreateMonitoredItemsResponse_EncodeableType == pEncType)
                    {
                        StaMac_ProcessMsg_CreateMonitoredItemsResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_DeleteMonitoredItemsResponse_EncodeableType == pEncType)
                    {
                        StaMac_ProcessMsg_DeleteMonitoredItemsResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_CreateSubscriptionResponse_EncodeableType == pEncType)
                    {
                        StaMac_ProcessMsg_CreateSubscriptionResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_DeleteSubscriptionsResponse_EncodeableType == pEncType)
                    {
                        StaMac_ProcessMsg_DeleteSubscriptionResponse(pSM, arg, pParam, appCtx);
                    }
                    else if (&OpcUa_ServiceFault_EncodeableType == pEncType)
                    {
                        /* give appCtx, and not internal app context, to know more about the service fault */
                        StaMac_ProcessMsg_ServiceFault(pSM, arg, pParam, toolkitCtx);
                    }
                    else
                    {
                        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Received unknown message in event %s",
                                    SOPC_ClientAppComEvent_ToString(event));
                        pSM->state = stError;
                    }
                }
                else if (SE_SND_REQUEST_FAILED == event)
                {
                    // Use same processing as service fault: it concerns only publish request
                    StaMac_ProcessEvent_SendRequestFailed(pSM, arg, pParam, appCtx);
                }
                else
                {
                    Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Received unexpected event %d", event);
                    pSM->state = stError;
                }
            }
            else
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR,
                            "Received unexpected message or event '%s' in state '%s', switching to error state",
                            SOPC_ClientAppComEvent_ToString(event), SOPC_StaMacState_ToString(pSM->state));
                pSM->state = stError;
            }
        }
        /* Forward the event to the generic event callback if it is not an error */
        else
        {
            SOPC_ASSERT(SOPC_REQUEST_SCOPE_APPLICATION == requestScope);
            if (SE_SND_REQUEST_FAILED == event)
            {
                pSM->state = stClosing;
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Applicative message could not be sent, closing the connection.");
                if (NULL != pSM->pCbkGenericEvent)
                {
                    (*pSM->pCbkGenericEvent)(pSM->iCliId, SOPC_LibSub_ApplicativeEvent_SendFailed, arg, NULL, appCtx);
                }
            }
            else
            {
                if (NULL != pSM->pCbkGenericEvent)
                {
                    (*pSM->pCbkGenericEvent)(pSM->iCliId, SOPC_LibSub_ApplicativeEvent_Response, SOPC_STATUS_OK, pParam,
                                             appCtx);
                }
            }
        }

        StaMac_PostProcessActions(pSM, oldState);
    }

    mutStatus = SOPC_Mutex_Unlock(&pSM->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

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
 * Machine's mutex shall already be locked by the caller.
 *
 * When returns true, the event is targeted to this machine, the pAppCtx is set if not NULL,
 * as well as the request scope.
 */
static bool StaMac_IsEventTargeted(SOPC_StaMac_Machine* pSM,
                                   uintptr_t* pAppCtx,
                                   SOPC_StaMac_RequestScope* pRequestScope,
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
            SOPC_Free(reqCtx);
        }
        else
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Failed to pop the request from the pListReqCtx.");
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
        break;
    case SE_REVERSE_ENDPOINT_CLOSED:
        // Not managed in state machine
        bProcess = false;
        break;
    default:
        bProcess = false;
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Unexpected event received by a machine.");
        break;
    }

    return bProcess;
}

static void StaMac_ProcessMsg_ActivateSessionResponse(SOPC_StaMac_Machine* pSM,
                                                      uint32_t arg,
                                                      void* pParam,
                                                      uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(pParam);
    SOPC_UNUSED_ARG(appCtx);

    pSM->state = stActivated;
    pSM->iSessionID = arg;
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Session activated.");
}

static void StaMac_ProcessMsg_CloseSessionResponse(SOPC_StaMac_Machine* pSM,
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

static void StaMac_ProcessMsg_PubResp_NotifData(SOPC_StaMac_Machine* pSM,
                                                OpcUa_PublishResponse* pPubResp,
                                                OpcUa_DataChangeNotification* pDataNotif)
{
    SOPC_LibSub_Value* plsVal = NULL;
    uintptr_t* newAPImonitoredItemCtxArray = NULL;
    OpcUa_MonitoredItemNotification* pMonItNotif = NULL;

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
                    SOPC_Dict_Get(pSM->miCliHandleToUserAppCtxDict, (uintptr_t) pMonItNotif->ClientHandle, &found);
                if (!found)
                {
                    Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Unexpected monitored item client handle not found.");
                }
            }
        }
        else // deprecated APIs behavior
        {
            SOPC_ReturnStatus status = Helpers_NewValueFromDataValue(&pMonItNotif->Value, &plsVal);
            if (SOPC_STATUS_OK == status)
            {
                if (NULL != pSM->pCbkLibSubDataChanged)
                {
                    (*pSM->pCbkLibSubDataChanged)(pSM->iCliId, pMonItNotif->ClientHandle, plsVal);
                }
                else if (NULL != pSM->pCbkClientHelperDataChanged && INT32_MAX >= pSM->iCliId)
                {
                    void* nodeId =
                        (void*) SOPC_SLinkedList_FindFromId(pSM->dataIdToNodeIdList, pMonItNotif->ClientHandle);
                    if (NULL != nodeId)
                    {
                        (*pSM->pCbkClientHelperDataChanged)((int32_t) pSM->iCliId, (char*) nodeId, &pMonItNotif->Value);
                    }
                }
                SOPC_Free(plsVal->value);
                plsVal->value = NULL;
                SOPC_Variant_Delete(plsVal->raw_value);
                SOPC_Free(plsVal);
                plsVal = NULL;
            }
        }
    }

    if (NULL != pSM->pCbkNotification)
    {
        if (pDataNotif->NoOfMonitoredItems < 0)
        {
            pDataNotif->NoOfMonitoredItems = 0;
        }

        pSM->pCbkNotification(pSM->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult,
                              &OpcUa_DataChangeNotification_EncodeableType, (uint32_t) pDataNotif->NoOfMonitoredItems,
                              pDataNotif, newAPImonitoredItemCtxArray);

        SOPC_Free(newAPImonitoredItemCtxArray);
    }
}

static void StaMac_ProcessMsg_PubResp_EventNotifList(SOPC_StaMac_Machine* pSM,
                                                     OpcUa_PublishResponse* pPubResp,
                                                     OpcUa_EventNotificationList* pEventNotif)
{
    SOPC_ASSERT(NULL != pSM->pCbkLibSubDataChanged);
    uintptr_t* newAPImonitoredItemCtxArray = NULL;
    if (NULL != pSM->pCbkNotification && pEventNotif->NoOfEvents > 0)
    {
        newAPImonitoredItemCtxArray = SOPC_Calloc((size_t) pEventNotif->NoOfEvents, sizeof(uintptr_t));
    }
    // Retrieve user context associated to each MI and set it in dedicated array (same index as MI)
    for (int32_t i = 0; NULL != newAPImonitoredItemCtxArray && i < pEventNotif->NoOfEvents; ++i)
    {
        bool found = false;
        uintptr_t userAppCtx = (uintptr_t) SOPC_Dict_Get(pSM->miCliHandleToUserAppCtxDict,
                                                         (uintptr_t) pEventNotif->Events[i].ClientHandle, &found);
        newAPImonitoredItemCtxArray[i] = userAppCtx;
        if (!found)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Unexpected monitored item client handle not found.");
        }
    }

    if (NULL != pSM->pCbkNotification)
    {
        if (pEventNotif->NoOfEvents < 0)
        {
            pEventNotif->NoOfEvents = 0;
        }

        pSM->pCbkNotification(pSM->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult,
                              &OpcUa_EventNotificationList_EncodeableType, (uint32_t) pEventNotif->NoOfEvents,
                              pEventNotif, newAPImonitoredItemCtxArray);
    }
}

static void StaMac_ProcessMsg_PublishResponse(SOPC_StaMac_Machine* pSM, uint32_t arg, void* pParam, uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx); // Nothing in context

    OpcUa_PublishResponse* pPubResp = NULL;
    OpcUa_NotificationMessage* pNotifMsg = NULL;
    OpcUa_DataChangeNotification* pDataNotif = NULL;

    /* There should be an EncodeableType pointer in the first field of the message struct */
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "PublishResponse received.");

    if (!pSM->bSubscriptionCreated)
    {
        // Ignore PublishResponse since subscription is not valid anymore
        return;
    }

    pPubResp = (OpcUa_PublishResponse*) pParam;
    /* Take note to acknowledge in next PublishRequest (which should be sent just after this function call).
       note: there is no ack with KeepAlive. */
    /* TODO: in the future we should manage need for republish and thus using the actual SN received instead of
     * available ones. For now it allows the server to avoid keeping them for nothing since no republish will never be
     * requested by us. */
    if (0 < pPubResp->NoOfAvailableSequenceNumbers)
    {
        pSM->bAckSubscr = true;
        /* Only take the last one when more than 1 is available */
        pSM->iAckSeqNum = pPubResp->AvailableSequenceNumbers[pPubResp->NoOfAvailableSequenceNumbers - 1];
    }
    if (pSM->nTokenUsable > 0) // Ensure we do not underflow
    {
        pSM->nTokenUsable -= 1;
    }
    else
    {
        Helpers_Log(SOPC_LOG_LEVEL_WARNING, "Unexpected number of PublishResponse received.");
    }

    /* Traverse the notifications and calls the callback */
    pNotifMsg = &pPubResp->NotificationMessage;
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
                StaMac_ProcessMsg_PubResp_NotifData(pSM, pPubResp, pDataNotif);
            }
            else if (&OpcUa_EventNotificationList_EncodeableType ==
                         pNotifMsg->NotificationData[iNotif].Body.Object.ObjType &&
                     NULL != pSM->pCbkNotification)
            {
                SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == pNotifMsg->NotificationData[iNotif].Encoding);

                // New API compatible only
                OpcUa_EventNotificationList* pEventNotif =
                    (OpcUa_EventNotificationList*) pNotifMsg->NotificationData[iNotif].Body.Object.Value;
                StaMac_ProcessMsg_PubResp_EventNotifList(pSM, pPubResp, pEventNotif);
            }
            else
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Unexpected notification type received %s.",
                            pNotifMsg->NotificationData[iNotif].Body.Object.ObjType->TypeName);
            }
        }
    }
    else if (pSM->pCbkNotification != NULL)
    {
        if (SOPC_IsGoodStatus(pPubResp->ResponseHeader.ServiceResult))
        {
            pPubResp->ResponseHeader.ServiceResult = OpcUa_BadNothingToDo;
        }
        pSM->pCbkNotification(pSM->subscriptionAppCtx, pPubResp->ResponseHeader.ServiceResult, NULL, 0, NULL, NULL);
    }
    /* TODO: verify the results[] which contains a status for each Ack */
}

static void StaMac_ProcessMsg_CreateSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx);

    SOPC_ASSERT(!pSM->bSubscriptionCreated);
    OpcUa_CreateSubscriptionResponse* resp = (OpcUa_CreateSubscriptionResponse*) pParam;
    pSM->iSubscriptionID = resp->SubscriptionId;
    pSM->bSubscriptionCreated = true;
    pSM->subscriptionAppCtx = appCtx;
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Subscription %" PRIu32 " created.", pSM->iSubscriptionID);
    pSM->state = stActivated;
    pSM->fPublishInterval = resp->RevisedPublishingInterval;
    pSM->iCntLifetime = resp->RevisedLifetimeCount;
    pSM->iCntMaxKeepAlive = resp->RevisedMaxKeepAliveCount;
}

static void StaMac_ProcessMsg_DeleteSubscriptionResponse(SOPC_StaMac_Machine* pSM,
                                                         uint32_t arg,
                                                         void* pParam,
                                                         uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(appCtx);

    SOPC_ASSERT(pSM->bSubscriptionCreated);
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
    pSM->iSubscriptionID = 0;
    pSM->bSubscriptionCreated = false;
    pSM->nMonItClientHandle = 0;
    SOPC_SLinkedList_Clear(pSM->pListMonIt);
    SOPC_SLinkedList_Clear(pSM->pListDelMonIt);
    SOPC_SLinkedList_Apply(pSM->dataIdToNodeIdList, SOPC_SLinkedList_EltGenericFree);
    SOPC_SLinkedList_Clear(pSM->dataIdToNodeIdList);
    SOPC_Dict_Delete(pSM->miCliHandleToUserAppCtxDict);
    pSM->miCliHandleToUserAppCtxDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
    // Note: avoid to delete the dict by implementing SOPC_Dict_Clear
    SOPC_ASSERT(NULL != pSM->miCliHandleToUserAppCtxDict);
    SOPC_Dict_SetTombstoneKey(pSM->miCliHandleToUserAppCtxDict, DICT_TOMBSTONE); // Necessary for remove

    SOPC_Dict_Delete(pSM->miIdToCliHandleDict);
    pSM->miIdToCliHandleDict = SOPC_Dict_Create(0, uintptr_hash, direct_equal, NULL, NULL);
    // Note: avoid to delete the dict by implementing SOPC_Dict_Clear
    SOPC_ASSERT(NULL != pSM->miIdToCliHandleDict);
    SOPC_Dict_SetTombstoneKey(pSM->miIdToCliHandleDict, DICT_TOMBSTONE); // Necessary for remove

    // Reset values that will not be used anymore since no more subscription available
    pSM->nTokenUsable = 0;
    pSM->bAckSubscr = false;
    pSM->iAckSeqNum = 0;

    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Subscription deleted.");
    pSM->state = stActivated;
}

static void StaMac_ProcessMsg_CreateMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
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

    for (i = 0; i < pMonItResp->NoOfResults; ++i)
    {
        if (SOPC_IsGoodStatus(pMonItResp->Results[i].StatusCode))
        {
            if (NULL != pMonItReq)
            {
                bool result =
                    SOPC_Dict_Insert(pSM->miIdToCliHandleDict, (uintptr_t) pMonItResp->Results[i].MonitoredItemId,
                                     (uintptr_t) pMonItReq->ItemsToCreate[i].RequestedParameters.ClientHandle);
                if (!result)
                {
                    pMonItResp->Results[i].StatusCode = OpcUa_BadInternalError;
                    Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Internal error creating monitored item with index '%" PRIi32 ".",
                                i);
                }
            }
        }
    }
    if (pMonItResp->NoOfResults > 0)
    {
        bool result =
            SOPC_SLinkedList_Append(pSM->pListMonIt, pMonItResp->Results[0].MonitoredItemId, MIappCtx->outCtxId);
        if (!result)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Internal error creating monitored item result context");
        }
    }

    OpcUa_CreateMonitoredItemsResponse* resp = ((SOPC_CreateMonitoredItems_Ctx*) appCtx)->Results;
    if (NULL != resp)
    {
        // Transfer response data into app context
        *resp = *pMonItResp;
        SOPC_EncodeableObject_Initialize(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, pMonItResp);
    }
    SOPC_Encodeable_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &pMonItReq);
    pSM->state = stActivated;
}

static void StaMac_ProcessMsg_DeleteMonitoredItemsResponse(SOPC_StaMac_Machine* pSM,
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

    for (i = 0; i < pMonItResp->NoOfResults; ++i)
    {
        if (SOPC_IsGoodStatus(pMonItResp->Results[i]))
        {
            bool found = false;
            uintptr_t miCliHandle =
                SOPC_Dict_Get(pSM->miIdToCliHandleDict, (uintptr_t) pMonItReq->MonitoredItemIds[i], &found);
            if (found)
            {
                // Remove internal context associated
                SOPC_Dict_Remove(pSM->miIdToCliHandleDict, (uintptr_t) pMonItReq->MonitoredItemIds[i]);
                SOPC_Dict_Remove(pSM->miCliHandleToUserAppCtxDict, miCliHandle);
            }
            else
            {
                Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Internal error finding monitored item id %" PRIu32,
                            pMonItReq->MonitoredItemIds[i]);
            }
        }
    }
    if (pMonItResp->NoOfResults > 0)
    {
        bool result = SOPC_SLinkedList_Append(pSM->pListDelMonIt, pMonItReq->MonitoredItemIds[0], MIappCtx->outCtxId);
        if (!result)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Internal error creating delete monitored item result context");
        }
    }

    OpcUa_DeleteMonitoredItemsResponse* resp = ((SOPC_DeleteMonitoredItems_Ctx*) appCtx)->Results;
    if (NULL != resp)
    {
        // Transfer response data into app context
        *resp = *pMonItResp;
        SOPC_EncodeableObject_Initialize(&OpcUa_DeleteMonitoredItemsResponse_EncodeableType, pMonItResp);
    }
    SOPC_Encodeable_Delete(&OpcUa_DeleteMonitoredItemsRequest_EncodeableType, (void**) &pMonItReq);
    pSM->state = stActivated;
}

static void StaMac_ProcessMsg_ServiceFault(SOPC_StaMac_Machine* pSM, uint32_t arg, void* pParam, uintptr_t appCtx)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_ARG(pParam);

    if (0 == appCtx)
    {
        pSM->state = stError;
    }
    else
    {
        SOPC_StaMac_ReqCtx reqCtx = *(SOPC_StaMac_ReqCtx*) appCtx;
        SOPC_StaMac_RequestType reqType = reqCtx.requestType;

        switch (reqType)
        {
        case SOPC_REQUEST_TYPE_PUBLISH:
            /* ServiceFault on PublishRequest is allowed */
            if (pSM->nTokenUsable > 0) // Ensure we do not underflow
            {
                pSM->nTokenUsable -= 1;
            }
            else
            {
                Helpers_Log(SOPC_LOG_LEVEL_WARNING, "Unexpected number of PublishResponse received.");
            }
            break;
        default:
            /* else go into error mode */
            pSM->state = stError;
            break;
        }
    }
}

static void StaMac_ProcessEvent_SendRequestFailed(SOPC_StaMac_Machine* pSM,
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
                Helpers_Log(SOPC_LOG_LEVEL_WARNING, "Unexpected number of PublishResponse received.");
            }
        }
        else
        {
            /* else go into error mode */
            pSM->state = stError;
        }
    }
}

static void StaMac_ProcessEvent_stError(SOPC_StaMac_Machine* pSM,
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
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Received post-closed closed event or activation failure, ignored.");
        break;
    case SE_SND_REQUEST_FAILED:
        if (NULL != pParam)
        {
            Helpers_Log(SOPC_LOG_LEVEL_INFO, "Received post-closed send request failure for message of type %s.",
                        ((SOPC_EncodeableType*) pParam)->TypeName);
        }
        else
        {
            Helpers_Log(SOPC_LOG_LEVEL_INFO, "Received post-closed send request failure for message of unknown type");
        }
        break;
    case SE_SESSION_REACTIVATING:
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Reactivating event received, but closed connection are considered lost.");
        break;
    default:
        Helpers_Log(SOPC_LOG_LEVEL_WARNING, "Receiving unexpected event %i in closed state, ignored.", event);
        break;
    }
}

/**
 * \brief Do the post process actions: create a subscription, keep the target number of PublishRequest, ...
 *
 * Machine's mutex shall already be locked by the caller.
 */
static void StaMac_PostProcessActions(SOPC_StaMac_Machine* pSM, SOPC_StaMac_State oldState)
{
    SOPC_ASSERT(NULL != pSM);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    void* pRequest = NULL;

    switch (pSM->state)
    {
    /* Mostly when stActivated is reached */
    case stActivated:
    case stCreatingMonIt:
        /* add tokens, but wait for at least a monitored item */
        if (pSM->bSubscriptionCreated && pSM->nTokenUsable < pSM->nTokenTarget)
        {
            while (SOPC_STATUS_OK == status && pSM->nTokenUsable < pSM->nTokenTarget)
            {
                /* Send a PublishRequest */
                Helpers_Log(SOPC_LOG_LEVEL_INFO, "Adding publish token.");
                status = Helpers_NewPublishRequest(pSM->bAckSubscr, pSM->iSubscriptionID, pSM->iAckSeqNum, &pRequest);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_StaMac_SendRequest(pSM, pRequest, pSM->subscriptionAppCtx,
                                                     SOPC_REQUEST_SCOPE_STATE_MACHINE, SOPC_REQUEST_TYPE_PUBLISH);
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
    /* Try to send a close session if the session was connected before the error */
    case stError:
        if (stError != oldState && stClosing != oldState)
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
                    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Closing the connection because of the previous error.");
                }
            }
            else
            {
                pSM->state = stError;
            }
        }
        break;
    default:
        break;
    }
}
