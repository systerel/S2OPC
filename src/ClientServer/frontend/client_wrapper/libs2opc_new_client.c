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

#include "libs2opc_client_internal.h"
#include "libs2opc_common_config.h"
#include "libs2opc_new_client.h"

#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"

#include "state_machine.h"

#include "opcua_statuscodes.h"

struct SOPC_ClientConnection
{
    SOPC_ClientConnectionEvent_Fct* connCb;

    // Synchronous treatment
    Condition syncCond;
    Mutex syncConnMutex;
    bool syncConnection;

    uint16_t secureConnectionIdx;
    SOPC_SecureChannelConfigIdx cfgId;
    bool isDiscovery;
    SOPC_StaMac_Machine* stateMachine; // only if !isDiscovery
};

/* The generic request context is used to managed the canceled request
 * (event response not received before timeout) */
typedef struct
{
    uint16_t secureConnectionIdx;

    bool isAsyncCall; /* If call is async the callback is set */
    SOPC_ServiceAsyncResp_Fct* asyncRespCb;
    uintptr_t userCtx;

    Mutex mutex; /* protect this context */
    Condition condition;
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
        // finished => already false
        result->status = SOPC_STATUS_NOK;
        result->isDiscoveryModeService = isDiscoveryModeService;
        result->responseResultCtx = responseResultCtx;
        status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Mutex_Initialization(&result->mutex);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Condition_Init(&result->condition);
    }
    if (SOPC_STATUS_OK != status)
    {
        Condition_Clear(&result->condition);
        Mutex_Clear(&result->mutex);
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
        Mutex_Initialization(&result->mutex);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Condition_Init(&result->condition);
    }
    if (SOPC_STATUS_OK != status)
    {
        Condition_Clear(&result->condition);
        Mutex_Clear(&result->mutex);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

static void SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(SOPC_ClientHelper_ReqCtx* genReqCtx)
{
    SOPC_ASSERT(NULL != genReqCtx);
    Condition_Clear(&genReqCtx->condition);
    Mutex_Clear(&genReqCtx->mutex);
    genReqCtx->responseResultCtx = NULL; // shall be freed by caller or previously
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
        Mutex_UnlockAndWaitCond(&genReqCtx->condition, &genReqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    return genReqCtx->status;
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
        // TODO: check config content (mandatory + secu mode vs secu policy vs certs vs user policy vs ...)
        SOPC_Client_Config* cConfig = &config->clientConfig;
        status = SOPC_HelperConfigClient_Finalize_SecureConnectionConfig(cConfig, secConnConfig);
    }
    return status;
}

/*
 * TODO: remove and configure dynamically subscription parameters
 */
/* Connection global timeout */
#define TMP_TIMEOUT_MS 10000
/* Default publish period */
#define TMP_PUBLISH_PERIOD_MS 500
/* Default max keep alive count */
#define TMP_MAX_KEEP_ALIVE_COUNT 3
/* Lifetime Count of subscriptions */
#define TMP_MAX_LIFETIME_COUNT 10
/* Number of targeted publish token */
#define TMP_PUBLISH_N_TOKEN 3

static void TMP_DataChangeCbk(const SOPC_LibSub_ConnectionId c_id,
                              const SOPC_LibSub_DataId d_id,
                              const SOPC_LibSub_Value* value)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_UNUSED_ARG(d_id);
    SOPC_UNUSED_ARG(value);
}

static void SOPC_ClientInternal_EventCbk(SOPC_LibSub_ConnectionId c_id,
                                         SOPC_LibSub_ApplicativeEvent event,
                                         SOPC_StatusCode status, /* Note: actually a ReturnStatus */
                                         const void* response,
                                         uintptr_t genContext)
{
    SOPC_UNUSED_ARG(c_id); // TODO: check connection still exists ? It should not be necessary with a sync disc

    if (SOPC_LibSub_ApplicativeEvent_Response != event)
    {
        return;
    }

    bool isAsync = false;
    SOPC_ClientHelper_ReqCtx* genCtx = (SOPC_ClientHelper_ReqCtx*) genContext;
    SOPC_ReturnStatus statusMutex = Mutex_Lock(&genCtx->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

    if (genCtx->isAsyncCall)
    {
        isAsync = true;
        SOPC_EncodeableType* pEncType = NULL;
        if (SOPC_LibSub_ApplicativeEvent_Response == event)
        {
            pEncType = *(SOPC_EncodeableType* const*) response;
        }
        genCtx->asyncRespCb(pEncType, response, genCtx->userCtx);
    }
    else
    {
        void* responseContext = genCtx->responseResultCtx;
        SOPC_ASSERT(NULL != responseContext);
        if (SOPC_LibSub_ApplicativeEvent_Response == event)
        {
            SOPC_EncodeableType* pEncType = *(SOPC_EncodeableType* const*) response;

            void** genResponseContext = (void**) responseContext;

            status = SOPC_Encodeable_Create(pEncType, genResponseContext);
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
    statusMutex = Mutex_Unlock(&genCtx->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);
    /* Signal that the response is available */
    statusMutex = Condition_SignalAll(&genCtx->condition);
    SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

    if (isAsync)
    {
        // Context is not necessary anymore
        SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(genCtx);
    }
}

static uintptr_t TMP_StaMacCtx = 0; // inhibit ...

static void SOPC_ClientInternal_ConnectionStateCallback(SOPC_App_Com_Event event,
                                                        void* param,
                                                        SOPC_ClientConnection* cc)
{
    SOPC_ASSERT(NULL != cc);
    if (SE_CLOSED_SESSION == event || SE_SESSION_ACTIVATION_FAILURE == event || SE_SESSION_REACTIVATING == event ||
        SE_ACTIVATED_SESSION == event)
    {
        bool isSyncConn = false;
        SOPC_ReturnStatus statusMutex = Mutex_Lock(&cc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        isSyncConn = cc->syncConnection;

        Mutex_Unlock(&cc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSyncConn)
        {
            /* Signal that the response is available */
            statusMutex = Condition_SignalAll(&cc->syncCond);
            SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);
        }
        else
        {
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
            cc->connCb(cc, connEvent, serviceStatus);
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

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ClientHelper_ReqCtx* serviceReqCtx = NULL;

    switch (event)
    {
    /* appCtx is request context */
    case SE_RCV_SESSION_RESPONSE:
    case SE_RCV_DISCOVERY_RESPONSE:
    case SE_SND_REQUEST_FAILED:
        SOPC_ASSERT(0 != appContext);
        serviceReqCtx = (SOPC_ClientHelper_ReqCtx*) ((SOPC_StaMac_ReqCtx*) appContext)->appCtx;
        cc = sopc_client_helper_config.secureConnections[serviceReqCtx->secureConnectionIdx];
        break;
    /* appCtx is session context */
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_ACTIVATED_SESSION:
    case SE_SESSION_REACTIVATING:
    case SE_CLOSED_SESSION:
        cc = sopc_client_helper_config.secureConnections[appContext];
        break;
    case SE_REVERSE_ENDPOINT_CLOSED:
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, "Reverse endpoint '%s' closed",
                              SOPC_ToolkitClient_GetReverseEndpointURL(IdOrStatus));
        sopc_client_helper_config
            .openedReverseEndpointsFromCfgIdx[SOPC_ClientInternal_GetReverseEPcfgIdxNoOffset(IdOrStatus)] = false;
        mutStatus = Condition_SignalAll(&sopc_client_helper_config.reverseEPsClosedCond);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        break;
    default:
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ClientInternal_ToolkitEventCallback: unexpected event %d received.", event);
        return;
    }
    if (NULL == cc && event != SE_REVERSE_ENDPOINT_CLOSED)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ClientInternal_ToolkitEventCallback: unexpected context received for event %d.", event);
    }
    else if (NULL != serviceReqCtx && serviceReqCtx->isDiscoveryModeService)
    {
        // Discovery service call (no session) not managed by state machine: direct call to event callback
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        SOPC_LibSub_ApplicativeEvent libsubEvent = SOPC_LibSub_ApplicativeEvent_Response;
        if (SE_RCV_DISCOVERY_RESPONSE != event)
        {
            status = SOPC_STATUS_NOK;
            libsubEvent = SOPC_LibSub_ApplicativeEvent_SendFailed;
        }
        SOPC_ClientInternal_EventCbk(cc->secureConnectionIdx, libsubEvent, status, param, (uintptr_t) serviceReqCtx);
        SOPC_Free((void*) appContext);
    }
    else if (event != SE_REVERSE_ENDPOINT_CLOSED) // state machine does not manage reverse EPs
    {
        // Connection management or service on session call managed by state machine
        pSM = sopc_client_helper_config.secureConnections[cc->secureConnectionIdx]->stateMachine;
        SOPC_ASSERT(NULL != pSM);

        if (SOPC_StaMac_EventDispatcher(pSM, NULL, event, IdOrStatus, param, appContext))
        {
            /* Post process the event for callbacks. */
            SOPC_ClientInternal_ConnectionStateCallback(event, param, cc);
        }
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_MayCreateReverseEp(const SOPC_SecureConnection_Config* secConnConfig,
                                                                      SOPC_ReverseEndpointConfigIdx* res)
{
    if (NULL == secConnConfig->reverseURL)
    {
        return SOPC_STATUS_OK;
    }
    SOPC_ASSERT(NULL != res);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    const SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    bool foundIdx = false;
    uint16_t rEPidx = 0;
    for (uint16_t i = 0; !foundIdx && i < pConfig->clientConfig.nbReverseEndpointURLs; i++)
    {
        if (0 == strcmp(pConfig->clientConfig.reverseEndpointURLs[i], secConnConfig->reverseURL))
        {
            foundIdx = true;
            rEPidx = i;
        }
    }
    if (foundIdx)
    {
        SOPC_ReverseEndpointConfigIdx reverseConfigIdx =
            sopc_client_helper_config.configuredReverseEndpointsToCfgIdx[rEPidx];
        if (0 == reverseConfigIdx)
        {
            reverseConfigIdx = SOPC_ToolkitClient_AddReverseEndpointConfig(secConnConfig->reverseURL);
        }
        if (0 != reverseConfigIdx)
        {
            // If the reverse endpoint is not opened, open ti
            const uint32_t reverseConfigIdxNoOffset = reverseConfigIdx - SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS;
            if (!sopc_client_helper_config.openedReverseEndpointsFromCfgIdx[reverseConfigIdxNoOffset])
            {
                // Store the reverse EP configuration index
                sopc_client_helper_config.configuredReverseEndpointsToCfgIdx[rEPidx] = reverseConfigIdx;
                // Open the reverse endpoint
                SOPC_ToolkitClient_AsyncOpenReverseEndpoint(reverseConfigIdx);
                sopc_client_helper_config.openedReverseEndpointsFromCfgIdx[reverseConfigIdxNoOffset] = true;
            }
            *res = reverseConfigIdx;
            status = SOPC_STATUS_OK;
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_ClientHelperInternal_MayCreateReverseEp: creation of reverse endpoint config %s failed",
                secConnConfig->reverseURL);
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SOPC_ClientHelperInternal_MayCreateReverseEp: unexpected: reverse endpoint URL %s not "
                               "recorded in client config",
                               secConnConfig->reverseURL);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_CreateClientConnection(
    const SOPC_SecureConnection_Config* secConnConfig,
    bool isDiscovery,
    SOPC_ClientConnection** outClientConnection)
{
    SOPC_ASSERT(secConnConfig != NULL);
    SOPC_ASSERT(outClientConnection != NULL);
    SOPC_ReverseEndpointConfigIdx reverseConfigIdx = 0;

    SOPC_ClientConnection* res = SOPC_Calloc(sizeof(*res), 1);
    if (NULL == res)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_ClientHelperInternal_MayCreateReverseEp(secConnConfig, &reverseConfigIdx);

    SOPC_SecureChannelConfigIdx cfgId = 0;
    SOPC_StaMac_Machine* stateMachine = NULL;
    if (SOPC_STATUS_OK == status)
    {
        /* TODO: propagate the const in low level API ? */
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        cfgId = SOPC_ToolkitClient_AddSecureChannelConfig((SOPC_SecureChannel_Config*) &secConnConfig->scConfig);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        if (0 != cfgId)
        {
            status = SOPC_STATUS_OK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Note: still create state machine even in discovery since it might be used for non-discovery later
        const char* username = NULL;
        const char* password = NULL;

        const char* userX509certPath = NULL;
        const char* userX509keyPath = NULL;
        bool userX509keyEncrypted = false;

        if (secConnConfig->sessionConfig.userTokenType == OpcUa_UserTokenType_UserName)
        {
            username = secConnConfig->sessionConfig.userToken.userName.userName;
            password = secConnConfig->sessionConfig.userToken.userName.userPwd;
        }
        else if (secConnConfig->sessionConfig.userTokenType == OpcUa_UserTokenType_Certificate)
        {
            SOPC_ASSERT(secConnConfig->sessionConfig.userToken.userX509.isConfigFromPathNeeded);
            userX509certPath = secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath;
            userX509keyPath = secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyPath;
            userX509keyEncrypted = secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyEncrypted;
        }

        status = SOPC_StaMac_Create(cfgId, reverseConfigIdx, secConnConfig->secureConnectionIdx,
                                    secConnConfig->sessionConfig.userPolicyId, username, password, userX509certPath,
                                    userX509keyPath, userX509keyEncrypted, TMP_DataChangeCbk, TMP_PUBLISH_PERIOD_MS,
                                    TMP_MAX_KEEP_ALIVE_COUNT, TMP_MAX_LIFETIME_COUNT, TMP_PUBLISH_N_TOKEN,
                                    TMP_TIMEOUT_MS, SOPC_ClientInternal_EventCbk, TMP_StaMacCtx, &stateMachine);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Data for synchronous calls
        SOPC_ReturnStatus mutStatus = Condition_Init(&res->syncCond);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        mutStatus = Mutex_Initialization(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        res->cfgId = cfgId;
        res->secureConnectionIdx = secConnConfig->secureConnectionIdx;
        res->isDiscovery = isDiscovery;
        res->stateMachine = stateMachine;
        *outClientConnection = res;
    }
    else
    {
        SOPC_Free(res);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_DiscoveryService(bool isSynchronous,
                                                                    SOPC_SecureConnection_Config* secConnConfig,
                                                                    void* request,
                                                                    void** response,
                                                                    uintptr_t userContext)
{
    if (NULL == secConnConfig || NULL == request)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (isSynchronous && NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Check connection is valid and is not already created
    if (!SOPC_ClientHelperInternal_CheckConnectionValid(pConfig, secConnConfig) ||
        (!isSynchronous && NULL == sopc_client_helper_config.asyncRespCb))
    {
        Mutex_Unlock(&sopc_client_helper_config.configMutex);
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_ClientHelperInternal_MayFinalizeSecureConnection(pConfig, secConnConfig);

    SOPC_ReverseEndpointConfigIdx reverseConfigIdx = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperInternal_MayCreateReverseEp(secConnConfig, &reverseConfigIdx);
    }

    SOPC_ClientConnection* res = sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx];
    if (SOPC_STATUS_OK == status && NULL == res)
    {
        status = SOPC_ClientHelperInternal_CreateClientConnection(secConnConfig, true, &res);
        if (SOPC_STATUS_OK == status)
        {
            sopc_client_helper_config.secureConnections[secConnConfig->secureConnectionIdx] = res;
        }
    }

    /* Call discovery service */
    SOPC_StaMac_ReqCtx* smReqCtx = NULL;
    SOPC_ClientHelper_ReqCtx* reqCtx = NULL;
    if (SOPC_STATUS_OK == status)
    {
        /* create a context wrapper */

        smReqCtx = SOPC_Calloc(1, sizeof(*smReqCtx));
        if (isSynchronous)
        {
            reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateSync(res->secureConnectionIdx, response, true);
        }
        else
        {
            reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateAsync(
                res->secureConnectionIdx, true, sopc_client_helper_config.asyncRespCb, userContext);
        }
        if (NULL == reqCtx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
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

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* send the request and wait for the result if sync operation */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus statusMutex = Mutex_Lock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        /* send the request */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_EndpointConnectionCfg endpointConnectionCfg = {.reverseEndpointConfigIdx = reverseConfigIdx,
                                                                .secureChannelConfigIdx = res->cfgId};

            status = SOPC_ToolkitClient_AsyncSendDiscoveryRequest(endpointConnectionCfg, request, (uintptr_t) smReqCtx);
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

        statusMutex = Mutex_Unlock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSynchronous && NULL != reqCtx)
        {
            SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_DiscoveryServiceAsync(SOPC_SecureConnection_Config* secConnConfig,
                                                          void* request,
                                                          uintptr_t userContext)
{
    return SOPC_ClientHelperInternal_DiscoveryService(false, secConnConfig, request, NULL, userContext);
}

SOPC_ReturnStatus SOPC_ClientHelper_DiscoveryServiceSync(SOPC_SecureConnection_Config* secConnConfig,
                                                         void* request,
                                                         void** response)
{
    return SOPC_ClientHelperInternal_DiscoveryService(true, secConnConfig, request, response, 0);
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
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
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

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* Starts the machine */
    if (SOPC_STATUS_OK == status)
    {
        mutStatus = Mutex_Lock(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (!res->syncConnection)
        {
            res->syncConnection = true;
            status = SOPC_StaMac_StartSession(res->stateMachine);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            while (SOPC_STATUS_OK == status && !SOPC_StaMac_IsError(res->stateMachine) &&
                   !SOPC_StaMac_IsConnected(res->stateMachine))
            {
                // Note: we use the low layer timeouts and do not need a new one
                status = Mutex_UnlockAndWaitCond(&res->syncCond, &res->syncConnMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
            }

            if (SOPC_StaMac_IsError(res->stateMachine) || !SOPC_StaMac_IsConnected(res->stateMachine))
            {
                status = SOPC_STATUS_CLOSED;
            }
        }

        res->syncConnection = false;

        mutStatus = Mutex_Unlock(&res->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        *secureConnection = res;
    }
    else
    {
        if (NULL != res && NULL != res->stateMachine)
        {
            SOPC_StaMac_Delete(&res->stateMachine);
        }
        if (NULL != res)
        {
            sopc_client_helper_config.secureConnections[res->secureConnectionIdx] = NULL;
            SOPC_Free(res);
        }
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
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_StaMac_Machine* pSM = NULL;

    if (*secureConnection == sopc_client_helper_config.secureConnections[pSc->secureConnectionIdx])
    {
        status = SOPC_STATUS_OK;
        pSM = pSc->stateMachine;
    } // else: SOPC_STATUS_INVALID_STATE;

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Early return after mutex unlock
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    if (SOPC_StaMac_IsConnected(pSM))
    {
        mutStatus = Mutex_Lock(&pSc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        if (!pSc->syncConnection)
        {
            pSc->syncConnection = true;
            status = SOPC_StaMac_StopSession(pSM);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        while (SOPC_STATUS_OK == status && !SOPC_StaMac_IsError(pSM) && SOPC_StaMac_IsConnected(pSM))
        {
            // Note: we use the low layer timeouts and do not need a new one
            status = Mutex_UnlockAndWaitCond(&pSc->syncCond, &pSc->syncConnMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }

        pSc->syncConnection = false;

        mutStatus = Mutex_Unlock(&pSc->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        mutStatus = Condition_Clear(&(*secureConnection)->syncCond);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        mutStatus = Mutex_Clear(&(*secureConnection)->syncConnMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

        sopc_client_helper_config.secureConnections[(*secureConnection)->secureConnectionIdx] = NULL;
        SOPC_Free(*secureConnection);
        *secureConnection = NULL;
        SOPC_StaMac_Delete(&pSM);

        mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    return status;
}

static SOPC_ReturnStatus SOPC_ClientHelperInternal_Service(bool isSynchronous,
                                                           SOPC_ClientConnection* secureConnection,
                                                           void* request,
                                                           void** response,
                                                           uintptr_t userContext)
{
    if (NULL == secureConnection || NULL == request)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (isSynchronous && NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // TODO: filter request types ? (also depends on subscription choices)

    if (!SOPC_ClientInternal_IsInitialized())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;
    SOPC_ClientHelper_ReqCtx* reqCtx = NULL;

    if (secureConnection != sopc_client_helper_config.secureConnections[secureConnection->secureConnectionIdx] ||
        (!isSynchronous && NULL == sopc_client_helper_config.asyncRespCb))
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
            reqCtx =
                SOPC_ClientHelperInternal_GenReqCtx_CreateSync(secureConnection->secureConnectionIdx, response, false);
        }
        else
        {
            reqCtx = SOPC_ClientHelperInternal_GenReqCtx_CreateAsync(
                secureConnection->secureConnectionIdx, false, sopc_client_helper_config.asyncRespCb, userContext);
        }
        if (NULL == reqCtx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    /* send the request and wait for the result if sync operation */
    if (SOPC_STATUS_OK == status)
    {
        /* Prepare the synchronous context */
        SOPC_ReturnStatus statusMutex = Mutex_Lock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        /* send the request */

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_StaMac_SendRequest(pSM, request, (uintptr_t) reqCtx, SOPC_REQUEST_SCOPE_APPLICATION,
                                             SOPC_REQUEST_TYPE_USER);
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

        statusMutex = Mutex_Unlock(&reqCtx->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == statusMutex);

        if (isSynchronous && NULL != reqCtx)
        {
            SOPC_ClientHelperInternal_GenReqCtx_ClearAndFree(reqCtx);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientHelper_ServiceAsync(SOPC_ClientConnection* secureConnection,
                                                 void* request,
                                                 uintptr_t userContext)
{
    return SOPC_ClientHelperInternal_Service(false, secureConnection, request, NULL, userContext);
}

SOPC_ReturnStatus SOPC_ClientHelper_ServiceSync(SOPC_ClientConnection* secureConnection, void* request, void** response)
{
    return SOPC_ClientHelperInternal_Service(true, secureConnection, request, response, 0);
}
