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
#include <stdio.h>

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_internal.h"

#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config.h"

static void SOPC_ServerHelper_ComEventCb(SOPC_App_Com_Event event,
                                         uint32_t IdOrStatus,
                                         void* param,
                                         uintptr_t helperContext);

const SOPC_ServerHelper_Config sopc_server_helper_config_default = {
    .initialized = false,
    .state = SOPC_SERVER_STATE_INITIALIZING,
    .addressSpace = NULL,
    .writeNotifCb = NULL,
    .asyncRespCb = NULL,
    .syncLocalServiceId = 0,
    .syncResp = NULL,
    .syncServeStopData =
        {
            .serverRequestedToStop = false,
            .serverAllEndpointsClosed = false,
        },
    .serverStoppedStatus = SOPC_STATUS_OK,
    .stoppedCb = NULL,
    .configuredSecondsTillShutdown = DEFAULT_SHUTDOWN_PHASE_IN_SECONDS,
    .authenticationManager = NULL,
    .authorizationManager = NULL,
    .buildInfo = NULL,
    .nbEndpoints = 0,
    .endpointIndexes = NULL,
    .endpointClosed = NULL,
};

SOPC_ServerHelper_Config sopc_server_helper_config;

// Manage configuration state
bool SOPC_ServerInternal_IsConfiguring(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_CONFIGURING == sopc_server_helper_config.state;
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

bool SOPC_ServerInternal_IsStarted(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_STARTED == sopc_server_helper_config.state;
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

bool SOPC_ServerInternal_IsStopped(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_STOPPED == sopc_server_helper_config.state;
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

// Check configuration is correct
static bool SOPC_HelperConfigServer_CheckConfig(void)
{
    bool res = sopc_server_helper_config.nbEndpoints > 0;
    if (!res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Error no endpoint defined, at least one shall be defined");
    }
    bool hasUserName = false;
    bool hasSecurity = false;
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
    {
        SOPC_Endpoint_Config* ep = sopc_server_helper_config.endpoints[i];
        for (uint8_t j = 0; j < ep->nbSecuConfigs; j++)
        {
            // Is it using security ?
            SOPC_SecurityPolicy* sp = &ep->secuConfigurations[j];
            if (0 != (sp->securityModes & (SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK)))
            {
                hasSecurity = true;
            }

            // Is it using username policy ?
            for (uint8_t k = 0; k < sp->nbOfUserTokenPolicies; k++)
            {
                OpcUa_UserTokenPolicy* utp = &sp->userTokenPolicies[k];
                if (OpcUa_UserTokenType_UserName == utp->TokenType)
                {
                    // Note that SOPC_SecurityPolicy_AddUserTokenPolicy already warns when used with None security SC
                    hasUserName = true;
                }
            }
        }

        // Other verifications to be done by SOPC_ToolkitServer_AddEndpointConfig
    }
    // Check that the server defines certificates and PKI provider if endpoint uses security
    if (hasSecurity)
    {
        if (NULL == pConfig->serverConfig.serverCertificate || NULL == pConfig->serverConfig.serverKey)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Server key and certificates not defined whereas some endpoint(s) are defined as secured.");
            res = false;
        }

        if (NULL == pConfig->serverConfig.pki)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "No server PKI provider defined whereas some endpoint(s) are defined as secured.");
            res = false;
        }
    }
    // Check that the server define user managers
    if (hasUserName && (NULL == sopc_server_helper_config.authenticationManager ||
                        NULL == sopc_server_helper_config.authorizationManager))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "No authentication and/or authorization user manager defined."
                                 " Default will be permissive whereas UserName policy is used in endpoint(s).");
    }

    if (NULL == pConfig->serverConfig.namespaces)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "No namespace defined for the server, 1 server namespace shall be defined");
        res = false;
    }

    if (NULL == pConfig->serverConfig.localeIds)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "No locales defined for the server.");
        // Create empty locale array
        pConfig->serverConfig.localeIds = SOPC_Calloc(1, sizeof(char*));
        assert(NULL != pConfig->serverConfig.localeIds);
        pConfig->serverConfig.localeIds[0] = NULL;
    }

    // TODO: checks on application description content ?

    return res;
}

// Finalize checked configuration
static bool SOPC_HelperConfigServer_FinaliseCheckedConfig(void)
{
    bool res = true;

    if (NULL == sopc_server_helper_config.authenticationManager)
    {
        sopc_server_helper_config.authenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
        if (NULL == sopc_server_helper_config.authenticationManager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create a default authentication manager");
            res = false;
        }
    }
    if (NULL == sopc_server_helper_config.authorizationManager)
    {
        sopc_server_helper_config.authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == sopc_server_helper_config.authorizationManager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create a default authorization manager");
            res = false;
        }
    }
    // Associate global user authentication/authorization managers with each low-level endpoint configuration
    // Note: in the future, low level configuration should be changed to define those at server level and not endpoint
    if (res)
    {
        SOPC_ServerInternal_SetEndpointsUserMgr();
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == pConfig->serverConfig.namespaces)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "No namespace defined for the server, 1 server namespace shall be defined");
        res = false;
    }

    if (NULL == pConfig->serverConfig.localeIds)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "No locales defined for the server.");
        // Create empty locale array
        pConfig->serverConfig.localeIds = SOPC_Calloc(1, sizeof(char*));
        assert(NULL != pConfig->serverConfig.localeIds);
        pConfig->serverConfig.localeIds[0] = NULL;
    }

    // If none discovery is described in application description
    if (res && 0 == pConfig->serverConfig.serverDescription.NoOfDiscoveryUrls)
    {
        // And some discovery endpoints are present, add them into it
        uint8_t nbDiscovery = 0;
        for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
        {
            SOPC_Endpoint_Config* ep = sopc_server_helper_config.endpoints[i];
            // Is this a discovery endpoint ? Add it to application description.
            if (ep->hasDiscoveryEndpoint)
            {
                nbDiscovery++;
            }
        }

        if (nbDiscovery > 0)
        {
            OpcUa_ApplicationDescription* appDesc = &pConfig->serverConfig.serverDescription;
            appDesc->DiscoveryUrls = SOPC_Calloc((size_t) nbDiscovery, sizeof(SOPC_String));
            if (NULL != appDesc->DiscoveryUrls)
            {
                appDesc->NoOfDiscoveryUrls = nbDiscovery;
                for (uint8_t i = 0; i < nbDiscovery; i++)
                {
                    SOPC_String_Initialize(&appDesc->DiscoveryUrls[i]);
                }
                uint8_t j = 0;
                for (uint8_t i = 0; res && i < sopc_server_helper_config.nbEndpoints && j < nbDiscovery; i++)
                {
                    SOPC_Endpoint_Config* ep = sopc_server_helper_config.endpoints[i];
                    // Is this a discovery endpoint ? Add it to application description.
                    if (ep->hasDiscoveryEndpoint)
                    {
                        SOPC_ReturnStatus status =
                            SOPC_String_CopyFromCString(&appDesc->DiscoveryUrls[j], ep->endpointURL);
                        res = (SOPC_STATUS_OK == status);
                        j++;
                    }
                }
            }
            else
            {
                res = false;
            }
        }
    }

    return res;
}

// Check for configuration issues and set server state as configured
bool SOPC_ServerInternal_CheckConfigAndSetConfiguredState(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_CONFIGURING == sopc_server_helper_config.state;
        if (res)
        {
            res = SOPC_HelperConfigServer_CheckConfig();
        }
        if (res)
        {
            res = SOPC_HelperConfigServer_FinaliseCheckedConfig();
        }
        if (res)
        {
            // Set state as configured in case of success
            sopc_server_helper_config.state = SOPC_SERVER_STATE_CONFIGURED;
        }
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

bool SOPC_ServerInternal_SetStartedState(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_CONFIGURED == sopc_server_helper_config.state;
        if (res)
        {
            sopc_server_helper_config.state = SOPC_SERVER_STATE_STARTED;
        }
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

// Check current state and set server state as stopping in case of success
bool SOPC_ServerInternal_SetStoppingState(void)
{
    bool res = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        res = SOPC_SERVER_STATE_STARTED == sopc_server_helper_config.state;
        if (res)
        {
            sopc_server_helper_config.state = SOPC_SERVER_STATE_STOPPING;
        }
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    return res;
}

// Set server state as stopped
void SOPC_ServerInternal_SetStoppedState(void)
{
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        sopc_server_helper_config.state = SOPC_SERVER_STATE_STOPPED;
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
}

void SOPC_ServerInternal_SetEndpointsUserMgr(void)
{
    for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
    {
        sopc_server_helper_config.endpoints[i]->authenticationManager = sopc_server_helper_config.authenticationManager;
        sopc_server_helper_config.endpoints[i]->authorizationManager = sopc_server_helper_config.authorizationManager;
    }
}

static void SOPC_ServerHelper_ComEventCb(SOPC_App_Com_Event event,
                                         uint32_t IdOrStatus,
                                         void* param,
                                         uintptr_t helperContext)
{
    SOPC_UNUSED_ARG(IdOrStatus);

    bool startedOrStopping = false;
    if (SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        Mutex_Lock(&sopc_server_helper_config.stateMutex);
        startedOrStopping = SOPC_SERVER_STATE_STARTED == sopc_server_helper_config.state ||
                            SOPC_SERVER_STATE_STOPPING == sopc_server_helper_config.state;
        Mutex_Unlock(&sopc_server_helper_config.stateMutex);
    }
    if (!startedOrStopping)
    {
        return;
    }

    SOPC_HelperConfigInternal_Ctx* ctx;
    switch (event)
    {
    /* Server events  */
    case SE_CLOSED_ENDPOINT:
        SOPC_ServerInternal_ClosedEndpoint(IdOrStatus, (SOPC_ReturnStatus) helperContext);
        break;
    case SE_LOCAL_SERVICE_RESPONSE:
        ctx = (SOPC_HelperConfigInternal_Ctx*) helperContext;
        assert(event == ctx->event);
        if (ctx->eventCtx.localService.isSyncCall)
        {
            SOPC_ServerInternal_SyncLocalServiceCb(*(SOPC_EncodeableType**) param, param, ctx);
        }
        else
        {
            SOPC_ServerInternal_AsyncLocalServiceCb(*(SOPC_EncodeableType**) param, param, ctx);
        }
        SOPC_Free(ctx);
        break;
    default:
        assert(false && "Unexpected events");
    }
}

static void SOPC_ServerHelper_AdressSpaceNotifCb(const SOPC_CallContext* callCtxPtr,
                                                 SOPC_App_AddSpace_Event event,
                                                 void* opParam,
                                                 SOPC_StatusCode opStatus)
{
    if (AS_WRITE_EVENT != event || NULL == sopc_server_helper_config.writeNotifCb)
    {
        return;
    }
    sopc_server_helper_config.writeNotifCb(callCtxPtr, (OpcUa_WriteValue*) opParam, opStatus);
}

SOPC_ReturnStatus SOPC_HelperConfigServer_Initialize(void)
{
    if (!SOPC_CommonHelper_GetInitialized() || SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        // Common wrapper not initialized or server wrapper already initialized
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_CommonHelper_SetServerComEvent(SOPC_ServerHelper_ComEventCb);

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    sopc_server_helper_config = sopc_server_helper_config_default;

    // We only do copies in helper config
    pConfig->serverConfig.freeCstringsFlag = true;

    // Server state initialization
    Mutex_Initialization(&sopc_server_helper_config.stateMutex);
    sopc_server_helper_config.state = SOPC_SERVER_STATE_CONFIGURING;

    // Data for synchronous local service call
    Condition_Init(&sopc_server_helper_config.syncLocalServiceCond);
    Mutex_Initialization(&sopc_server_helper_config.syncLocalServiceMutex);

    // Data used only when server is running with synchronous Serve function
    Condition_Init(&sopc_server_helper_config.syncServeStopData.serverStoppedCond);
    Mutex_Initialization(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);

    status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_ServerHelper_AdressSpaceNotifCb);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_HelperConfigServer_Clear();
    }
    else
    {
        SOPC_Atomic_Int_Set(&sopc_server_helper_config.initialized, (int32_t) true);
    }
    return status;
}

void SOPC_HelperConfigServer_Clear(void)
{
    if (!SOPC_Atomic_Int_Get(&sopc_server_helper_config.initialized))
    {
        return;
    }

    SOPC_CommonHelper_SetServerComEvent(NULL);

    // Ensures event callback will not be executed after this point, or will return immediately
    Mutex_Lock(&sopc_server_helper_config.stateMutex);
    sopc_server_helper_config.state = SOPC_SERVER_STATE_INITIALIZING;
    Mutex_Unlock(&sopc_server_helper_config.stateMutex);

    // Clear endpoints since not stored in S2OPC config anymore in wrapper:
    for (int i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
    {
        SOPC_ServerInternal_ClearEndpoint(sopc_server_helper_config.endpoints[i]);
        SOPC_Free(sopc_server_helper_config.endpoints[i]);
        sopc_server_helper_config.endpoints[i] = NULL;
    }
    SOPC_AddressSpace_Delete(sopc_server_helper_config.addressSpace);
    sopc_server_helper_config.addressSpace = NULL;
    Condition_Clear(&sopc_server_helper_config.syncLocalServiceCond);
    Mutex_Clear(&sopc_server_helper_config.syncLocalServiceMutex);

    // Data used only when server is running with synchronous Serve function
    Condition_Clear(&sopc_server_helper_config.syncServeStopData.serverStoppedCond);
    Mutex_Clear(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);

    SOPC_UserAuthentication_FreeManager(&sopc_server_helper_config.authenticationManager);
    SOPC_UserAuthorization_FreeManager(&sopc_server_helper_config.authorizationManager);

    if (NULL != sopc_server_helper_config.buildInfo)
    {
        OpcUa_BuildInfo_Clear(sopc_server_helper_config.buildInfo);
        SOPC_Free(sopc_server_helper_config.buildInfo);
        sopc_server_helper_config.buildInfo = NULL;
    }

    SOPC_Free(sopc_server_helper_config.endpointIndexes);
    SOPC_Free(sopc_server_helper_config.endpointClosed);

    SOPC_Atomic_Int_Set(&sopc_server_helper_config.initialized, (int32_t) false);
    Mutex_Clear(&sopc_server_helper_config.stateMutex);
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetMethodCallManager(SOPC_MethodCallManager* mcm)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == mcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    pConfig->serverConfig.mcm = mcm;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetWriteNotifCallback(SOPC_WriteNotif_Fct* writeNotifCb)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == writeNotifCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_server_helper_config.writeNotifCb = writeNotifCb;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetMonitItemNodeAvailCallback(SOPC_CreateMI_NodeAvail_Fct* nodeAvailCb)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == nodeAvailCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    pConfig->serverConfig.nodeAvailFunc = nodeAvailCb;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(SOPC_LocalServiceAsyncResp_Fct* asyncRespCb)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == asyncRespCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_server_helper_config.asyncRespCb = asyncRespCb;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetShutdownCountdown(uint16_t secondsTillShutdown)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    sopc_server_helper_config.configuredSecondsTillShutdown = secondsTillShutdown;
    return SOPC_STATUS_OK;
}

void SOPC_ServerInternal_ClearEndpoint(SOPC_Endpoint_Config* epConfig)
{
    if (NULL == epConfig)
    {
        return;
    }
    SOPC_Free(epConfig->endpointURL);
    for (int i = 0; i < epConfig->nbClientsToConnect && i < SOPC_MAX_REVERSE_CLIENT_CONNECTIONS; i++)
    {
        SOPC_Free(epConfig->clientsToConnect[i].clientApplicationURI);
        SOPC_Free(epConfig->clientsToConnect[i].clientEndpointURL);
    }
    for (int i = 0; i < epConfig->nbSecuConfigs && i < SOPC_MAX_SECU_POLICIES_CFG; i++)
    {
        SOPC_String_Clear(&epConfig->secuConfigurations[i].securityPolicy);
        for (int j = 0; j < epConfig->secuConfigurations[i].nbOfUserTokenPolicies && j < SOPC_MAX_SECU_POLICIES_CFG;
             j++)
        {
            OpcUa_UserTokenPolicy_Clear(&epConfig->secuConfigurations[i].userTokenPolicies[j]);
        }
    }
    // Do not clear user managers since it is managed in a global way in high level API
}
