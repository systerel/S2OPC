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

#include "libs2opc_server_config.h"
#include "libs2opc_server_config_internal.h"

#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config.h"

/* Internal configuration structure and functions */
static void SOPC_Helper_ComEventCb(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t helperContext);

#define INITIAL_HELPER_CONFIG                                                                            \
    {                                                                                                    \
        .initialized = (int32_t) false, .locked = (int32_t) false, .comEventCb = SOPC_Helper_ComEventCb, \
                                                                                                         \
        .server = {                                                                                      \
            .addressSpace = NULL,                                                                        \
            .writeNotifCb = NULL,                                                                        \
            .asyncRespCb = NULL,                                                                         \
            .syncLocalServiceId = 0,                                                                     \
            .syncResp = NULL,                                                                            \
            .serverRequestedToStop = false,                                                              \
            .serverAllEndpointsClosed = false,                                                           \
            .serverStoppedStatus = SOPC_STATUS_OK,                                                       \
            .stoppedCb = NULL,                                                                           \
            .secondsTillShutdown = DEFAULT_SHUTDOWN_PHASE_IN_SECONDS,                                    \
            .authenticationManager = NULL,                                                               \
            .authorizationManager = NULL,                                                                \
            .manufacturerName = "Systerel",                                                              \
            .nbEndpoints = 0,                                                                            \
            .endpointIndexes = NULL,                                                                     \
            .endpointOpened = NULL,                                                                      \
            .started = false,                                                                            \
        }                                                                                                \
    }

const SOPC_Helper_Config sopc_helper_config_default = INITIAL_HELPER_CONFIG;

SOPC_Helper_Config sopc_helper_config = INITIAL_HELPER_CONFIG;

// Initialize sopc_helper_config
static void SOPC_HelperConfigInternal_Initialize(void);
// Clear sopc_helper_config
static void SOPC_HelperConfigInternal_Clear(void);

// Manage configuration state
bool SOPC_HelperConfig_IsInitAndUnlock(void)
{
    return SOPC_Atomic_Int_Get(&sopc_helper_config.initialized) && !SOPC_Atomic_Int_Get(&sopc_helper_config.locked);
}

bool SOPC_HelperConfig_IsInitAndLock(void)
{
    return SOPC_Atomic_Int_Get(&sopc_helper_config.initialized) && SOPC_Atomic_Int_Get(&sopc_helper_config.locked);
}

// Check configuration is correct
static bool SOPC_HelperConfigServer_CheckConfig(void)
{
    bool res = sopc_helper_config.config.serverConfig.nbEndpoints > 0;
    if (!res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "At least 1 endpoint shall be defined to start a server");
    }
    bool hasUserName = false;
    bool hasSecurity = false;
    uint8_t nbDiscovery = 0;
    for (uint8_t i = 0; i < sopc_helper_config.config.serverConfig.nbEndpoints; i++)
    {
        SOPC_Endpoint_Config* ep = &sopc_helper_config.config.serverConfig.endpoints[i];
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
                    hasUserName = true;
                }
            }
        }

        // Is this a discovery endpoint ? Add it to application description.
        if (ep->hasDiscoveryEndpoint)
        {
            nbDiscovery++;
        }
        // Other verifications to be done by SOPC_ToolkitServer_AddEndpointConfig
    }
    // Check that the server define certificates and PKI provide if endpoint use security
    if (hasSecurity)
    {
        if (NULL == sopc_helper_config.config.serverConfig.serverCertificate ||
            NULL == sopc_helper_config.config.serverConfig.serverKey)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Server key and certificates not defined whereas some endpoint(s) are defined as secured.");
            res = false;
        }

        if (NULL == sopc_helper_config.config.serverConfig.pki)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "No server PKI provider defined whereas some endpoint(s) are defined as secured.");
            res = false;
        }
    }
    // Check that the server define user managers
    if (hasUserName && (NULL == sopc_helper_config.server.authenticationManager ||
                        NULL == sopc_helper_config.server.authorizationManager))
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "No authentication and/or authorization user manager defined."
                                 " Default will be permissive whereas UserName policy is used in endpoint(s).");
    }

    if (NULL == sopc_helper_config.server.authenticationManager)
    {
        sopc_helper_config.server.authenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
        if (NULL == sopc_helper_config.server.authenticationManager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create a default authentication manager");
            res = false;
        }
    }
    if (NULL == sopc_helper_config.server.authorizationManager)
    {
        sopc_helper_config.server.authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == sopc_helper_config.server.authorizationManager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to create a default authorization manager");
            res = false;
        }
    }

    if (NULL == sopc_helper_config.config.serverConfig.namespaces)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "No namespace defined for the server, 1 server namespace shall be defined");
        res = false;
    }

    if (NULL == sopc_helper_config.config.serverConfig.localeIds)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "No locales defined for the server.");
        // Create empty locale array
        sopc_helper_config.config.serverConfig.localeIds = SOPC_Calloc(1, sizeof(char*));
        assert(NULL != sopc_helper_config.config.serverConfig.localeIds);
        sopc_helper_config.config.serverConfig.localeIds[0] = NULL;
    }

    // If some discovery endpoints are present and none is described in application description, add them into it
    if (res && nbDiscovery > 0 && 0 == sopc_helper_config.config.serverConfig.serverDescription.NoOfDiscoveryUrls)
    {
        OpcUa_ApplicationDescription* appDesc = &sopc_helper_config.config.serverConfig.serverDescription;
        appDesc->DiscoveryUrls = SOPC_Calloc((size_t) nbDiscovery, sizeof(SOPC_String));
        if (NULL != appDesc->DiscoveryUrls)
        {
            appDesc->NoOfDiscoveryUrls = nbDiscovery;
            for (uint8_t i = 0; i < nbDiscovery; i++)
            {
                SOPC_String_Initialize(&appDesc->DiscoveryUrls[i]);
            }
            uint8_t j = 0;
            for (uint8_t i = 0; i < sopc_helper_config.config.serverConfig.nbEndpoints && j < nbDiscovery; i++)
            {
                SOPC_Endpoint_Config* ep = &sopc_helper_config.config.serverConfig.endpoints[i];
                // Is this a discovery endpoint ? Add it to application description.
                if (ep->hasDiscoveryEndpoint)
                {
                    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&appDesc->DiscoveryUrls[j], ep->endpointURL);
                    assert(SOPC_STATUS_OK == status);
                    j++;
                }
            }
        }
    }
    // TODO: checks on application description content ?

    return res;
}

// Lock the configuration and check for configuration issues
bool SOPC_HelperConfig_LockState(void)
{
    if (!SOPC_Atomic_Int_Get(&sopc_helper_config.initialized))
    {
        return false;
    }
    if (SOPC_Atomic_Int_Get(&sopc_helper_config.locked))
    {
        // Already locked
        return true;
    }

    bool res = SOPC_HelperConfigServer_CheckConfig();
    // Only locks if succeeded
    SOPC_Atomic_Int_Set(&sopc_helper_config.locked, (int32_t) res);
    return res;
}

// Manage global user manager vs user manager by endpoint in low-level API

static SOPC_ReturnStatus SOPC_HelperConfigInternal_Authen_ValidateUser(
    SOPC_UserAuthentication_Manager* authenticationManager,
    const SOPC_ExtensionObject* pUser,
    SOPC_UserAuthentication_Status* pUserAuthenticated)
{
    // HelperConfig shall have defined a default user manager if none defined
    assert(NULL != sopc_helper_config.server.authenticationManager);

    return sopc_helper_config.server.authenticationManager->pFunctions->pFuncValidateUserIdentity(
        authenticationManager, pUser, pUserAuthenticated);
}

static void SOPC_HelperConfigInternal_Authen_Free(SOPC_UserAuthentication_Manager* authenticationManager)
{
    // Note: global authentication manager freed by SOPC_HelperConfigInternal_Clear
    (void) authenticationManager;
}

static SOPC_ReturnStatus SOPC_HelperConfigInternal_AuthorizeOperation(
    SOPC_UserAuthorization_Manager* authorizationManager,
    SOPC_UserAuthorization_OperationType operationType,
    const SOPC_NodeId* nodeId,
    uint32_t attributeId,
    const SOPC_User* pUser,
    bool* pbOperationAuthorized)
{
    // HelperConfig shall have defined a default user manager if none defined
    assert(NULL != sopc_helper_config.server.authorizationManager);
    return sopc_helper_config.server.authorizationManager->pFunctions->pFuncAuthorizeOperation(
        authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized);
}

static void SOPC_HelperConfigInternal_Autho_Free(SOPC_UserAuthorization_Manager* authorizationManager)
{
    // Note: global authorization manager freed by SOPC_HelperConfigInternal_Clear
    (void) authorizationManager;
}

static const SOPC_UserAuthentication_Functions authentication_functions = {
    .pFuncFree = SOPC_HelperConfigInternal_Authen_Free,
    .pFuncValidateUserIdentity = SOPC_HelperConfigInternal_Authen_ValidateUser};

static SOPC_UserAuthentication_Manager authentication_manager = {
    .pFunctions = &authentication_functions,
    .pData = NULL,
};

static const SOPC_UserAuthorization_Functions authorization_functions = {
    .pFuncFree = SOPC_HelperConfigInternal_Autho_Free,
    .pFuncAuthorizeOperation = SOPC_HelperConfigInternal_AuthorizeOperation};

static SOPC_UserAuthorization_Manager authorization_manager = {
    .pFunctions = &authorization_functions,
    .pData = NULL,
};

void SOPC_HelperConfig_SetEndpointUserMgr(SOPC_Endpoint_Config* epConfig)
{
    epConfig->authenticationManager = &authentication_manager;
    epConfig->authorizationManager = &authorization_manager;
}

void SOPC_HelperConfig_SetEndpointsUserMgr(void)
{
    // Ensure custom user manager data is correctly set
    if (NULL != sopc_helper_config.server.authorizationManager)
    {
        authorization_manager.pData = sopc_helper_config.server.authorizationManager->pData;
    }
    if (NULL != sopc_helper_config.server.authenticationManager)
    {
        authentication_manager.pData = sopc_helper_config.server.authenticationManager->pData;
    }

    for (uint8_t i = 0; i < sopc_helper_config.config.serverConfig.nbEndpoints; i++)
    {
        SOPC_HelperConfig_SetEndpointUserMgr(&sopc_helper_config.config.serverConfig.endpoints[i]);
    }
}

void SOPC_Helper_ComEventCb(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t helperContext)
{
    // Avoid unused parameter warning
    (void) IdOrStatus;

    SOPC_HelperConfigInternal_Ctx* ctx;
    switch (event)
    {
    /* Client events not managed for now */
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_ACTIVATED_SESSION:
    case SE_SESSION_REACTIVATING:
    case SE_RCV_SESSION_RESPONSE:
    case SE_CLOSED_SESSION:
    case SE_RCV_DISCOVERY_RESPONSE:
    case SE_SND_REQUEST_FAILED:
        if (NULL != sopc_helper_config.client.clientComEventCb)
        {
            sopc_helper_config.client.clientComEventCb(event, IdOrStatus, param, helperContext);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Error: server wrapper cannot manage client event %d\n", event);
        }
        break;
    /* Server events all managed */
    case SE_CLOSED_ENDPOINT:
        // Cases to manage:
        // - Unexpected closed endpoint: close others, do we need to do shutdown phase here ?
        // - Expected closed endpoint: determine when all endpoints are closed
        SOPC_HelperInternal_ClosedEndpoint(IdOrStatus, (SOPC_ReturnStatus) helperContext);
        break;
    case SE_LOCAL_SERVICE_RESPONSE:
        ctx = (SOPC_HelperConfigInternal_Ctx*) helperContext;
        assert(event == ctx->event);
        if (ctx->eventCtx.localService.isSyncCall)
        {
            SOPC_HelperInternal_SyncLocalServiceCb(*(SOPC_EncodeableType**) param, param, ctx);
        }
        else
        {
            SOPC_HelperInternal_AsyncLocalServiceCb(*(SOPC_EncodeableType**) param, param, ctx);
        }
        SOPC_Free(ctx);
        break;
    default:
        assert(false);
    }
}

static void SOPC_HelperConfigInternal_Initialize(void)
{
    sopc_helper_config = sopc_helper_config_default;
    SOPC_S2OPC_Config_Initialize(&sopc_helper_config.config);
    // We only do copies in helper config
    sopc_helper_config.config.serverConfig.freeCstringsFlag = true;
    SOPC_Atomic_Int_Set(&sopc_helper_config.initialized, (int32_t) true);
    Condition_Init(&sopc_helper_config.server.syncLocalServiceCond);
    Mutex_Initialization(&sopc_helper_config.server.syncLocalServiceMutex);
    Condition_Init(&sopc_helper_config.server.serverStoppedCond);
    Mutex_Initialization(&sopc_helper_config.server.serverStoppedMutex);
}

static void SOPC_HelperConfigInternal_Clear(void)
{
    SOPC_S2OPC_Config_Clear(&sopc_helper_config.config);
    SOPC_AddressSpace_Delete(sopc_helper_config.server.addressSpace);
    sopc_helper_config.server.addressSpace = NULL;
    Condition_Clear(&sopc_helper_config.server.syncLocalServiceCond);
    Mutex_Clear(&sopc_helper_config.server.syncLocalServiceMutex);
    Condition_Clear(&sopc_helper_config.server.serverStoppedCond);
    Mutex_Clear(&sopc_helper_config.server.serverStoppedMutex);
    SOPC_UserAuthentication_FreeManager(&sopc_helper_config.server.authenticationManager);
    SOPC_UserAuthorization_FreeManager(&sopc_helper_config.server.authorizationManager);
    SOPC_Free(sopc_helper_config.server.endpointIndexes);
    SOPC_Free(sopc_helper_config.server.endpointOpened);
    SOPC_Atomic_Int_Set(&sopc_helper_config.initialized, (int32_t) false);
    SOPC_Atomic_Int_Set(&sopc_helper_config.locked, (int32_t) false);
}

SOPC_ReturnStatus SOPC_Helper_Initialize(SOPC_Log_Configuration* optLogConfig)
{
    if (SOPC_Atomic_Int_Get(&sopc_helper_config.initialized))
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_HelperConfigInternal_Initialize();

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != optLogConfig)
    {
        status = SOPC_Common_Initialize(*optLogConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(sopc_helper_config.comEventCb);
    }
    return status;
}

void SOPC_Helper_Clear(void)
{
    SOPC_HelperConfigInternal_Clear();
    SOPC_Toolkit_Clear();
}

SOPC_Toolkit_Build_Info SOPC_Helper_GetBuildInfo(void)
{
    return SOPC_ToolkitConfig_GetBuildInfo();
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetMethodCallManager(SOPC_MethodCallManager* mcm)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == mcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.config.serverConfig.mcm = mcm;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetWriteNotifCallback(SOPC_WriteNotif_Fct* writeNotifCb)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == writeNotifCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.server.writeNotifCb = writeNotifCb;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(SOPC_LocalServiceAsyncResp_Fct* asyncRespCb)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == asyncRespCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.server.asyncRespCb = asyncRespCb;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetShutdownCountdown(uint16_t secondsTillShutdown)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    sopc_helper_config.server.secondsTillShutdown = secondsTillShutdown;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigClient_SetRawClientComEvent(SOPC_ComEvent_Fct* clientComEvtCb)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    sopc_helper_config.client.clientComEventCb = clientComEvtCb;
    return SOPC_STATUS_OK;
}
