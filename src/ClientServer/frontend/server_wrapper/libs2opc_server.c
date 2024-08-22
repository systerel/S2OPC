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

#include <signal.h>
#include <stdlib.h>

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_server.h"
#include "libs2opc_server_internal.h"

#include "opcua_identifiers.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_encodeabletype.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

// Periodic timeout used to check for catch signal or for updating seconds shutdown counter
#define UPDATE_TIMEOUT_MS 500

// Flag used on sig stop
static int32_t stopServer = 0;

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

bool SOPC_ServerInternal_GetKeyPassword(char** outPassword)
{
    if (NULL == sopc_server_helper_config.getServerKeyPassword)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "The following user callback is missing: SOPC_GetServerKeyPassword_Fct");
        return false;
    }
    bool res = sopc_server_helper_config.getServerKeyPassword(outPassword);
    return res;
}

void SOPC_ServerInternal_SyncLocalServiceCb(SOPC_EncodeableType* encType,
                                            void* response,
                                            SOPC_HelperConfigInternal_Ctx* helperCtx)
{
    struct LocalServiceCtx* ls = &(helperCtx->eventCtx.localService);
    // Helper internal call to internal services are always using asynchronous way
    SOPC_ASSERT(!ls->isHelperInternal);
    SOPC_Mutex_Lock(&sopc_server_helper_config.syncLocalServiceMutex);
    // Chech synchronous response id is the one expected
    if (ls->syncId != sopc_server_helper_config.syncLocalServiceId)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Received unexpected synchronous local service response: %s",
                                 SOPC_EncodeableType_GetName(encType));
    }
    else
    {
        // Move content of response into synchronous response to avoid deallocation on return by toolkit
        SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(encType, &sopc_server_helper_config.syncResp);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Move(sopc_server_helper_config.syncResp, response);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_EncodeableObject_Delete(encType, &sopc_server_helper_config.syncResp);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Issue %d treating synchronous local service response: %s", (int) status,
                                   SOPC_EncodeableType_GetName(encType));
        }
        else
        {
            SOPC_Condition_SignalAll(&sopc_server_helper_config.syncLocalServiceCond);
        }
    }
    SOPC_Mutex_Unlock(&sopc_server_helper_config.syncLocalServiceMutex);
}

// Callback dedicated to runtime variable update treatment: check received response is correct or trace error
static void SOPC_HelperInternal_RuntimeVariableSetResponseCb(SOPC_EncodeableType* encType,
                                                             void* response,
                                                             uintptr_t context)
{
    SOPC_HelperConfigInternal_Ctx* helperCtx = (SOPC_HelperConfigInternal_Ctx*) context;

    SOPC_ASSERT(&OpcUa_WriteResponse_EncodeableType == encType);
    OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
    OpcUa_WriteRequest* writeReqCtx = (OpcUa_WriteRequest*) helperCtx->userContext;
    bool ok = (SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult));

    for (int32_t i = 0; ok && i < writeResp->NoOfResults; ++i)
    {
        ok &= SOPC_IsGoodStatus(writeResp->Results[i]);
    }

    if (!ok && NULL != writeReqCtx)
    {
        // Only display warning if we have context information to display node
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Error while updating address space: %s",
                                 helperCtx->eventCtx.localService.internalErrorMsg);
        for (int32_t i = 0; i < writeResp->NoOfResults && i < writeReqCtx->NoOfNodesToWrite; ++i)
        {
            if (!SOPC_IsGoodStatus(writeResp->Results[i]))
            {
                char* nodeIdStr = SOPC_NodeId_ToCString(&writeReqCtx->NodesToWrite[i].NodeId);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "- Writing runtime variable %s failed with status 0x%" PRIX32, nodeIdStr,
                                         writeResp->Results[i]);
                SOPC_Free(nodeIdStr);
            }
        }
    }

    if (NULL != writeReqCtx)
    {
        OpcUa_WriteRequest_Clear((void*) writeReqCtx);
        SOPC_Free(writeReqCtx);
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
            " Please check you configured an asynchronous local service response callback if you sent request.",
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

    uint32_t* endpointIndexes = NULL;
    bool* endpointClosed = NULL;

    if (SOPC_STATUS_OK == status)
    {
        endpointIndexes = SOPC_Calloc((size_t) nbEndpoints, sizeof(uint32_t));
        endpointClosed = SOPC_Calloc((size_t) nbEndpoints, sizeof(bool));
        status = (NULL == endpointIndexes || NULL == endpointClosed) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
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
                                   " Please check associated configuration data is coherent and complete.",
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

static SOPC_ReturnStatus SOPC_HelperInternal_SendWriteRequestWithCopyInCtx(OpcUa_WriteRequest* writeRequest,
                                                                           SOPC_HelperConfigInternal_Ctx* ctx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != writeRequest)
    {
        OpcUa_WriteRequest* writeRequestCopyCtx = NULL;
        status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &writeRequestCopyCtx);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Copy(&OpcUa_WriteRequest_EncodeableType, writeRequestCopyCtx, writeRequest);
        }
        if (SOPC_STATUS_OK == status)
        {
            ctx->userContext = (uintptr_t) writeRequestCopyCtx;
            SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_server_helper_config.endpointIndexes[0], writeRequest,
                                                        (uintptr_t) ctx);
        }
        else
        {
            OpcUa_WriteRequest_Clear(writeRequest);
            SOPC_Free(writeRequest);
            SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &writeRequestCopyCtx);
        }
    }

    return status;
}

static void SOPC_UpdateCurrentTime_EventHandler_Callback(SOPC_EventHandler* handler,
                                                         int32_t event,
                                                         uint32_t eltId,
                                                         uintptr_t params,
                                                         uintptr_t auxParam)
{
    SOPC_ASSERT(OpcUaId_Server_ServerStatus_CurrentTime == event);
    SOPC_ASSERT(OpcUaId_Server_ServerStatus_CurrentTime == eltId);
    SOPC_UNUSED_ARG(handler);
    SOPC_UNUSED_ARG(params);
    SOPC_UNUSED_ARG(auxParam);
    SOPC_HelperConfigInternal_Ctx* ctx = SOPC_HelperConfigInternalCtx_Create(0, SE_LOCAL_SERVICE_RESPONSE);
    if (NULL != ctx)
    {
        ctx->eventCtx.localService.isHelperInternal = true;
        ctx->eventCtx.localService.internalErrorMsg =
            "Updating server status current time runtime variables of server information nodes failed."
            " Please check address space content includes necessary base information nodes.";
        OpcUa_WriteRequest* writeRequest =
            SOPC_RuntimeVariables_UpdateCurrentTimeWriteRequest(&sopc_server_helper_config.runtimeVariables);
        if (NULL != writeRequest)
        {
            SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_server_helper_config.endpointIndexes[0], writeRequest,
                                                        (uintptr_t) ctx);
        }
        else
        {
            SOPC_Free(ctx);
        }
    }
}

// Build and update server runtime variables (Server node info) and request to open all endpoints of the server
static SOPC_ReturnStatus SOPC_HelperInternal_OpenEndpoints(void)
{
    if (0 == sopc_server_helper_config.nbEndpoints)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Set runtime variable
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
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
        OpcUa_WriteRequest* writeRequest =
            SOPC_RuntimeVariables_BuildWriteRequest(&sopc_server_helper_config.runtimeVariables);

        status = SOPC_HelperInternal_SendWriteRequestWithCopyInCtx(writeRequest, ctx);

        if (SOPC_STATUS_OK == status && 0 != sopc_server_helper_config.configuredCurrentTimeRefreshIntervalMs)
        {
            SOPC_Looper* appLooper = SOPC_App_GetLooper();
            SOPC_EventHandler* currentTimeHandler =
                SOPC_EventHandler_Create(appLooper, SOPC_UpdateCurrentTime_EventHandler_Callback);
            SOPC_LooperEvent currentTimeEvent = {OpcUaId_Server_ServerStatus_CurrentTime,
                                                 OpcUaId_Server_ServerStatus_CurrentTime, 0, 0};
            uint32_t currentTimeTimerId = SOPC_EventTimer_CreatePeriodic(
                currentTimeHandler, currentTimeEvent, sopc_server_helper_config.configuredCurrentTimeRefreshIntervalMs);
            if (0 == currentTimeTimerId)
            {
                SOPC_Logger_TraceWarning(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Timer creation to update server status current time failed, it will not be updated.");
            }
            sopc_server_helper_config.currentTimeRefreshTimerId = currentTimeTimerId;
        }
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
    // Stop the current time update timer
    SOPC_EventTimer_Cancel(sopc_server_helper_config.currentTimeRefreshTimerId);

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
            OpcUa_WriteRequest* writeRequest = SOPC_RuntimeVariables_BuildUpdateServerStatusWriteRequest(runtime_vars);

            status = SOPC_HelperInternal_SendWriteRequestWithCopyInCtx(writeRequest, ctx);

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
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (allEndpointsAlreadyClosed)
    {
        // Case in which server is stopping because all endpoints were closed before server requested to stop
        sopc_server_helper_config.syncServeStopData.serverAllEndpointsClosed = true;
    }
    SOPC_Atomic_Int_Set(&sopc_server_helper_config.syncServeStopData.serverRequestedToStop, true);
    status = SOPC_Condition_SignalAll(&sopc_server_helper_config.syncServeStopData.serverStoppedCond);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
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
        while (!SOPC_Atomic_Int_Get(&sopc_server_helper_config.syncServeStopData.serverRequestedToStop) &&
               !SOPC_Atomic_Int_Get(&stopServer))
        {
            SOPC_Sleep(UPDATE_TIMEOUT_MS);
        }
    }
    else
    {
        status = SOPC_Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);

        // If we don't catch sig stop we only have to wait for signal on condition variable (no timeout)
        // Note: we do not need to access serverRequestedToStop in an atomic way since assignment is protected by mutex
        while (SOPC_STATUS_OK == status && !sopc_server_helper_config.syncServeStopData.serverRequestedToStop)
        {
            status = SOPC_Mutex_UnlockAndWaitCond(&sopc_server_helper_config.syncServeStopData.serverStoppedCond,
                                                  &sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        }

        status = SOPC_Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
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
        status = SOPC_Mutex_Lock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        while (SOPC_STATUS_OK == status && !sopc_server_helper_config.syncServeStopData.serverAllEndpointsClosed)
        {
            status = SOPC_Mutex_UnlockAndWaitCond(&sopc_server_helper_config.syncServeStopData.serverStoppedCond,
                                                  &sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
        }
        status = SOPC_Mutex_Unlock(&sopc_server_helper_config.syncServeStopData.serverStoppedMutex);
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

    SOPC_Mutex_Lock(&sopc_server_helper_config.syncLocalServiceMutex);
    if (sopc_server_helper_config.syncCalled)
    {
        // Synchronous call already in execution
        status = SOPC_STATUS_INVALID_STATE;
        SOPC_Free(ctx);
        ctx = NULL;
    }
    else
    {
        sopc_server_helper_config.syncCalled = true;
        SOPC_ASSERT(NULL == sopc_server_helper_config.syncResp);
        // Set helper local service context
        ctx->eventCtx.localService.isSyncCall = true;
        ctx->eventCtx.localService.syncId = sopc_server_helper_config.syncLocalServiceId;

        // Send request
        SOPC_ToolkitServer_AsyncLocalServiceRequest(sopc_server_helper_config.endpointIndexes[0], request,
                                                    (uintptr_t) ctx);

        // Wait until response received or error status (timeout)
        while (SOPC_STATUS_OK == status && NULL == sopc_server_helper_config.syncResp)
        {
            status = SOPC_Mutex_UnlockAndTimedWaitCond(&sopc_server_helper_config.syncLocalServiceCond,
                                                       &sopc_server_helper_config.syncLocalServiceMutex,
                                                       SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS);
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(NULL != sopc_server_helper_config.syncResp);
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
        sopc_server_helper_config.syncCalled = false;
        sopc_server_helper_config.syncLocalServiceId++;
    }
    SOPC_Mutex_Unlock(&sopc_server_helper_config.syncLocalServiceMutex);
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

SOPC_ReturnStatus SOPC_ServerHelper_CreateEvent(const SOPC_NodeId* eventTypeId, SOPC_Event** event)
{
    if (NULL == eventTypeId || NULL == event)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ServerInternal_IsConfiguring() && !SOPC_ServerInternal_IsStarted())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
#if S2OPC_EVENT_MANAGEMENT
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (NULL == pConfig->serverConfig.eventTypes)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    *event = SOPC_EventManager_CreateEventInstance(pConfig->serverConfig.eventTypes, eventTypeId);
    status = (NULL == *event ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
#else
    status = SOPC_STATUS_NOT_SUPPORTED;
#endif
    return status;
}

SOPC_ReturnStatus SOPC_ServerHelper_TriggerEvent(const SOPC_NodeId* notifierNodeId,
                                                 SOPC_Event* event,
                                                 uint32_t optSubscriptionId,
                                                 uint32_t optMonitoredItemId)
{
    if (!SOPC_ServerInternal_IsStarted())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ToolkitServer_TriggerEvent(sopc_server_helper_config.endpointIndexes[0], notifierNodeId, event,
                                    optSubscriptionId, optMonitoredItemId);

    return SOPC_STATUS_OK;
}
