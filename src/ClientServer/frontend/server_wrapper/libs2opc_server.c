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
#include <signal.h>
#include <stdlib.h>

#include "libs2opc_server.h"
#include "libs2opc_server_config_internal.h"

#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

// Periodic timeout used to check for catch signal or for updating seconds shutdown counter
#define UPDATE_TIMEOUT_MS 500

// Flag used on sig stop
volatile sig_atomic_t stopServer = 0;

/*
 * Management of Ctrl-C to stop the server (callback on stop signal)
 */
static void SOPC_HelperInternal_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    /*
     * Signal steps:
     * - 1st signal: activate server shutdown phase of OPC UA server
     * - 2rd signal: abrupt exit with error code '1'
     */
    if (stopServer > 0)
    {
        exit(1);
    }
    else
    {
        stopServer++;
    }
}

void SOPC_HelperInternal_SyncLocalServiceCb(SOPC_EncodeableType* encType,
                                            void* response,
                                            SOPC_HelperConfigInternal_Ctx* helperCtx)
{
    struct LocalServiceCtx* ls = &(helperCtx->eventCtx.localService);
    // Helper internal call to internal services are always using asynchronous way
    assert(!ls->isHelperInternal);
    Mutex_Lock(&sopc_helper_config.server.syncLocalServiceMutex);
    // Chech synchronous response id is the one expected
    if (ls->syncId != sopc_helper_config.server.syncLocalServiceId)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Received unexpected synchronous local service response: %s\n",
                                 SOPC_EncodeableType_GetName(encType));
    }
    else
    {
        // Move content of response into synchronous response to avoid deallocation on return by toolkit
        SOPC_ReturnStatus status = SOPC_Encodeable_Create(encType, &sopc_helper_config.server.syncResp);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Encodeable_Move(sopc_helper_config.server.syncResp, response);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Issue %d treating synchronous local service response: %s\n", (int) status,
                                   SOPC_EncodeableType_GetName(encType));
        }
        else
        {
            Condition_SignalAll(&sopc_helper_config.server.syncLocalServiceCond);
        }
    }
    Mutex_Unlock(&sopc_helper_config.server.syncLocalServiceMutex);
}

void SOPC_HelperInternal_AsyncLocalServiceCb(SOPC_EncodeableType* encType,
                                             void* response,
                                             SOPC_HelperConfigInternal_Ctx* helperCtx)
{
    struct LocalServiceCtx* ls = &helperCtx->eventCtx.localService;
    // Helper internal call to internal services are always using asynchronous way
    if (ls->isHelperInternal)
    {
        ls->helperInternalCb(encType, response, (uintptr_t) helperCtx);
    }
    else if (NULL != sopc_helper_config.server.asyncRespCb)
    {
        sopc_helper_config.server.asyncRespCb(encType, response, helperCtx->userContext);
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Received an asynchronous local service response without configured handler for response type %s."
            " Please check you configured an asynchronous local service response callback if you sent request.\n",
            SOPC_EncodeableType_GetName(encType));
    }
}

// Creates an helper context containing user context to associate with a sent request
static SOPC_HelperConfigInternal_Ctx* SOPC_HelperConfigInternalCtx_Create(uintptr_t userContext,
                                                                          SOPC_App_Com_Event event)
{
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_Calloc(1, sizeof(SOPC_HelperConfigInternal_Ctx));
    if (NULL != ctx)
    {
        ctx->userContext = userContext;
        ctx->event = event;
    }
    return ctx;
}

// Finalize the toolkit configuration based on helper configuration data.
// After this step configuration is not modifiable anymore.
static SOPC_ReturnStatus SOPC_HelperInternal_FinalizeToolkitConfiguration(void)
{
    SOPC_Server_Config* serverCfg = &sopc_helper_config.config.serverConfig;
    uint8_t nbEndpoints = serverCfg->nbEndpoints;
    if (0 == nbEndpoints)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Error no endpoint defined, at least one shall be defined");
        return SOPC_STATUS_INVALID_STATE;
    }
    uint32_t* endpointIndexes = SOPC_Calloc((size_t) nbEndpoints, sizeof(uint32_t));
    bool* endpointOpened = SOPC_Calloc((size_t) nbEndpoints, sizeof(bool));
    if (NULL == endpointIndexes || NULL == endpointOpened)
    {
        SOPC_Free(endpointIndexes);
        SOPC_Free(endpointOpened);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint8_t i = 0; SOPC_STATUS_OK == status && i < nbEndpoints; i++)
    {
        uint32_t epIdx = SOPC_ToolkitServer_AddEndpointConfig(&serverCfg->endpoints[i]);
        if (epIdx > 0)
        {
            endpointIndexes[i] = epIdx;
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Error configuring endpoint %" PRIu8
                                   ": %s."
                                   " Please check associated configuration data is coherent and complete.\n",
                                   i, serverCfg->endpoints[i].endpointURL);
            status = SOPC_STATUS_NOK;
        }
    }

    // Lock the helper state (verification and finalization of configuration)
    if (SOPC_STATUS_OK == status)
    {
        bool res = SOPC_HelperConfig_LockState();
        status = (res ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Configured();
    }

    if (SOPC_STATUS_OK == status)
    {
        sopc_helper_config.server.nbEndpoints = nbEndpoints;
        sopc_helper_config.server.endpointIndexes = endpointIndexes;
        sopc_helper_config.server.endpointOpened = endpointOpened;
    }
    else
    {
        SOPC_Free(endpointIndexes);
        SOPC_Free(endpointOpened);
    }

    return status;
}

// Callback dedicated to runtime variable update treatment: check received response is correct or trace error
static void SOPC_HelperInternal_RuntimeVariableSetResponseCb(SOPC_EncodeableType* encType,
                                                             void* response,
                                                             uintptr_t context)
{
    SOPC_HelperConfigInternal_Ctx* helperCtx = (SOPC_HelperConfigInternal_Ctx*) context;

    assert(&OpcUa_WriteResponse_EncodeableType == encType);
    OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
    bool ok = (writeResp->ResponseHeader.ServiceResult == SOPC_GoodGenericStatus);

    for (int32_t i = 0; i < writeResp->NoOfResults; ++i)
    {
        ok &= (writeResp->Results[i] == SOPC_GoodGenericStatus);
    }

    if (!ok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Error while updating address space: %s\n",
                               helperCtx->eventCtx.localService.errorMsg);
    }
}

// Build and update server runtime variables (Server node info) and request to open all endpoints of the server
static SOPC_ReturnStatus SOPC_HelperInternal_OpenEndpoints(void)
{
    if (0 == sopc_helper_config.server.nbEndpoints)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Set runtime variable
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    sopc_helper_config.server.runtimeVariables =
        SOPC_RuntimeVariables_Build(SOPC_ToolkitConfig_GetBuildInfo(), &sopc_helper_config.config.serverConfig,
                                    sopc_helper_config.server.manufacturerName);
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);
    if (NULL != ctx)
    {
        ctx->eventCtx.localService.isHelperInternal = true;
        ctx->eventCtx.localService.helperInternalCb = SOPC_HelperInternal_RuntimeVariableSetResponseCb;
        ctx->eventCtx.localService.errorMsg =
            "Setting runtime variables of server build information nodes failed."
            " Please check address space content includes necessary base information nodes.";
        bool res = SOPC_RuntimeVariables_Set(sopc_helper_config.server.endpointIndexes[0],
                                             &sopc_helper_config.server.runtimeVariables, (uintptr_t) ctx);
        status = (res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Open the endpoints
    if (SOPC_STATUS_OK == status)
    {
        for (uint8_t i = 0; i < sopc_helper_config.server.nbEndpoints; i++)
        {
            sopc_helper_config.server.endpointOpened[i] = true;
            SOPC_ToolkitServer_AsyncOpenEndpoint(sopc_helper_config.server.endpointIndexes[i]);
        }
        SOPC_Atomic_Int_Set(&sopc_helper_config.server.started, true);
    }

    return status;
}

// Update the server node shutdown information for shutdown phase (state and shutdown countdown)
// Returns when shutdown countdown is terminated
static void SOPC_HelperInternal_ShutdownPhaseServer(void)
{
    if (!SOPC_Atomic_Int_Get(&sopc_helper_config.server.started))
    {
        return;
    }

    // The OPC UA server indicates it will shutdown during a few seconds and then actually stop
    SOPC_TimeReference targetTime =
        SOPC_TimeReference_GetCurrent() + (SOPC_TimeReference) sopc_helper_config.server.secondsTillShutdown * 1000;
    bool targetTimeReached = false;
    SOPC_Server_RuntimeVariables* runtime_vars = &sopc_helper_config.server.runtimeVariables;
    // From part 5: "The server has shut down or is in the process of shutting down."
    runtime_vars->server_state = OpcUa_ServerState_Shutdown;
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&runtime_vars->shutdownReason.defaultLocale, "");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&runtime_vars->shutdownReason.defaultText, "Requested shutdown");
    }
    uint32_t secondsTillShutdown = sopc_helper_config.server.secondsTillShutdown;
    while (SOPC_STATUS_OK == status && !targetTimeReached)
    {
        // Update the seconds till shutdown value
        runtime_vars->secondsTillShutdown = secondsTillShutdown;
        SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);
        if (NULL == ctx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == status)
        {
            ctx->eventCtx.localService.isHelperInternal = true;
            ctx->eventCtx.localService.helperInternalCb = SOPC_HelperInternal_RuntimeVariableSetResponseCb;
            ctx->eventCtx.localService.errorMsg = "Updating runtime variables of server build information nodes failed";
            if (!SOPC_RuntimeVariables_UpdateServerStatus(sopc_helper_config.server.endpointIndexes[0], runtime_vars,
                                                          (uintptr_t) ctx))
            {
                status = SOPC_STATUS_NOK;
            }
            else if (sopc_helper_config.server.secondsTillShutdown > 0)
            {
                SOPC_Sleep(UPDATE_TIMEOUT_MS);
            }
            // Evaluation of seconds till shutdown
            SOPC_TimeReference currentTime = SOPC_TimeReference_GetCurrent();
            if (currentTime < targetTime)
            {
                secondsTillShutdown = (uint32_t)((targetTime - currentTime) / 1000);
            }
            else
            {
                targetTimeReached = true;
            }
        }
    }
}

// Request to close all endpoints of the server
static void SOPC_HelperInternal_ActualShutdownServer(void)
{
    for (uint8_t i = 0; i < sopc_helper_config.config.serverConfig.nbEndpoints; i++)
    {
        SOPC_ToolkitServer_AsyncCloseEndpoint(sopc_helper_config.server.endpointIndexes[i]);
    }
}

SOPC_ReturnStatus SOPC_ServerHelper_StartServer(SOPC_ServerStopped_Fct* stoppedCb)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    sopc_helper_config.server.stoppedCb = stoppedCb;
    SOPC_ReturnStatus status = SOPC_HelperInternal_FinalizeToolkitConfiguration();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperInternal_OpenEndpoints();
    }
    return status;
}

void SOPC_HelperInternal_ClosedEndpoint(uint32_t epConfigIdx, SOPC_ReturnStatus status)
{
    uint8_t nbEndpoints = sopc_helper_config.config.serverConfig.nbEndpoints;
    bool allEndpointsClosed = true;
    if (SOPC_STATUS_OK != status && SOPC_STATUS_OK == sopc_helper_config.server.serverStoppedStatus)
    {
        // Only keep the fist error status notified, but all errors are logged below.
        sopc_helper_config.server.serverStoppedStatus = status;
    }
    for (uint8_t i = 0; i < nbEndpoints; i++)
    {
        if (epConfigIdx == sopc_helper_config.server.endpointIndexes[i])
        {
            sopc_helper_config.server.endpointOpened[i] = false;
        }
        allEndpointsClosed &= !sopc_helper_config.server.endpointOpened[i];

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Endpoint number %" PRIu8 " closed with error status: %d", i, status);
        }
    }
    if (allEndpointsClosed)
    {
        sopc_helper_config.server.stoppedCb(sopc_helper_config.server.serverStoppedStatus);
    }
}

// server stopped callback used by ::SOPC_ServerHelper_Serve
static void SOPC_HelperInternal_SyncServerStoppedCb(SOPC_ReturnStatus stopStatus)
{
    (void) stopStatus;
    SOPC_ReturnStatus status = Mutex_Lock(&sopc_helper_config.server.serverStoppedMutex);
    assert(SOPC_STATUS_OK == status);
    sopc_helper_config.server.serverAllEndpointsClosed = true;
    // Wake up SOPC_ServerHelper_Serve waiting for update
    status = Condition_SignalAll(&sopc_helper_config.server.serverStoppedCond);
    assert(SOPC_STATUS_OK == status);
    status = Mutex_Unlock(&sopc_helper_config.server.serverStoppedMutex);
    assert(SOPC_STATUS_OK == status);
}

SOPC_ReturnStatus SOPC_ServerHelper_StopServer(void)
{
    if (!SOPC_HelperConfig_IsInitAndLock() || !SOPC_Atomic_Int_Get(&sopc_helper_config.server.started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_HelperInternal_SyncServerStoppedCb == sopc_helper_config.server.stoppedCb)
    {
        // use condition variable and let ::SOPC_ServerHelper_Serve manage shutdown
        status = Mutex_Lock(&sopc_helper_config.server.serverStoppedMutex);
        assert(SOPC_STATUS_OK == status);
        sopc_helper_config.server.serverRequestedToStop = true;
        status = Condition_SignalAll(&sopc_helper_config.server.serverStoppedCond);
        assert(SOPC_STATUS_OK == status);
        status = Mutex_Unlock(&sopc_helper_config.server.serverStoppedMutex);
        assert(SOPC_STATUS_OK == status);
    }
    else
    {
        // since server was started using ::SOPC_ServerHelper_StartServer we shall shutdown in synchronous way
        SOPC_HelperInternal_ShutdownPhaseServer();
        SOPC_HelperInternal_ActualShutdownServer();

        // then the stopped callback will be called when server actually stopped
    }
    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_Serve(bool catchSigStop)
{
    if (catchSigStop)
    {
        // Install signal handler to close the server gracefully when server needs to stop
        signal(SIGINT, SOPC_HelperInternal_StopSignal);
        signal(SIGTERM, SOPC_HelperInternal_StopSignal);
    }
    SOPC_ReturnStatus status = SOPC_ServerHelper_StartServer(SOPC_HelperInternal_SyncServerStoppedCb);
    // If failed to start return immediately
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = Mutex_Lock(&sopc_helper_config.server.serverStoppedMutex);
    if (catchSigStop)
    {
        // If we catch sig stop we have to check if the signal occured regularly (timeout cond)
        while ((SOPC_STATUS_OK == status || SOPC_STATUS_TIMEOUT == status) &&
               !sopc_helper_config.server.serverRequestedToStop && !stopServer)
        {
            status = Mutex_UnlockAndTimedWaitCond(&sopc_helper_config.server.serverStoppedCond,
                                                  &sopc_helper_config.server.serverStoppedMutex, UPDATE_TIMEOUT_MS);
        }
    }
    else
    {
        // If we don't catch sig stop we only have to wait for signal on condition variable (no timeout)
        while (SOPC_STATUS_OK == status && !sopc_helper_config.server.serverRequestedToStop)
        {
            status = Mutex_UnlockAndWaitCond(&sopc_helper_config.server.serverStoppedCond,
                                             &sopc_helper_config.server.serverStoppedMutex);
        }
    }
    status = Mutex_Unlock(&sopc_helper_config.server.serverStoppedMutex);

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // Shutdown phase
    SOPC_HelperInternal_ShutdownPhaseServer();
    // Closing endpoints
    SOPC_HelperInternal_ActualShutdownServer();

    // Wait for all endpoints to close
    status = Mutex_Lock(&sopc_helper_config.server.serverStoppedMutex);
    while (SOPC_STATUS_OK == status && !sopc_helper_config.server.serverAllEndpointsClosed)
    {
        status = Mutex_UnlockAndWaitCond(&sopc_helper_config.server.serverStoppedCond,
                                         &sopc_helper_config.server.serverStoppedMutex);
    }
    status = Mutex_Unlock(&sopc_helper_config.server.serverStoppedMutex);

    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceSync(void* request, void** response)
{
    if (NULL == request || NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_HelperConfig_IsInitAndLock() || !SOPC_Atomic_Int_Get(&sopc_helper_config.server.started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Allocate local service helper context
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_Calloc(1, sizeof(SOPC_HelperConfigInternal_Ctx));
    if (NULL == ctx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    Mutex_Lock(&sopc_helper_config.server.syncLocalServiceMutex);
    assert(NULL == sopc_helper_config.server.syncResp);
    // Set helper local service context
    ctx->event = SE_LOCAL_SERVICE_RESPONSE;
    ctx->eventCtx.localService.isSyncCall = true;
    ctx->eventCtx.localService.syncId = sopc_helper_config.server.syncLocalServiceId;

    // Send request
    SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_helper_config.server.endpointIndexes[0], request, (uintptr_t) ctx);

    // Wait until response received or timeout
    while (SOPC_STATUS_OK == status && NULL == sopc_helper_config.server.syncResp)
    {
        status = Mutex_UnlockAndTimedWaitCond(&sopc_helper_config.server.syncLocalServiceCond,
                                              &sopc_helper_config.server.syncLocalServiceMutex,
                                              SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status)
    {
        assert(NULL != sopc_helper_config.server.syncResp);
        // Set response output
        *response = sopc_helper_config.server.syncResp;
    }
    else if (NULL != sopc_helper_config.server.syncResp)
    {
        // Clear response since result incorrect
        SOPC_EncodeableObject_Clear(*(SOPC_EncodeableType**) sopc_helper_config.server.syncResp,
                                    sopc_helper_config.server.syncResp);
    }
    sopc_helper_config.server.syncResp = NULL;
    sopc_helper_config.server.syncLocalServiceId++;
    Mutex_Unlock(&sopc_helper_config.server.syncLocalServiceMutex);
    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceAsync(void* request, uintptr_t userContext)
{
    if (!SOPC_HelperConfig_IsInitAndLock() || !SOPC_Atomic_Int_Get(&sopc_helper_config.server.started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(userContext, SE_LOCAL_SERVICE_RESPONSE);
    if (NULL == ctx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_helper_config.server.endpointIndexes[0], request, (uintptr_t) ctx);

    return SOPC_STATUS_OK;
}
