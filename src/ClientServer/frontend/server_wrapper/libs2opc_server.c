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

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_server.h"
#include "libs2opc_server_internal.h"

#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
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
    SOPC_UNUSED_ARG(sig);

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

void SOPC_ServerInternal_SyncLocalServiceCb(SOPC_EncodeableType* encType,
                                            void* response,
                                            SOPC_HelperConfigInternal_Ctx* helperCtx)
{
    struct LocalServiceCtx* ls = &(helperCtx->eventCtx.localService);
    // Helper internal call to internal services are always using asynchronous way
    assert(!ls->isHelperInternal);
    Mutex_Lock(&sopc_server_helper_config.syncLocalServiceMutex);
    // Chech synchronous response id is the one expected
    if (ls->syncId != sopc_server_helper_config.syncLocalServiceId)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Received unexpected synchronous local service response: %s\n",
                                 SOPC_EncodeableType_GetName(encType));
    }
    else
    {
        // Move content of response into synchronous response to avoid deallocation on return by toolkit
        SOPC_ReturnStatus status = SOPC_Encodeable_Create(encType, &sopc_server_helper_config.syncResp);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Encodeable_Move(sopc_server_helper_config.syncResp, response);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Issue %d treating synchronous local service response: %s\n", (int) status,
                                   SOPC_EncodeableType_GetName(encType));
        }
        else
        {
            Condition_SignalAll(&sopc_server_helper_config.syncLocalServiceCond);
        }
    }
    Mutex_Unlock(&sopc_server_helper_config.syncLocalServiceMutex);
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
                               helperCtx->eventCtx.localService.internalErrorMsg);
    }
}

void SOPC_ServerInternal_AsyncLocalServiceCb(SOPC_EncodeableType* encType,
                                             void* response,
                                             SOPC_HelperConfigInternal_Ctx* helperCtx)
{
    struct LocalServiceCtx* ls = &helperCtx->eventCtx.localService;
    // Helper internal call to internal services are always using asynchronous way
    if (ls->isHelperInternal)
    {
        SOPC_HelperInternal_RuntimeVariableSetResponseCb(encType, response, (uintptr_t) helperCtx);
    }
    else if (NULL != sopc_server_helper_config.asyncRespCb)
    {
        sopc_server_helper_config.asyncRespCb(encType, response, helperCtx->userContext);
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const uint8_t nbEndpoints = sopc_server_helper_config.nbEndpoints;

    // Lock the helper state (verification and finalization of configuration)
    if (SOPC_STATUS_OK == status)
    {
        bool res = SOPC_ServerInternal_CheckConfigAndSetConfiguredState();
        status = (res ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE);
    }

    uint32_t* endpointIndexes = SOPC_Calloc((size_t) nbEndpoints, sizeof(uint32_t));
    bool* endpointClosed = SOPC_Calloc((size_t) nbEndpoints, sizeof(bool));
    if (NULL == endpointIndexes || NULL == endpointClosed)
    {
        SOPC_Free(endpointIndexes);
        SOPC_Free(endpointClosed);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (uint8_t i = 0; SOPC_STATUS_OK == status && i < nbEndpoints; i++)
    {
        uint32_t epIdx = SOPC_ToolkitServer_AddEndpointConfig(sopc_server_helper_config.endpoints[i]);
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
                                   i, sopc_server_helper_config.endpoints[i]->endpointURL);
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_Configured();
    }

    if (SOPC_STATUS_OK == status)
    {
        sopc_server_helper_config.nbEndpoints = nbEndpoints;
        sopc_server_helper_config.endpointIndexes = endpointIndexes;
        sopc_server_helper_config.endpointClosed = endpointClosed;
    }
    else
    {
        SOPC_Free(endpointIndexes);
        SOPC_Free(endpointClosed);
    }

    return status;
}

// Build and update server runtime variables (Server node info) and request to open all endpoints of the server
static SOPC_ReturnStatus SOPC_HelperInternal_OpenEndpoints(void)
{
    if (0 == sopc_server_helper_config.nbEndpoints)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Set runtime variable
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == sopc_server_helper_config.buildInfo)
    {
        sopc_server_helper_config.runtimeVariables =
            SOPC_RuntimeVariables_BuildDefault(SOPC_ToolkitConfig_GetBuildInfo(), &pConfig->serverConfig);
    }
    else
    {
        SOPC_RuntimeVariables_Build(sopc_server_helper_config.buildInfo, &pConfig->serverConfig);
    }
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);
    if (NULL != ctx)
    {
        ctx->eventCtx.localService.isHelperInternal = true;
        ctx->eventCtx.localService.internalErrorMsg =
            "Setting runtime variables of server build information nodes failed."
            " Please check address space content includes necessary base information nodes.";
        bool res = SOPC_RuntimeVariables_Set(sopc_server_helper_config.endpointIndexes[0],
                                             &sopc_server_helper_config.runtimeVariables, (uintptr_t) ctx);
        status = (res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Open the endpoints
    if (SOPC_STATUS_OK == status)
    {
        for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
        {
            SOPC_ToolkitServer_AsyncOpenEndpoint(sopc_server_helper_config.endpointIndexes[i]);
        }
    }

    return status;
}

// Update the server node shutdown information for shutdown phase (state and shutdown countdown)
// Returns when shutdown countdown is terminated
static void SOPC_HelperInternal_ShutdownPhaseServer(void)
{
    // The OPC UA server indicates it will shutdown during a few seconds and then actually stop

    SOPC_Server_RuntimeVariables* runtime_vars = &sopc_server_helper_config.runtimeVariables;
    // From part 5: "The server has shut down or is in the process of shutting down."
    runtime_vars->server_state = OpcUa_ServerState_Shutdown;
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&runtime_vars->shutdownReason.defaultLocale, "");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&runtime_vars->shutdownReason.defaultText, "Requested shutdown");
    }
    if (SOPC_STATUS_OK != status)
    {
        return;
    }

    SOPC_TimeReference targetTime = SOPC_TimeReference_GetCurrent() +
                                    (SOPC_TimeReference) sopc_server_helper_config.configuredSecondsTillShutdown * 1000;
    bool targetTimeReached = false;
    uint32_t remainingSecondsTillShutdown = sopc_server_helper_config.configuredSecondsTillShutdown;

    do
    {
        // Update the seconds till shutdown value
        runtime_vars->secondsTillShutdown = remainingSecondsTillShutdown;
        SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);
        if (NULL == ctx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == status)
        {
            ctx->eventCtx.localService.isHelperInternal = true;
            ctx->eventCtx.localService.internalErrorMsg =
                "Updating runtime variables of server build information nodes failed";
            if (!SOPC_RuntimeVariables_UpdateServerStatus(sopc_server_helper_config.endpointIndexes[0], runtime_vars,
                                                          (uintptr_t) ctx))
            {
                status = SOPC_STATUS_NOK;
            }

            // Evaluation of seconds till shutdown
            SOPC_TimeReference currentTime = SOPC_TimeReference_GetCurrent();
            if (currentTime < targetTime)
            {
                SOPC_Sleep(UPDATE_TIMEOUT_MS);
                remainingSecondsTillShutdown = (uint32_t)((targetTime - currentTime) / 1000);
            }
            else
            {
                targetTimeReached = true;
            }
        }
    } while (SOPC_STATUS_OK == status && !targetTimeReached);
}

// Request to close all endpoints of the server
static void SOPC_HelperInternal_ActualShutdownServer(void)
{
    for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
    {
        SOPC_ToolkitServer_AsyncCloseEndpoint(sopc_server_helper_config.endpointIndexes[i]);
    }
}

SOPC_ReturnStatus SOPC_ServerHelper_StartServer(SOPC_ServerStopped_Fct* stoppedCb)
{
    if (NULL == stoppedCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    sopc_server_helper_config.stoppedCb = stoppedCb;
    SOPC_ReturnStatus status = SOPC_HelperInternal_FinalizeToolkitConfiguration();
    // Set started stated prior to call OpenEndpoints to ensure event callback is already active
    if (SOPC_STATUS_OK == status && !SOPC_ServerInternal_SetStartedState())
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperInternal_OpenEndpoints();
    }

    return status;
}

void SOPC_ServerInternal_ClosedEndpoint(uint32_t epConfigIdx, SOPC_ReturnStatus status)
{
    uint8_t nbEndpoints = sopc_server_helper_config.nbEndpoints;
    bool allEndpointsClosed = true;
    if (SOPC_STATUS_OK != status && SOPC_STATUS_OK == sopc_server_helper_config.serverStoppedStatus)
    {
        // Only keep the fist error status notified, but all errors are logged below.
        sopc_server_helper_config.serverStoppedStatus = status;
    }
    for (uint8_t i = 0; i < nbEndpoints; i++)
    {
        if (epConfigIdx == sopc_server_helper_config.endpointIndexes[i])
        {
            if (SOPC_STATUS_OK != status && !sopc_server_helper_config.endpointClosed[i])
            {
                // Log an error on first endpoint close event if it is an error status
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Endpoint number %" PRIu8 " closed with error status: %d", i, status);
            }

            sopc_server_helper_config.endpointClosed[i] = true;
        }
        allEndpointsClosed &= sopc_server_helper_config.endpointClosed[i];
    }
    if (allEndpointsClosed)
    {
        // Server is considered stopped when no client connection possible anymore
        SOPC_UNUSED_RESULT(SOPC_ServerInternal_SetStoppedState());
        sopc_server_helper_config.stoppedCb(sopc_server_helper_config.serverStoppedStatus);
    }
}

// server stop function used when server running with ::SOPC_ServerHelper_Serve
static void SOPC_HelperInternal_SyncServerAsyncStop(bool allEndpointsAlreadyClosed)
{
    // use condition variable and let ::SOPC_ServerHelper_Serve manage shutdown
    SOPC_ReturnStatus status = Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    assert(SOPC_STATUS_OK == status);
    if (allEndpointsAlreadyClosed)
    {
        // Case in which server is stopping because all endpoints were closed before server requested to stop
        sopc_server_helper_config.syncServeStopData.serverAllEndpointsClosed = true;
    }
    SOPC_Atomic_Int_Set(&sopc_server_helper_config.syncServeStopData.serverRequestedToStop, true);
    status = Condition_SignalAll(&sopc_server_helper_config.syncServeStopData.serverStoppedCond);
    assert(SOPC_STATUS_OK == status);
    status = Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    assert(SOPC_STATUS_OK == status);
}

// server stopped callback used by ::SOPC_ServerHelper_Serve
static void SOPC_HelperInternal_SyncServerStoppedCb(SOPC_ReturnStatus stopStatus)
{
    if (SOPC_STATUS_OK != stopStatus)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Endpoint closed with error status: %d", stopStatus);
    }
    SOPC_HelperInternal_SyncServerAsyncStop(true);
}

SOPC_ReturnStatus SOPC_ServerHelper_StopServer(void)
{
    if (!SOPC_ServerInternal_SetStoppingState())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_HelperInternal_SyncServerStoppedCb == sopc_server_helper_config.stoppedCb)
    {
        // Since server is running synchronously with ::SOPC_ServerHelper_Serve, stop request is asynchronous
        SOPC_HelperInternal_SyncServerAsyncStop(false);
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
    SOPC_ReturnStatus status = SOPC_ServerHelper_StartServer(SOPC_HelperInternal_SyncServerStoppedCb);
    // If failed to start return immediately
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    if (catchSigStop)
    {
        // Install signal handler to close the server gracefully when server needs to stop
        signal(SIGINT, SOPC_HelperInternal_StopSignal);
        signal(SIGTERM, SOPC_HelperInternal_StopSignal);
    }

    // Waiting the server requested to stop
    if (catchSigStop)
    {
        // If we catch sig stop we have to check if the signal occurred regularly or if server is requested to stop
        // Note: we avoid recurrent use of mutex by accessing serverRequestedToStop in an atomic way
        while (!SOPC_Atomic_Int_Get(&sopc_server_helper_config.syncServeStopData.serverRequestedToStop) && !stopServer)
        {
            SOPC_Sleep(UPDATE_TIMEOUT_MS);
        }
    }
    else
    {
        status = Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);

        // If we don't catch sig stop we only have to wait for signal on condition variable (no timeout)
        // Note: we do not need to access serverRequestedToStop in an atomic way since assignment is protected by mutex
        while (SOPC_STATUS_OK == status && !sopc_server_helper_config.syncServeStopData.serverRequestedToStop)
        {
            status = Mutex_UnlockAndWaitCond(&sopc_server_helper_config.syncServeStopData.serverStoppedCond,
                                             &sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        }

        status = Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    }

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // Check if server is not already stopped before
    if (!SOPC_ServerInternal_IsStopped())
    {
        if (stopServer)
        {
            // Note: if stopping was not requested using StopServer API, set stopping state when force by sig stop
            SOPC_UNUSED_RESULT(SOPC_ServerInternal_SetStoppingState());
        }

        // Shutdown phase
        SOPC_HelperInternal_ShutdownPhaseServer();
        // Closing endpoints
        SOPC_HelperInternal_ActualShutdownServer();

        // Wait for all endpoints to close
        status = Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        while (SOPC_STATUS_OK == status && !sopc_server_helper_config.syncServeStopData.serverAllEndpointsClosed)
        {
            status = Mutex_UnlockAndWaitCond(&sopc_server_helper_config.syncServeStopData.serverStoppedCond,
                                             &sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        }
        status = Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceSync(void* request, void** response)
{
    if (NULL == request || NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ServerInternal_IsStarted())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Allocate local service helper context
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);

    if (NULL == ctx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    Mutex_Lock(&sopc_server_helper_config.syncLocalServiceMutex);
    assert(NULL == sopc_server_helper_config.syncResp);
    // Set helper local service context
    ctx->eventCtx.localService.isSyncCall = true;
    ctx->eventCtx.localService.syncId = sopc_server_helper_config.syncLocalServiceId;

    // Send request
    SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_server_helper_config.endpointIndexes[0], request, (uintptr_t) ctx);

    // Wait until response received or error status (timeout)
    while (SOPC_STATUS_OK == status && NULL == sopc_server_helper_config.syncResp)
    {
        status = Mutex_UnlockAndTimedWaitCond(&sopc_server_helper_config.syncLocalServiceCond,
                                              &sopc_server_helper_config.syncLocalServiceMutex,
                                              SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status)
    {
        assert(NULL != sopc_server_helper_config.syncResp);
        // Set response output
        *response = sopc_server_helper_config.syncResp;
    }
    else if (NULL != sopc_server_helper_config.syncResp)
    {
        // This is possible timeout occurred during response reception, in this case we might ignore timeout
        if (SOPC_STATUS_TIMEOUT == status)
        {
            // Set response output
            *response = sopc_server_helper_config.syncResp;
            status = SOPC_STATUS_OK;
        }
        else
        {
            // Clear response since result incorrect
            SOPC_EncodeableObject_Clear(*(SOPC_EncodeableType**) sopc_server_helper_config.syncResp,
                                        sopc_server_helper_config.syncResp);
        }
    }
    sopc_server_helper_config.syncResp = NULL;
    sopc_server_helper_config.syncLocalServiceId++;
    Mutex_Unlock(&sopc_server_helper_config.syncLocalServiceMutex);
    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_LocalServiceAsync(void* request, uintptr_t userContext)
{
    if (!SOPC_ServerInternal_IsStarted())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(userContext, SE_LOCAL_SERVICE_RESPONSE);
    if (NULL == ctx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_server_helper_config.endpointIndexes[0], request, (uintptr_t) ctx);

    return SOPC_STATUS_OK;
}
