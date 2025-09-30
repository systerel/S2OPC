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

#include "libs2opc_client.h"
#include "libs2opc_client_internal.h"
#include "libs2opc_common_config.h"

#include "libs2opc_new_client.h"

#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"

#include "state_machine.h"

#include "opcua_statuscodes.h"

/* Number of targeted publish token */
#ifndef SOPC_DEFAULT_PUBLISH_N_TOKEN
#define SOPC_DEFAULT_PUBLISH_N_TOKEN 3
#endif

// TODO: change SM behavior to avoid sleep mechanism (use condition variable instead)
#define CONNECTION_TIMEOUT_MS_STEP 50
struct SOPC_ClientConnection
{
    SOPC_ClientConnectionEvent_Fct* connCb;

    // Synchronous treatment
    SOPC_Condition syncCond;
    SOPC_Mutex syncConnMutex;
    bool syncConnDisconStarted;
    bool syncConnDisconEventRcvd;

    uint16_t secureConnectionIdx;
    bool isDiscovery;
    SOPC_StaMac_Machine* stateMachine; // only if !isDiscovery

    // Reference on client's subscriptions (used in case of Disconnect call)
    SOPC_SLinkedList* subscriptions; // list of SOPC_ClientHelper_Subscription*
    // Reference on client's discovery requests context
    SOPC_SLinkedList* discoveryReqCtxList; // list of SOPC_StaMac_ReqCtx**
};

/* The request context is used to manage
   synchronous/asynchronous context for a request */
typedef struct
{
    uint16_t secureConnectionIdx;

    bool isAsyncCall; /* If call is async the callback is set */
    SOPC_ServiceAsyncResp_Fct* asyncRespCb;
    uintptr_t userCtx;

    SOPC_Mutex mutex; /* protect this context */
    SOPC_Condition condition;
    bool finished; /* set when treatment is done */

    bool isDiscoveryModeService; /* Set if the endpoint service API was used */
    void* responseResultCtx;     /* Context dedicated to the request type */
    SOPC_StatusCode status;      /* Service response status */

} SOPC_ClientHelper_ReqCtx;

static SOPC_ClientHelper_ReqCtx* SOPC_ClientHelperInternal_GenReqCtx_CreateSync(uint16_t secureConnectionIdx,
                                                                                void* responseResultCtx,
                                                                                bool isDiscoveryModeService)
{
    SOPC_ASSERT(NULL != responseResultCtx);

    SOPC_ClientHelper_ReqCtx* result = SOPC_Calloc(1, sizeof(*result));
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL != result)
    {
        result->secureConnectionIdx = secureConnectionIdx;
        // isAsyncCall => already false
        // asyncRespCb => already NULL
        // userCtx => already NULL
        // finished => already false
        result->status = SOPC_STATUS_NOK;
        result->isDiscoveryModeService = isDiscoveryModeService;
        result->responseResultCtx = responseResultCtx;
        status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Mutex_Initialization(&result->mutex);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Condition_Init(&result->condition);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Condition_Clear(&result->condition);
        SOPC_Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

static SOPC_ClientHelper_ReqCtx* SOPC_ClientHelperInternal_GenReqCtx_CreateAsync(uint16_t secureConnectionIdx,
                                                                                 bool isDiscoveryModeService,
                                                                                 SOPC_ServiceAsyncResp_Fct* asyncRespCb,
                                                                                 uintptr_t userContext)
{
    SOPC_ASSERT(NULL != asyncRespCb);

    SOPC_ClientHelper_ReqCtx* result = SOPC_Calloc(1, sizeof(*result));
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL != result)
    {
        result->secureConnectionIdx = secureConnectionIdx;
        result->isAsyncCall = true;
        result->asyncRespCb = asyncRespCb;
        result->userCtx = userContext;
        // finished => already false
        result->status = SOPC_STATUS_NOK;
        result->isDiscoveryModeService = isDiscoveryModeService;
        // responseResultCtx => already NULL
        status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Mutex_Initialization(&result->mutex);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Condition_Init(&result->condition);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Condition_Clear(&result->condition);
        SOPC_Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

// Create a request context for unicity which will not be used for synchronizing response by client helper
// (for temporary subscription specific behavior which is partially managed by SM)
static SOPC_ClientHelper_ReqCtx* SOPC_ClientHelperInternal_GenReqCtx_CreateNoSync(uint16_t secureConnectionIdx,
                                                                                  uintptr_t userContext)
{
    SOPC_ClientHelper_ReqCtx* result = SOPC_Calloc(1, sizeof(*result));
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL != result)
    {
        result->secureConnectionIdx = secureConnectionIdx;
        // isAsyncCall => already false
        // asyncRespCb => already NULL
        result->userCtx = userContext;
        // finished => already false
        // status => already SOPC_STATUS_OK
        // isDiscoveryModeService => already false
        // responseResultCtx => already NULL
        status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Mutex_Initialization(&result->mutex);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Condition_Init(&result->condition);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Condition_Clear(&result->condition);
        SOPC_Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

static void SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(SOPC_ClientHelper_ReqCtx* genReqCtx)
{
    SOPC_ASSERT(NULL != genReqCtx);
    SOPC_Condition_Clear(&genReqCtx->condition);
    SOPC_Mutex_Clear(&genReqCtx->mutex);
    /* Avoid "warning: potential null pointer dereference [-Werror=null-dereference]"
     * genReqCtx is not NULL, this is checked by SOPC_ASSERT at the start of this function*/
    SOPC_GCC_DIAGNOSTIC_PUSH
    SOPC_GCC_DIAGNOSTIC_IGNORE_POTENTIAL_NULL_POINTER_DEREF
    genReqCtx->responseResultCtx = NULL; // shall be freed by caller or previously
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_Free(genReqCtx);
}

// Wait for condition signaled and finished flag set
// When returns OK, operationStatus is set to the global service response status
static SOPC_ReturnStatus SOPC_ClientHelperInternal_GenReqCtx_WaitFinishedOrTimeout(SOPC_ClientHelper_ReqCtx* genReqCtx)
{
    SOPC_ASSERT(NULL != genReqCtx);
    SOPC_ReturnStatus mutStatus = SOPC_STATUS_OK;
    /* Wait for the response */
    while (SOPC_STATUS_OK == mutStatus && !genReqCtx->finished)
    {
        // Note: limit to 2*SOPC_REQUEST_TIMEOUT_MS since a request timeout is at least expected after
        // SOPC_REQUEST_TIMEOUT_MS. It guarantees we will not
        mutStatus =
            SOPC_Mutex_UnlockAndTimedWaitCond(&genReqCtx->condition, &genReqCtx->mutex, 2 * SOPC_REQUEST_TIMEOUT_MS);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus || SOPC_STATUS_TIMEOUT == mutStatus);
    }
    return (SOPC_STATUS_OK == mutStatus ? genReqCtx->status : SOPC_STATUS_TIMEOUT);
}

static bool SOPC_ClientHelperInternal_CheckConnectionValid(const SOPC_S2OPC_Config* config,
                                                           const SOPC_SecureConnection_Config* secConnConfig)
{
    if (config->clientConfig.secureConnections[secConnConfig->secureConnectionIdx] != secConnConfig)
    {
        return false;
    }
    return true;
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_MayFinalizeSecureConnection(
    SOPC_S2OPC_Config* config,
    SOPC_SecureConnection_Config* secConnConfig)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // If configuration is not finalized yet, do it now
    if (!secConnConfig->finalized)
    {
        SOPC_Client_Config* cConfig = &config->clientConfig;
        status = SOPC_ClientConfigHelper_Finalize_SecureConnectionConfig(cConfig, secConnConfig);
    }
    return status;
}

static void SOPC_ClientInternal_EventCbk(uint32_t c_id,
                                         SOPC_StaMac_ApplicativeEvent event,
                                         SOPC_StatusCode status, /* Note: actually a ReturnStatus */
                                         const void* response,
                                         uintptr_t genContext)
{
    SOPC_UNUSED_ARG(c_id); // managed by caller

    bool isAsync = false;
    SOPC_ClientHelper_ReqCtx* genCtx = (SOPC_ClientHelper_ReqCtx*) genContext;
    SOPC_ReturnStatus statusMutex = SOPC_Mutex_Lock(&genCtx->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

    if (genCtx->isAsyncCall)
    {
        isAsync = true;
        SOPC_EncodeableType* pEncType = NULL;
        if (SOPC_StaMac_ApplicativeEvent_Response == event)
        {
            pEncType = *(SOPC_EncodeableType* const*) response;
        }
        genCtx->asyncRespCb(pEncType, response, genCtx->userCtx);
    }
    else
    {
        void* responseContext = genCtx->responseResultCtx;
        SOPC_ASSERT(NULL != responseContext);
        if (SOPC_StaMac_ApplicativeEvent_Response == event)
        {
            SOPC_EncodeableType* pEncType = *(SOPC_EncodeableType* const*) response;

            void** genResponseContext = (void**) responseContext;

            status = SOPC_EncodeableObject_Create(pEncType, genResponseContext);
            if (SOPC_STATUS_OK == status)
            {
                SOPC_ASSERT(NULL != *genResponseContext);
                // Move response to application context
                *genResponseContext = memcpy(*genResponseContext, response, pEncType->AllocationSize);
                // Avoid dealloc by caller by resetting content of provided response
                SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
                SOPC_EncodeableObject_Initialize(pEncType, (void*) response);
                SOPC_GCC_DIAGNOSTIC_RESTORE
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "SOPC_ClientInternal_EventCbk: unexpected error for %s creation",
                                       pEncType->TypeName);
            }
        } // else: response is NULL and status is not OK
    }
    genCtx->status = status;

    genCtx->finished = true;
    statusMutex = SOPC_Mutex_Unlock(&genCtx->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);
    /* Signal that the response is available */
    statusMutex = SOPC_Condition_SignalAll(&genCtx->condition);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

    if (isAsync)
    {
        // Context is not necessary anymore
        SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(genCtx);
    }
}

static void SOPC_ClientInternal_ConnectionStateCallback(SOPC_App_Com_Event event,
                                                        void* param,
                                                        SOPC_ClientConnection* cc)
{
    SOPC_ASSERT(NULL != cc);
    if (SE_CLOSED_SESSION == event || SE_SESSION_ACTIVATION_FAILURE == event || SE_SESSION_REACTIVATING == event ||
        SE_ACTIVATED_SESSION == event)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        bool unlockedMutex = false;

        SOPC_ReturnStatus statusMutex = SOPC_Mutex_Lock(&cc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        bool isSyncConnDiscon = cc->syncConnDisconStarted;
        if (isSyncConnDiscon)
        {
            cc->syncConnDisconEventRcvd = true;
        }

        SOPC_Mutex_Unlock(&cc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSyncConnDiscon)
        {
            /* Synchronous connection operation:
               Signal that the response is available */
            statusMutex = SOPC_Condition_SignalAll(&cc->syncCond);
            SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);
        }
        else
        {
            /* Unexpected connection event
               or asynchronous connection operation response (NOT IMPLEMENTED YET) */
            SOPC_ClientConnectionEvent connEvent;
            SOPC_StatusCode serviceStatus = SOPC_GoodGenericStatus;
            switch (event)
            {
            case SE_ACTIVATED_SESSION:
                connEvent = SOPC_ClientConnectionEvent_Connected;
                break;
            case SE_SESSION_ACTIVATION_FAILURE:
                connEvent = SOPC_ClientConnectionEvent_Disconnected;
                serviceStatus =
                    (SOPC_StatusCode)(uintptr_t) param; // TODO: casting void to unintptr is not legit, only reverse is
                break;
            case SE_SESSION_REACTIVATING:
                connEvent = SOPC_ClientConnectionEvent_Disconnected; // TODO: reconnecting when managed by SM
                serviceStatus = OpcUa_BadWouldBlock;
                break;
            case SE_CLOSED_SESSION:
                connEvent = SOPC_ClientConnectionEvent_Disconnected;
                serviceStatus =
                    (SOPC_StatusCode)(uintptr_t) param; // TODO: casting void to unintptr is not legit, only reverse is
                break;
            default:
                SOPC_ASSERT(false);
                return;
            }
            // Release mutex to avoid possible deadlock in user callback
            mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            unlockedMutex = true;
            cc->connCb(cc, connEvent, serviceStatus);
        }
        if (!unlockedMutex)
        {
            mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
    }
}

void SOPC_ClientInternal_ToolkitEventCallback(SOPC_App_Com_Event event,
                                              uint32_t IdOrStatus,
                                              void* param,
                                              uintptr_t appContext)
{
    SOPC_ClientConnection* cc = NULL;
    SOPC_StaMac_Machine* pSM = NULL;

    if (!SOPC_ClientInternal_IsInitialized())
    {
        return;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bool unlockedMutex = false;

    SOPC_StaMac_ReqCtx* staMacCtx = NULL;
    SOPC_ClientHelper_ReqCtx* serviceReqCtx = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    switch (event)
    {
    /* appCtx is request context */
    case SE_RCV_SESSION_RESPONSE:
    case SE_RCV_DISCOVERY_RESPONSE:
    case SE_SND_REQUEST_FAILED:
        SOPC_ASSERT(0 != appContext);
        staMacCtx = (SOPC_StaMac_ReqCtx*) appContext;
        if (SOPC_REQUEST_SCOPE_STATE_MACHINE != staMacCtx->requestScope)
        {
            serviceReqCtx = (SOPC_ClientHelper_ReqCtx*) staMacCtx->appCtx;
            cc = sopc_client_helper_config.secureConnections[serviceReqCtx->secureConnectionIdx];
        }
        break;
    /* appCtx session related event context is connection index */
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_ACTIVATED_SESSION:
    case SE_SESSION_REACTIVATING:
    case SE_CLOSED_SESSION:
        cc = sopc_client_helper_config.secureConnections[appContext];
        break;
    /* IdOrStatus is reverse endpoint config index */
    case SE_REVERSE_ENDPOINT_CLOSED:
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Reverse endpoint '%s' closed",
                              SOPC_ToolkitClient_GetReverseEndpointURL(IdOrStatus));
        sopc_client_helper_config
            .openedReverseEndpointsFromCfgIdx[SOPC_ClientInternal_GetReverseEPcfgIdxNoOffset(IdOrStatus)] = false;
        mutStatus = SOPC_Condition_SignalAll(&sopc_client_helper_config.reverseEPsClosedCond);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        break;
    default:
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ClientInternal_ToolkitEventCallback: unexpected event %d received.", event);
        status = SOPC_STATUS_NOT_SUPPORTED;
        break;
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != serviceReqCtx && serviceReqCtx->isDiscoveryModeService)
        {
            // Discovery service call (no session) not managed by state machine: direct call to event callback
            SOPC_StaMac_ApplicativeEvent libsubEvent = SOPC_StaMac_ApplicativeEvent_Response;
            if (SE_RCV_DISCOVERY_RESPONSE != event)
            {
                status = SOPC_STATUS_NOK;
                libsubEvent = SOPC_StaMac_ApplicativeEvent_SendFailed;
                // set param to NULL to keep same behavior as non discovery service failure
                param = NULL;
            }
            // Release mutex to avoid possible deadlock in user callback
            mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            unlockedMutex = true;
            SOPC_ClientInternal_EventCbk(serviceReqCtx->secureConnectionIdx, libsubEvent, status, param,
                                         (uintptr_t) serviceReqCtx);
            uintptr_t removedContext = SOPC_SLinkedList_RemoveFromValuePtr(cc->discoveryReqCtxList, appContext);
            SOPC_UNUSED_RESULT(removedContext == appContext);
            SOPC_Free((void*) appContext);
        }
        else if (event !=
                 SE_REVERSE_ENDPOINT_CLOSED) // state machine does not manage reverse EPs (excluded from treatment)
        {
            if (NULL != staMacCtx)
            {
                // Service request management provides state machine
                pSM = staMacCtx->pSM;
            }
            else if (NULL != cc)
            {
                // Connection management event: retrieve state machine from connection context
                pSM = sopc_client_helper_config.secureConnections[cc->secureConnectionIdx]->stateMachine;
            }
            if (NULL != pSM)
            {
                // Release mutex to avoid possible deadlock in user callback
                mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                unlockedMutex = true;
                // Call state machine callback for event treatment
                if (SOPC_StaMac_EventDispatcher(pSM, NULL, event, IdOrStatus, param, appContext) && NULL != cc)
                {
                    /* Post process the event in case of connection management events */
                    SOPC_ClientInternal_ConnectionStateCallback(event, param, cc);
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ClientInternal_ToolkitEventCallback: unexpected context received for event %d.",
                                       event);
            }
        }
    }

    if (!unlockedMutex)
    {
        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_CreateClientConnection(
    const SOPC_SecureConnection_Config* secConnConfig,
    bool isDiscovery,
    SOPC_ClientConnection** outClientConnection)
{
    SOPC_ASSERT(secConnConfig != NULL);
    SOPC_ASSERT(outClientConnection != NULL);

    SOPC_ClientConnection* res = SOPC_Calloc(1, sizeof(*res));
    SOPC_StaMac_Machine* stateMachine = NULL;

    if (NULL == res)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Note: still create state machine even in discovery since it might be used for non-discovery later
    const char* username = NULL;
    const char* password = NULL;

    const SOPC_SerializedCertificate* pUserCertX509 = NULL;
    const SOPC_SerializedAsymmetricKey* pUserKey = NULL;

    if (secConnConfig->sessionConfig.userTokenType == OpcUa_UserTokenType_UserName)
    {
        username = secConnConfig->sessionConfig.userToken.userName.userName;
        password = secConnConfig->sessionConfig.userToken.userName.userPwd;
    }
    else if (secConnConfig->sessionConfig.userTokenType == OpcUa_UserTokenType_Certificate)
    {
        pUserCertX509 = secConnConfig->sessionConfig.userToken.userX509.certX509;
        pUserKey = secConnConfig->sessionConfig.userToken.userX509.keyX509;
    }

    SOPC_ReturnStatus status = SOPC_StaMac_Create(
        secConnConfig->secureChannelConfigIdx, secConnConfig->reverseEndpointConfigIdx,
        secConnConfig->secureConnectionIdx, secConnConfig->sessionConfig.userPolicyId, username, password,
        pUserCertX509, pUserKey, SOPC_DEFAULT_PUBLISH_N_TOKEN, SOPC_ClientInternal_EventCbk, 0, &stateMachine);

    if (SOPC_STATUS_OK == status)
    {
        // Data for synchronous calls
        SOPC_ReturnStatus mutStatus = SOPC_Condition_Init(&res->syncCond);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        mutStatus = SOPC_Mutex_Initialization(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        res->secureConnectionIdx = secConnConfig->secureConnectionIdx;
        res->isDiscovery = isDiscovery;
        res->stateMachine = stateMachine;
        res->subscriptions = SOPC_SLinkedList_Create(0);
        res->discoveryReqCtxList = SOPC_SLinkedList_Create(0);
        if (NULL == res->subscriptions)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *outClientConnection = res;
    }
    else
    {
        SOPC_Free(res);
    }

    return status;
}

static void SOPC_ClientHelperInternal_ClearDiscoveryReqCtx(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_StaMac_ReqCtx* reqCtx = (SOPC_StaMac_ReqCtx*) val;
    SOPC_ASSERT(NULL != reqCtx);
    SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree((SOPC_ClientHelper_ReqCtx*) reqCtx->appCtx);
    SOPC_Free(reqCtx);
}

static void SOPC_ClientHelperInternal_ClearClientConnection(SOPC_ClientConnection* connection)
{
    if (NULL != connection)
    {
        SOPC_ReturnStatus mutStatus;

        SOPC_StaMac_Delete(&connection->stateMachine);
        if (SOPC_INVALID_COND != connection->syncCond)
        {
            mutStatus = SOPC_Condition_Clear(&connection->syncCond);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
        if (SOPC_INVALID_MUTEX != connection->syncConnMutex)
        {
            mutStatus = SOPC_Mutex_Clear(&connection->syncConnMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
        if (NULL != connection->subscriptions)
        {
            SOPC_SLinkedList_Delete(connection->subscriptions);
            connection->subscriptions = NULL;
        }

        if (NULL != connection->discoveryReqCtxList)
        {
            // Clear discovery requests
            SOPC_SLinkedList_Apply(connection->discoveryReqCtxList, SOPC_ClientHelperInternal_ClearDiscoveryReqCtx);
            SOPC_SLinkedList_Delete(connection->discoveryReqCtxList);
            connection->discoveryReqCtxList = NULL;
        }
        sopc_client_helper_config.secureConnections[connection->secureConnectionIdx] = NULL;
        SOPC_Free(connection);
    }
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_DiscoveryService(bool isSynchronous,
                                                                    SOPC_SecureConnection_Config* secConnConfig,
                                                                    void* request,
                                                                    void** response,
                                                                    uintptr_t userContext,
                                                                    SOPC_ServiceAsyncResp_Fct* asyncRespCb)
{
    bool requestSentToServices = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ClientConnection* res = NULL;
    SOPC_ClientHelper_ReqCtx* reqCtx = NULL;
    SOPC_StaMac_ReqCtx* smReqCtx = NULL;
    SOPC_ServiceAsyncResp_Fct* customAsyncRespCb = asyncRespCb;
    if (NULL == customAsyncRespCb)
    {
        customAsyncRespCb = sopc_client_helper_config.asyncRespCb;
    }
    if (NULL == secConnConfig || NULL == request || (isSynchronous && NULL == response))
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        // Check connection is valid and is not already created
        if (!SOPC_ClientHelperInternal_CheckConnectionValid(pConfig, secConnConfig) ||
            (!isSynchronous && NULL == customAsyncRespCb))
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientHelperInternal_MayFinalizeSecureConnection(pConfig, secConnConfig);
        }
        if (SOPC_STATUS_OK == status)
        {
            res = sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx];
        }
        if (SOPC_STATUS_OK == status && NULL == res)
        {
            status = SOPC_ClientHelperInternal_CreateClientConnection(secConnConfig, true, &res);
            if (SOPC_STATUS_OK == status)
            {
                sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx] = res;
            }
        }

        /* Call discovery service */
        if (SOPC_STATUS_OK == status)
        {
            /* create a context wrapper (use SOPC_StaMac_ReqCtx for unicity with other responses but SM not called) */

            smReqCtx = SOPC_Calloc(1, sizeof(*smReqCtx));
            if (isSynchronous)
            {
                reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateSync(res->secureConnectionIdx, response, true);
            }
            else
            {
                reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateAsync(res->secureConnectionIdx, true,
                                                                         customAsyncRespCb, userContext);
            }
            if (NULL == smReqCtx || NULL == reqCtx)
            {
                SOPC_Free(smReqCtx);
                SOPC_Free(reqCtx);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                smReqCtx->appCtx = (uintptr_t) reqCtx;
                smReqCtx->requestScope = SOPC_REQUEST_SCOPE_DISCOVERY;
                smReqCtx->requestType = SOPC_REQUEST_TYPE_GET_ENDPOINTS;
            }
        }

        if (SOPC_STATUS_OK != status)
        {
            SOPC_ClientHelperInternal_ClearClientConnection(res);
        }
        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    /* send the request and wait for the result if sync operation */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus statusMutex = SOPC_Mutex_Lock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        /* send the request */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_EndpointConnectionCfg endpointConnectionCfg = {
                .reverseEndpointConfigIdx = secConnConfig->reverseEndpointConfigIdx,
                .secureChannelConfigIdx = secConnConfig->secureChannelConfigIdx};

            status = SOPC_ToolkitClient_AsyncSendDiscoveryRequest(endpointConnectionCfg, request, (uintptr_t) smReqCtx);
            if (SOPC_STATUS_OK == status)
            {
                requestSentToServices = true;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            uintptr_t addedCtx = SOPC_SLinkedList_Append(res->discoveryReqCtxList, 0, (uintptr_t) smReqCtx);
            if (addedCtx != (uintptr_t) smReqCtx)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Unable to add discovery request context to list of connectionConfigIdx %" PRIu16,
                    secConnConfig->secureConnectionIdx);
            }
        }

        if (isSynchronous && SOPC_STATUS_OK == status)
        {
            /* Wait for the response => status OK (response received) */
            status = SOPC_ClientHelperInternal_GenReqCtx_WaitFinishedOrTimeout(reqCtx);

            if (SOPC_STATUS_OK == status)
            {
                status = reqCtx->status;
            }
        }

        statusMutex = SOPC_Mutex_Unlock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSynchronous && NULL != reqCtx)
        {
            SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
        }
    }
    if (!requestSentToServices && NULL != request)
    {
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) request, &request);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                          void* request,
                                                          uintptr_t userContext)
{
    return SOPC_ClientHelperInternal_DiscoveryService(false, secConnConfig, request, NULL, userContext, NULL);
}

SOPC_ReturnStatus SOPC_ClientHelper_DiscoveryServiceAsyncCustom(SOPC_SecureConnection_Config* secConnConfig,
                                                                void* request,
                                                                uintptr_t userContext,
                                                                SOPC_ServiceAsyncResp_Fct* asyncRespCb)
{
    return SOPC_ClientHelperInternal_DiscoveryService(false, secConnConfig, request, NULL, userContext, asyncRespCb);
}

SOPC_ReturnStatus SOPC_ClientHelper_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                         void* request,
                                                         void** response)
{
    return SOPC_ClientHelperInternal_DiscoveryService(true, secConnConfig, request, response, 0, NULL);
}

SOPC_ReturnStatus SOPC_ClientHelper_Connect(SOPC_SecureConnection_Config* secConnConfig,
                                            SOPC_ClientConnectionEvent_Fct* connectEventCb,
                                            SOPC_ClientConnection** secureConnection)
{
    if (NULL == secConnConfig || NULL == connectEventCb || NULL == secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Check connection is valid and is not already created
    SOPC_ClientConnection* res = sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx];
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (!SOPC_ClientHelperInternal_CheckConnectionValid(pConfig, secConnConfig))
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperInternal_MayFinalizeSecureConnection(pConfig, secConnConfig);
    }

    if (SOPC_STATUS_OK == status && NULL != res)
    {
        if (res->isDiscovery)
        {
            // The only case authorized for an existing connection is a discovery connection which is also valid for
            // non-discovery (<=> !secConnConfig->isDiscoveryConnection)
            res->isDiscovery = false;
        }
        else
        {
            // Connect already called, do not allow a new connect call
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    if (SOPC_STATUS_OK == status && NULL == res)
    {
        status = SOPC_ClientHelperInternal_CreateClientConnection(secConnConfig, false, &res);
        if (SOPC_STATUS_OK == status)
        {
            sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx] = res;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        res->connCb = connectEventCb;
    }

    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Starts the machine */
    if (SOPC_STATUS_OK == status)
    {
        mutStatus = SOPC_Mutex_Lock(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (!res->syncConnDisconStarted)
        {
            // Start connection events inhibition for the user application callback
            res->syncConnDisconStarted = true;
            status = SOPC_StaMac_StartSession(res->stateMachine);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            // note: we wait for connection event received by SOPC_ClientInternal_ConnectionStateCallback
            //       and until expected state machine state change occurred
            while (SOPC_STATUS_OK == status &&
                   (!res->syncConnDisconEventRcvd ||
                    (!SOPC_StaMac_IsError(res->stateMachine) && !SOPC_StaMac_IsConnected(res->stateMachine))))
            {
                // Reset received event flag as it was not the one expected (we are still NOT in connected state)
                res->syncConnDisconEventRcvd = false;
                // Note: we rely on the low layer timeouts and do not need a new one
                status = SOPC_Mutex_UnlockAndWaitCond(&res->syncCond, &res->syncConnMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
            }

            if (SOPC_StaMac_IsError(res->stateMachine) || !SOPC_StaMac_IsConnected(res->stateMachine))
            {
                status = SOPC_STATUS_CLOSED;
            }
        }

        // End connection events inhibition for the user application callback + reset event received flag
        res->syncConnDisconStarted = false;
        res->syncConnDisconEventRcvd = false;

        mutStatus = SOPC_Mutex_Unlock(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        *secureConnection = res;
    }
    else
    {
        SOPC_ClientHelperInternal_ClearClientConnection(res);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_Disconnect(SOPC_ClientConnection** secureConnection)
{
    if (NULL == secureConnection || NULL == *secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ClientConnection* pSc = *secureConnection;

    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_StaMac_Machine* pSM = NULL;

    SOPC_SLinkedList* subscriptions = NULL;

    if (pSc == sopc_client_helper_config.secureConnections[pSc->secureConnectionIdx])
    {
        status = SOPC_STATUS_OK;
        pSM = pSc->stateMachine;
        subscriptions = pSc->subscriptions;
    } // else: SOPC_STATUS_INVALID_STATE;

    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Early return after mutex unlock
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // Delete the subscriptions if there is still at least one created
    if (subscriptions != NULL)
    {
        // Store the subs in an array
        uint32_t nbrOfSubs = SOPC_SLinkedList_GetLength(subscriptions);
        uint32_t nbrOfSubsIndex = 0;
        SOPC_ClientHelper_Subscription** subsToDelete = SOPC_Calloc(nbrOfSubs, sizeof(SOPC_ClientHelper_Subscription*));
        SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(subscriptions);
        SOPC_ClientHelper_Subscription* sub = (SOPC_ClientHelper_Subscription*) SOPC_SLinkedList_Next(&it);
        while (NULL != sub)
        {
            SOPC_ASSERT(nbrOfSubsIndex < nbrOfSubs);
            subsToDelete[nbrOfSubsIndex] = sub;
            nbrOfSubsIndex++;
            sub = (SOPC_ClientHelper_Subscription*) SOPC_SLinkedList_Next(&it);
        }

        // Delete them
        for (uint32_t i = 0; i < nbrOfSubs; i++)
        {
            SOPC_ReturnStatus localStatus = SOPC_ClientHelper_DeleteSubscription(&subsToDelete[i]);
            SOPC_UNUSED_RESULT(localStatus); // We still want to disconnect the client connection anyway
        }
        SOPC_Free(subsToDelete);
    }

    if (SOPC_StaMac_IsConnected(pSM))
    {
        mutStatus = SOPC_Mutex_Lock(&pSc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (!pSc->syncConnDisconStarted)
        {
            pSc->syncConnDisconStarted = true;
            status = SOPC_StaMac_StopSession(pSM);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        // note: we wait for connection event received by SOPC_ClientInternal_ConnectionStateCallback
        //       and until expected state machine state change occurred
        while (SOPC_STATUS_OK == status &&
               (!pSc->syncConnDisconEventRcvd || SOPC_StaMac_IsConnected(pSc->stateMachine)))
        {
            // Reset received event flag as it was not the one expected (we are still in connected state)
            pSc->syncConnDisconEventRcvd = false;
            // Note: we use the low layer timeouts and do not need a new one
            status = SOPC_Mutex_UnlockAndWaitCond(&pSc->syncCond, &pSc->syncConnMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }

        // End connection events inhibition for the user application callback + reset event received flag
        pSc->syncConnDisconStarted = false;
        pSc->syncConnDisconEventRcvd = false;

        mutStatus = SOPC_Mutex_Unlock(&pSc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        SOPC_ClientHelperInternal_ClearClientConnection(pSc);

        *secureConnection = NULL;

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    return status;
}

// Used to filter requests in generic services functions:
// for now forbids acting on the SM subscription managed internally
static SOPC_ReturnStatus SOPC_ClientHelperInternal_FilterService(SOPC_ClientConnection* secureConnection, void* request)
{
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) request;
    if (&OpcUa_ModifySubscriptionRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_ModifySubscriptionRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (&OpcUa_SetPublishingModeRequest_EncodeableType == encType)
    {
        for (int32_t i = 0; i < ((OpcUa_SetPublishingModeRequest*) request)->NoOfSubscriptionIds; i++)
        {
            if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                              ((OpcUa_SetPublishingModeRequest*) request)->SubscriptionIds[i]))
            {
                return SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    } // Note: Publish and Republish are not associated to the subscription but the session
      // => do not filter (app responsibilty not to use it)
    else if (&OpcUa_TransferSubscriptionsRequest_EncodeableType == encType)
    {
        for (int32_t i = 0; i < ((OpcUa_TransferSubscriptionsRequest*) request)->NoOfSubscriptionIds; i++)
        {
            if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                              ((OpcUa_TransferSubscriptionsRequest*) request)->SubscriptionIds[i]))
            {
                return SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }
    else if (&OpcUa_DeleteSubscriptionsRequest_EncodeableType == encType)
    {
        for (int32_t i = 0; i < ((OpcUa_DeleteSubscriptionsRequest*) request)->NoOfSubscriptionIds; i++)
        {
            if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                              ((OpcUa_DeleteSubscriptionsRequest*) request)->SubscriptionIds[i]))
            {
                return SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }
    else if (&OpcUa_CreateMonitoredItemsRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_CreateMonitoredItemsRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (&OpcUa_ModifyMonitoredItemsRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_ModifyMonitoredItemsRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (&OpcUa_SetMonitoringModeRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_SetMonitoringModeRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (&OpcUa_SetTriggeringRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_SetTriggeringRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else if (&OpcUa_DeleteMonitoredItemsRequest_EncodeableType == encType)
    {
        if (SOPC_StaMac_HasSubscriptionId(secureConnection->stateMachine,
                                          ((OpcUa_DeleteMonitoredItemsRequest*) request)->SubscriptionId))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_Service(bool isSynchronous,
                                                           SOPC_ClientConnection* secureConnection,
                                                           void* request,
                                                           void** response,
                                                           uintptr_t userContext,
                                                           SOPC_ServiceAsyncResp_Fct* asyncRespCb)
{
    SOPC_StaMac_Machine* pSM = NULL;
    SOPC_ClientHelper_ReqCtx* reqCtx = NULL;

    bool requestSentToServices = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ServiceAsyncResp_Fct* customAsyncRespCb = asyncRespCb;
    if (NULL == customAsyncRespCb)
    {
        customAsyncRespCb = sopc_client_helper_config.asyncRespCb;
    }
    if (NULL == secureConnection || NULL == request)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (isSynchronous && (NULL == response || NULL != asyncRespCb))
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        status = SOPC_ClientHelperInternal_FilterService(secureConnection, request);

        if (secureConnection != sopc_client_helper_config.secureConnections[secureConnection->secureConnectionIdx] ||
            (!isSynchronous && NULL == customAsyncRespCb))
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        /* Call service */
        if (SOPC_STATUS_OK == status)
        {
            pSM = secureConnection->stateMachine;
            /* create a request context */
            if (isSynchronous)
            {
                reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateSync(secureConnection->secureConnectionIdx, response,
                                                                        false);
            }
            else
            {
                reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateAsync(secureConnection->secureConnectionIdx, false,
                                                                         customAsyncRespCb, userContext);
            }
            if (NULL == reqCtx)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    /* send the request and wait for the result if sync operation */
    if (SOPC_STATUS_OK == status)
    {
        /* Prepare the synchronous context */
        SOPC_ReturnStatus statusMutex = SOPC_Mutex_Lock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        /* send the request */

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, request, (uintptr_t) reqCtx, SOPC_REQUEST_SCOPE_APPLICATION,
                                             SOPC_REQUEST_TYPE_USER);
            requestSentToServices = (SOPC_STATUS_OK == status);
        }

        if (isSynchronous && SOPC_STATUS_OK == status)
        {
            /* Wait for the response => status OK (response received) */
            status = SOPC_ClientHelperInternal_GenReqCtx_WaitFinishedOrTimeout(reqCtx);

            if (SOPC_STATUS_OK == status)
            {
                status = reqCtx->status;
            }
        }

        statusMutex = SOPC_Mutex_Unlock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSynchronous && NULL != reqCtx)
        {
            SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
        }
    }
    if (!requestSentToServices && NULL != request)
    {
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) request, &request);
    }
    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                 void* request,
                                                 uintptr_t userContext)
{
    return SOPC_ClientHelperInternal_Service(false, secureConnection, request, NULL, userContext, NULL);
}

SOPC_ReturnStatus SOPC_ClientHelper_ServiceAsyncCustom(SOPC_ClientConnection* secureConnection,
                                                       void* request,
                                                       uintptr_t userContext,
                                                       SOPC_ServiceAsyncResp_Fct* asyncRespCb)
{
    return SOPC_ClientHelperInternal_Service(false, secureConnection, request, NULL, userContext, asyncRespCb);
}

SOPC_ReturnStatus SOPC_ClientHelper_ServiceSync(SOPC_ClientConnection* secureConnection, void* request, void** response)
{
    return SOPC_ClientHelperInternal_Service(true, secureConnection, request, response, 0, NULL);
}

struct SOPC_ClientHelper_Subscription
{
    SOPC_ClientConnection* secureConnection;
    uint32_t subscriptionId;
    uintptr_t userParam;

    SOPC_ClientSubscriptionNotification_Fct* subNotifCb;
};

static void SOPC_StaMacNotification_Cbk(uintptr_t subscriptionAppCtx,
                                        SOPC_StatusCode status,
                                        SOPC_EncodeableType* notificationType,
                                        uint32_t nbNotifElts,
                                        const void* notification,
                                        uintptr_t* monitoredItemCtxArray)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return;
    }
    if (NULL != (SOPC_ClientHelper_ReqCtx*) subscriptionAppCtx)
    {
        SOPC_ClientHelper_ReqCtx* subCtx = (SOPC_ClientHelper_ReqCtx*) subscriptionAppCtx;
        SOPC_ClientHelper_Subscription* subInst = (SOPC_ClientHelper_Subscription*) subCtx->userCtx;
        if (NULL != subInst && NULL != subInst->subNotifCb)
        {
            subInst->subNotifCb(subInst, status, notificationType, nbNotifElts, notification, monitoredItemCtxArray);
        }
    }
}

SOPC_ClientHelper_Subscription* SOPC_ClientHelper_CreateSubscription(
    SOPC_ClientConnection* secureConnection,
    OpcUa_CreateSubscriptionRequest* subParams,
    SOPC_ClientSubscriptionNotification_Fct* subNotifCb,
    uintptr_t userParam)
{
    bool requestInStaMac = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ClientHelper_Subscription* subInstance = NULL;

    if (NULL == secureConnection || NULL == subParams || NULL == subNotifCb)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        SOPC_StaMac_Machine* pSM = NULL;

        if (secureConnection != sopc_client_helper_config.secureConnections[secureConnection->secureConnectionIdx])
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        subInstance = SOPC_Calloc(1, sizeof(*subInstance));
        if (NULL == subInstance)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            subInstance->subNotifCb = subNotifCb;
        }
        /* Get SM and configure notification CB if not configured yet */
        if (SOPC_STATUS_OK == status)
        {
            pSM = secureConnection->stateMachine;
            // Define the state machine notif callback only if it has no active sub
            if (!SOPC_StaMac_HasAnySubscription(pSM))
            {
                status = SOPC_StaMac_NewConfigureNotificationCallback(pSM, SOPC_StaMacNotification_Cbk);
            }
        }
        /* Create the unified context for client helper layer */
        SOPC_ClientHelper_ReqCtx* reqCtx = NULL;
        if (SOPC_STATUS_OK == status)
        {
            reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateNoSync(secureConnection->secureConnectionIdx,
                                                                      (uintptr_t) subInstance);
            status = (NULL != reqCtx) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* Create the subscription */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_NewCreateSubscription(pSM, subParams, (uintptr_t) reqCtx);
            requestInStaMac = true;
        }

        /* Release the lock so that the event handler can work properly while waiting */

        /* Wait for the subscription to be created */
        if (SOPC_STATUS_OK == status)
        {
            int count = 0;
            while (!SOPC_StaMac_IsError(pSM) &&
                   SOPC_StaMac_IsSubscriptionInProgress(pSM) && // Wait for the new sub to be created
                   count * CONNECTION_TIMEOUT_MS_STEP < SOPC_REQUEST_TIMEOUT_MS)
            {
                mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);

                mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                ++count;
            }
            if (SOPC_StaMac_IsSubscriptionInProgressCreated(pSM))
            {
                subInstance->secureConnection = secureConnection;
                subInstance->subscriptionId = SOPC_StaMac_GetLatestSubscriptionId(pSM);
                subInstance->userParam = userParam;
            }
            else if (count * CONNECTION_TIMEOUT_MS_STEP >= SOPC_REQUEST_TIMEOUT_MS)
            {
                status = SOPC_STATUS_TIMEOUT;
                SOPC_StaMac_SetError(pSM);
            }
            else
            {
                // Creation of the sub failed: there's no more sub in progress && sub in progress created = false
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK != status)
        {
            // If state machine has no subscription, reset state machine callback
            if (!SOPC_StaMac_HasAnySubscription(pSM))
            {
                SOPC_UNUSED_RESULT(SOPC_StaMac_NewConfigureNotificationCallback(pSM, NULL));
            }
            SOPC_Free(subInstance);
        }
        else
        {
            // Keep track of the created subscription instance in case it is not deleted on disconnect call
            SOPC_SLinkedList_Append(secureConnection->subscriptions, 0, (uintptr_t) subInstance);
        }

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        // Free the allocated context if the subscription creation failed, otherwise kept during subscription
        // lifetime
        if (SOPC_STATUS_OK != status && NULL != reqCtx)
        {
            SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
        }
    }

    // Free the provided request if the subscription creation failed and request not provided to StaMac
    if (!requestInStaMac && NULL != subParams)
    {
        SOPC_EncodeableObject_Delete(subParams->encodeableType, (void**) &subParams);
    }
    if (SOPC_STATUS_OK == status)
    {
        return subInstance;
    }
    else
    {
        return NULL;
    }
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_SetAvailableTokens(SOPC_ClientConnection* secureConnection,
                                                                    uint32_t nbPublishTokens)
{
    if (nbPublishTokens == 0 || NULL == secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (secureConnection != sopc_client_helper_config.secureConnections[secureConnection->secureConnectionIdx])
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        SOPC_StaMac_Machine* pSM = secureConnection->stateMachine;
        status = SOPC_StaMac_SetSubscriptionNbTokens(pSM, nbPublishTokens);
    }
    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_GetRevisedParameters(SOPC_ClientHelper_Subscription* subscription,
                                                                      double* revisedPublishingInterval,
                                                                      uint32_t* revisedLifetimeCount,
                                                                      uint32_t* revisedMaxKeepAliveCount)
{
    if (NULL == subscription || NULL == subscription->secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_StaMac_Machine* pSM = subscription->secureConnection->stateMachine;
    SOPC_ReturnStatus status = SOPC_StaMac_GetSubscriptionRevisedParams(
        pSM, subscription->subscriptionId, revisedPublishingInterval, revisedLifetimeCount, revisedMaxKeepAliveCount);
    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

uintptr_t SOPC_ClientHelper_Subscription_GetUserParam(const SOPC_ClientHelper_Subscription* subscription)
{
    if (NULL == subscription || NULL == subscription->secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    return subscription->userParam;
}

SOPC_ClientConnection* SOPC_ClientHelper_GetSecureConnection(const SOPC_ClientHelper_Subscription* subscription)
{
    if (NULL == subscription || NULL == subscription->secureConnection)
    {
        return NULL;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return NULL;
    }
    // Check connection is still valid in configuration
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (subscription->secureConnection !=
        sopc_client_helper_config.secureConnections[subscription->secureConnection->secureConnectionIdx])
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    return subscription->secureConnection;
}

SOPC_ReturnStatus SOPC_ClientHelper_GetSubscriptionId(const SOPC_ClientHelper_Subscription* subscription,
                                                      uint32_t* pSubscriptionId)
{
    if (NULL == subscription || NULL == pSubscriptionId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    *pSubscriptionId = subscription->subscriptionId;

    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_CreateMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsReq,
    const uintptr_t* monitoredItemCtxArray,
    OpcUa_CreateMonitoredItemsResponse* monitoredItemsResp)
{
    bool requestInStaMac = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == subscription || NULL == subscription->secureConnection || NULL == monitoredItemsReq)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_CreateMonitoredItems_Ctx* appCtx = NULL;

        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (subscription->secureConnection !=
            sopc_client_helper_config.secureConnections[subscription->secureConnection->secureConnectionIdx])
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            appCtx = SOPC_Calloc(1, sizeof(*appCtx));
            if (NULL != appCtx)
            {
                appCtx->Results = monitoredItemsResp;
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }

        SOPC_StaMac_Machine* pSM = subscription->secureConnection->stateMachine;

        /* Create the monitored items and wait for its creation */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_NewCreateMonitoredItems(pSM, subscription->subscriptionId, monitoredItemsReq,
                                                         monitoredItemCtxArray, appCtx);
            requestInStaMac = true;
        }

        /* Wait for the monitored items to be created */
        if (SOPC_STATUS_OK == status)
        {
            int count = 0;
            while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_PopMonItByAppCtx(pSM, appCtx) &&
                   count * CONNECTION_TIMEOUT_MS_STEP < SOPC_REQUEST_TIMEOUT_MS)
            {
                /* Release the lock so that the event handler can work properly while waiting */
                mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

                SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);

                mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                ++count;
            }
            /* When the request timeoutHint is lower than SOPC_REQUEST_TIMEOUT_MS, the machine will go in error,
             *  and NOK is returned. */
            if (SOPC_StaMac_IsError(pSM))
            {
                status = SOPC_STATUS_NOK;
            }
            else if (count * CONNECTION_TIMEOUT_MS_STEP >= SOPC_REQUEST_TIMEOUT_MS)
            {
                status = SOPC_STATUS_TIMEOUT;
                SOPC_StaMac_SetError(pSM);
            }
        }
        SOPC_Free(appCtx);

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    // Free the provided request if the creation failed and request not provided to StaMac
    if (!requestInStaMac && NULL != monitoredItemsReq)
    {
        SOPC_EncodeableObject_Delete(monitoredItemsReq->encodeableType, (void**) &monitoredItemsReq);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_DeleteMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_DeleteMonitoredItemsRequest* delMonitoredItemsReq,
    OpcUa_DeleteMonitoredItemsResponse* delMonitoredItemsResp)
{
    bool requestInStaMac = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == subscription || NULL == subscription->secureConnection || NULL == delMonitoredItemsReq)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_DeleteMonitoredItems_Ctx* appCtx = NULL;

        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (subscription->secureConnection !=
            sopc_client_helper_config.secureConnections[subscription->secureConnection->secureConnectionIdx])
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            appCtx = SOPC_Calloc(1, sizeof(*appCtx));
            if (NULL != appCtx)
            {
                appCtx->Results = delMonitoredItemsResp;
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }

        SOPC_StaMac_Machine* pSM = subscription->secureConnection->stateMachine;

        /* Delete the monitored items and wait for its deletion */
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_StaMac_NewDeleteMonitoredItems(pSM, subscription->subscriptionId, delMonitoredItemsReq, appCtx);
            requestInStaMac = true;
        }

        /* Wait for the monitored items to be created */
        if (SOPC_STATUS_OK == status)
        {
            int count = 0;
            while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_PopDeleteMonItByAppCtx(pSM, appCtx) &&
                   count * CONNECTION_TIMEOUT_MS_STEP < SOPC_REQUEST_TIMEOUT_MS)
            {
                /* Release the lock so that the event handler can work properly while waiting */
                mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

                SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);

                mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
                ++count;
            }
            /* When the request timeoutHint is lower than SOPC_REQUEST_TIMEOUT_MS, the machine will go in error,
             *  and NOK is returned. */
            if (SOPC_StaMac_IsError(pSM))
            {
                status = SOPC_STATUS_NOK;
            }
            else if (count * CONNECTION_TIMEOUT_MS_STEP >= SOPC_REQUEST_TIMEOUT_MS)
            {
                status = SOPC_STATUS_TIMEOUT;
                SOPC_StaMac_SetError(pSM);
            }
        }
        SOPC_Free(appCtx);

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    // Free the provided request if the deletion failed and request not provided to StaMac
    if (!requestInStaMac && NULL != delMonitoredItemsReq)
    {
        SOPC_EncodeableObject_Delete(delMonitoredItemsReq->encodeableType, (void**) &delMonitoredItemsReq);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_DeleteSubscription(SOPC_ClientHelper_Subscription** ppSubscription)
{
    if (NULL == ppSubscription || NULL == *ppSubscription || NULL == (*ppSubscription)->secureConnection)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ClientHelper_Subscription* subscription = *ppSubscription;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (subscription->secureConnection !=
        sopc_client_helper_config.secureConnections[subscription->secureConnection->secureConnectionIdx])
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    SOPC_StaMac_Machine* pSM = subscription->secureConnection->stateMachine;

    SOPC_ClientHelper_ReqCtx* reqCtx = NULL;
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_StaMac_HasSubscriptionId(pSM, subscription->subscriptionId))
        {
            reqCtx = (SOPC_ClientHelper_ReqCtx*) SOPC_StaMac_GetSubscriptionCtx(pSM, subscription->subscriptionId);
            status = SOPC_StaMac_DeleteSubscription(pSM, subscription->subscriptionId);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    /* Wait for the subscription to be deleted */
    if (SOPC_STATUS_OK == status)
    {
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && SOPC_StaMac_HasSubscriptionId(pSM, subscription->subscriptionId) &&
               count * CONNECTION_TIMEOUT_MS_STEP < SOPC_REQUEST_TIMEOUT_MS)
        {
            mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);

            mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            ++count;
        }
        /* When the request timeoutHint is lower than SOPC_REQUEST_TIMEOUT_MS, the machine will go in error,
         *  and NOK is returned. */
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
        else if (count * CONNECTION_TIMEOUT_MS_STEP >= SOPC_REQUEST_TIMEOUT_MS)
        {
            status = SOPC_STATUS_TIMEOUT;
            SOPC_StaMac_SetError(pSM);
        }
        else if (SOPC_StaMac_HasSubscriptionId(pSM, subscription->subscriptionId))
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // Remove the subscription context if the subscription was deleted
    if (NULL != reqCtx)
    {
        SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
    }

    // Reset reference from secure connection to the created subscription
    SOPC_SLinkedList_RemoveFromValuePtr(subscription->secureConnection->subscriptions, (uintptr_t) subscription);

    // Unregister the notification context if there's no active subscription
    if (SOPC_STATUS_OK == status && !SOPC_StaMac_HasAnySubscription(pSM))
    {
        status = SOPC_StaMac_NewConfigureNotificationCallback(pSM, NULL);
    }

    mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_Free(subscription);
    *ppSubscription = NULL;

    return status;
}

// Subscription related: configure internal subscription id for authorized subscription services
// through API subscription service call
static void* SOPC_ClientHelperInternal_ConfigAndFilterService(const SOPC_ClientHelper_Subscription* subscription,
                                                              void* request)
{
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) request;
    if (&OpcUa_ModifySubscriptionRequest_EncodeableType == encType)
    {
        ((OpcUa_ModifySubscriptionRequest*) request)->SubscriptionId = subscription->subscriptionId;
    }
    else if (&OpcUa_SetPublishingModeRequest_EncodeableType == encType)
    {
        OpcUa_SetPublishingModeRequest* setPubModeReq = (OpcUa_SetPublishingModeRequest*) request;
        if (setPubModeReq->NoOfSubscriptionIds > 1)
        {
            return NULL;
        }
        setPubModeReq->SubscriptionIds[0] = subscription->subscriptionId;
    }
    else if (&OpcUa_ModifyMonitoredItemsRequest_EncodeableType == encType)
    {
        ((OpcUa_ModifyMonitoredItemsRequest*) request)->SubscriptionId = subscription->subscriptionId;
    }
    else if (&OpcUa_SetMonitoringModeRequest_EncodeableType == encType)
    {
        ((OpcUa_SetMonitoringModeRequest*) request)->SubscriptionId = subscription->subscriptionId;
    }
    else if (&OpcUa_SetTriggeringRequest_EncodeableType == encType)
    {
        ((OpcUa_SetTriggeringRequest*) request)->SubscriptionId = subscription->subscriptionId;
    }
    else
    {
        // Do not accept not related to subscription requests
        // and do not accept, CreateSub, DeleteSub, TransferSub, Pub, RePub, CreateMI, DeleteMI
        return NULL;
    }
    return request;
}

static SOPC_ReturnStatus SOPC_ClientHelper_Subscription_SyncAndAsyncRequest(
    const SOPC_ClientHelper_Subscription* subscription,
    void* subOrMIrequest,
    bool isSync,
    void** subOrMIresponse,
    uintptr_t asyncUserCtx)
{
    bool requestServiceCalled = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == subscription || NULL == subscription->secureConnection || NULL == subOrMIrequest ||
        (isSync && NULL == subOrMIresponse))
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status && !SOPC_ClientInternal_IsInitialized())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (subscription->secureConnection !=
            sopc_client_helper_config.secureConnections[subscription->secureConnection->secureConnectionIdx])
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        void* request = NULL;

        if (SOPC_STATUS_OK == status)
        {
            request = SOPC_ClientHelperInternal_ConfigAndFilterService(subscription, subOrMIrequest);

            if (NULL != request)
            {
                if (isSync)
                {
                    status = SOPC_ClientHelper_ServiceSync(subscription->secureConnection, request, subOrMIresponse);
                }
                else
                {
                    status = SOPC_ClientHelper_ServiceAsync(subscription->secureConnection, request, asyncUserCtx);
                }
                requestServiceCalled = true;
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }

        mutStatus = SOPC_Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    // Free the provided request if the deletion failed and request not provided to Service* function
    if (!requestServiceCalled && NULL != subOrMIrequest)
    {
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) subOrMIrequest, (void**) &subOrMIrequest);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_SyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                             void* subOrMIrequest,
                                                             void** subOrMIresponse)
{
    return SOPC_ClientHelper_Subscription_SyncAndAsyncRequest(subscription, subOrMIrequest, true, subOrMIresponse, 0);
}

SOPC_ReturnStatus SOPC_ClientHelper_Subscription_AsyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                              void* subOrMIrequest,
                                                              uintptr_t userContext)
{
    return SOPC_ClientHelper_Subscription_SyncAndAsyncRequest(subscription, subOrMIrequest, false, NULL, userContext);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                             void* request,
                                                             uintptr_t userContext)
{
    return SOPC_ClientHelper_DiscoveryServiceAsync(secConnConfig, request, userContext);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                            void* request,
                                                            void** response)
{
    return SOPC_ClientHelper_DiscoveryServiceSync(secConnConfig, request, response);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Connect(SOPC_SecureConnection_Config* secConnConfig,
                                               SOPC_ClientConnectionEvent_Fct* connectEventCb,
                                               SOPC_ClientConnection** secureConnection)
{
    return SOPC_ClientHelper_Connect(secConnConfig, connectEventCb, secureConnection);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Disconnect(SOPC_ClientConnection** secureConnection)
{
    return SOPC_ClientHelper_Disconnect(secureConnection);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                    void* request,
                                                    uintptr_t userContext)
{
    return SOPC_ClientHelper_ServiceAsync(secureConnection, request, userContext);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_ServiceSync(SOPC_ClientConnection* secureConnection,
                                                   void* request,
                                                   void** response)
{
    return SOPC_ClientHelper_ServiceSync(secureConnection, request, response);
}

SOPC_ClientHelper_Subscription* SOPC_ClientHelperNew_CreateSubscription(
    SOPC_ClientConnection* secureConnection,
    OpcUa_CreateSubscriptionRequest* subParams,
    SOPC_ClientSubscriptionNotification_Fct* subNotifCb,
    uintptr_t userParam)
{
    return SOPC_ClientHelper_CreateSubscription(secureConnection, subParams, subNotifCb, userParam);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_DeleteSubscription(SOPC_ClientHelper_Subscription** subscription)
{
    return SOPC_ClientHelper_DeleteSubscription(subscription);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SetAvailableTokens(SOPC_ClientConnection* secureConnection,
                                                                       uint32_t nbPublishTokens)
{
    return SOPC_ClientHelper_Subscription_SetAvailableTokens(secureConnection, nbPublishTokens);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_GetRevisedParameters(SOPC_ClientHelper_Subscription* subscription,
                                                                         double* revisedPublishingInterval,
                                                                         uint32_t* revisedLifetimeCount,
                                                                         uint32_t* revisedMaxKeepAliveCount)
{
    return SOPC_ClientHelper_Subscription_GetRevisedParameters(subscription, revisedPublishingInterval,
                                                               revisedLifetimeCount, revisedMaxKeepAliveCount);
}

uintptr_t SOPC_ClientHelperNew_Subscription_GetUserParam(const SOPC_ClientHelper_Subscription* subscription)
{
    return SOPC_ClientHelper_Subscription_GetUserParam(subscription);
}

SOPC_ClientConnection* SOPC_ClientHelperNew_GetSecureConnection(const SOPC_ClientHelper_Subscription* subscription)
{
    return SOPC_ClientHelper_GetSecureConnection(subscription);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_GetSubscriptionId(const SOPC_ClientHelper_Subscription* subscription,
                                                         uint32_t* pSubscriptionId)
{
    return SOPC_ClientHelper_GetSubscriptionId(subscription, pSubscriptionId);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsReq,
    const uintptr_t* monitoredItemCtxArray,
    OpcUa_CreateMonitoredItemsResponse* monitoredItemsResp)
{
    return SOPC_ClientHelper_Subscription_CreateMonitoredItems(subscription, monitoredItemsReq, monitoredItemCtxArray,
                                                               monitoredItemsResp);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems(
    const SOPC_ClientHelper_Subscription* subscription,
    OpcUa_DeleteMonitoredItemsRequest* delMonitoredItemsReq,
    OpcUa_DeleteMonitoredItemsResponse* delMonitoredItemsResp)
{
    return SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, delMonitoredItemsReq,
                                                               delMonitoredItemsResp);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_SyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                void* subOrMIrequest,
                                                                void** subOrMIresponse)
{
    return SOPC_ClientHelper_Subscription_SyncService(subscription, subOrMIrequest, subOrMIresponse);
}

SOPC_ReturnStatus SOPC_ClientHelperNew_Subscription_AsyncService(const SOPC_ClientHelper_Subscription* subscription,
                                                                 void* subOrMIrequest,
                                                                 uintptr_t userContext)
{
    return SOPC_ClientHelper_Subscription_AsyncService(subscription, subOrMIrequest, userContext);
}
