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
 * \privatesection
 *
 * \brief Internal module used to manage the wrapper for server config. It should not be used outside of the server
 * wraper implementation.
 *
 */

#ifndef LIBS2OPC_SERVER_CONFIG_INTERNAL_H_
#define LIBS2OPC_SERVER_CONFIG_INTERNAL_H_

#include <inttypes.h>
#include <stdbool.h>

#include "sopc_address_space.h"
#include "sopc_builtintypes.h"
#include "sopc_event_manager.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_user_app_itf.h"

#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_runtime_variables.h"

#ifndef SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS
#define SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS 5000
#endif

#ifndef SOPC_DEFAULT_SHUTDOWN_PHASE_IN_SECONDS
#define SOPC_DEFAULT_SHUTDOWN_PHASE_IN_SECONDS 5
#endif

#ifndef SOPC_DEFAULT_CURRENT_TIME_REFERSH_FREQ_MS
#define SOPC_DEFAULT_CURRENT_TIME_REFERSH_FREQ_MS 1000
#endif

typedef enum
{
    SOPC_SERVER_STATE_INITIALIZING = 0,
    SOPC_SERVER_STATE_CONFIGURING,
    SOPC_SERVER_STATE_CONFIGURED,
    SOPC_SERVER_STATE_STARTED,
    SOPC_SERVER_STATE_STOPPING,
    SOPC_SERVER_STATE_STOPPED,
} SOPC_HelperServer_State;

// The server helper dedicated configuration in addition to configuration ::SOPC_S2OPC_Config
typedef struct SOPC_ServerHelper_Config
{
    // Flag atomically set when the structure is initialized during call to SOPC_ServerConfigHelper_Initialize
    // and singleton config is initialized
    int32_t initialized;
    // Server state
    SOPC_Mutex stateMutex;
    SOPC_HelperServer_State state;

    // Address space instance
    SOPC_AddressSpace* addressSpace;

    // Callback called on HistoryRead requests
    SOPC_ExternalHistoryRawRead_Fct* externalHistoryReadCb;
    uintptr_t externalHistoryReadContext;
    // Application write notification callback record
    SOPC_WriteNotif_Fct* writeNotifCb;
    // Application session event notification callback record
    SOPC_SessionEventNotif_Fct* sessionNotifCb;
    // Application node availability for CreateMonitoreItem callback
    SOPC_CreateMI_NodeAvail_Fct* nodeAvailCb;
    // Application asynchronous local service response callback record
    SOPC_LocalServiceAsyncResp_Fct* asyncRespCb;
    // Application server private key password callback
    SOPC_GetServerKeyPassword_Fct*
        getServerKeyPassword; /* If it is defined and if the serverKey is encrypted,
                                 then the callback is called during call to
                                 ::SOPC_ServerConfigHelper_SetKeyCertPairFromPath
                                 or ::SOPC_ServerConfigHelper_ConfigureFromXML.
                                 The callback allows to retrieve the password for decryption */
    // Asynchronous local service response management
    SOPC_SLinkedList* localServiceListReqCtxList; /* List of yet-to-be-answered local requests, value is a
                                                  ::SOPC_HelperConfigInternal_Ctx* */

    // Synchronous local service response management
    SOPC_Condition syncLocalServiceCond;
    SOPC_Mutex syncLocalServiceMutex;
    uint32_t syncLocalServiceId;
    bool syncCalled;
    void* syncResp;

    // Stop server management:

    // Manage server stopping when server is running synchronously using SOPC_ServerHelper_Serve.
    struct
    {
        SOPC_Condition serverStoppedCond;
        SOPC_Mutex serverStoppedMutex;
        int32_t serverRequestedToStop;
        bool serverAllEndpointsClosed;
    } syncServeStopData;

    // Server stopped notification callback record
    SOPC_ServerStopped_Fct* userStoppedCb;
    // Server stopped notification callback data
    SOPC_ReturnStatus serverStoppedStatus;

    // Server shutdown phase duration configuration
    uint16_t configuredSecondsTillShutdown;
    // Server status current time refresh interval
    uint16_t configuredCurrentTimeRefreshIntervalMs;
    uint32_t currentTimeRefreshTimerId;

    // Server build info
    OpcUa_BuildInfo* buildInfo;

    // Configured endpoint indexes and opened state arrays
    uint8_t nbEndpoints;
    SOPC_Endpoint_Config* endpoints[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS]; // we do not use config.endpoints to
                                                                                   // avoid pre-allocating structure
    SOPC_EndpointConfigIdx* endpointIndexes; // array of endpoint indexes provided by toolkit
    bool* endpointClosed; // array of closed endpoint to keep track of endpoints notified closed by toolkit

    // Runtime variables
    SOPC_Server_RuntimeVariables runtimeVariables;

    // Write node specific behavior callbacks
    SOPC_Dict* writeInternalBehaviorCb;

} SOPC_ServerHelper_Config;

// Define the structure used as context for asynchronous calls
typedef struct SOPC_HelperConfigInternal_Ctx
{
    // actual context for helper user (user application)
    uintptr_t userContext;

    // Discriminant
    SOPC_App_Com_Event event;
    union
    {
        struct LocalServiceCtx
        {
            // sync call management
            bool isSyncCall;
            uint32_t syncId;
            // custom async response callback
            SOPC_LocalServiceAsyncResp_Fct* customAsyncRespCb;
            // internal use of local services + local service type callback (e.g runtime variables)
            bool isHelperInternal;
            // message to display in case of internal local service failure (response NOK)
            const char* internalErrorMsg;
        } localService;
    } eventCtx;
} SOPC_HelperConfigInternal_Ctx;

// The singleton configuration structure
extern SOPC_ServerHelper_Config sopc_server_helper_config;

// Returns true if the server is in configuring state, false otherwise
bool SOPC_ServerInternal_IsConfiguring(void);

// Returns true if the server is in started state, false otherwise
bool SOPC_ServerInternal_IsStarted(void);

// Returns true if the server is in stopping state, false otherwise
bool SOPC_ServerInternal_IsStopping(void);

// Returns true if the server is in stopped state, false otherwise
bool SOPC_ServerInternal_IsStopped(void);

// Returns true if the server is in stopped state or in a state previous to started state, false otherwise
// Note: server configuration is not clearable in shutdown or stopping states
bool SOPC_ServerInternal_IsConfigClearable(void);

// Check for configuration issues and set server state as configured in case of success
bool SOPC_ServerInternal_CheckConfigAndSetConfiguredState(void);

// Check current state and set server state as started in case of success
bool SOPC_ServerInternal_SetStartedState(void);

// Check current state and set server state as stopping in case of success
bool SOPC_ServerInternal_SetStoppingState(void);

// Set server state as stopped
void SOPC_ServerInternal_SetStoppedState(void);

// Called when server is stopped to signal stop to waiting condition variable
void SOPC_ServerInternal_SyncServerStoppedCb(SOPC_ReturnStatus stopStatus);

// Get password to decrypt server private key from internal callback
bool SOPC_ServerInternal_GetKeyPassword(char** outPassword);

// Local service synchronous internal callback
void SOPC_ServerInternal_SyncLocalServiceCb(SOPC_EncodeableType* encType,
                                            void* response,
                                            SOPC_HelperConfigInternal_Ctx* helperCtx);

// Local service asynchronous internal callback
void SOPC_ServerInternal_AsyncLocalServiceCb(SOPC_EncodeableType* encType,
                                             void* response,
                                             SOPC_HelperConfigInternal_Ctx* helperCtx);

// Endpoint closed asynchronous callback
void SOPC_ServerInternal_ClosedEndpoint(uint32_t epConfigIdx, SOPC_ReturnStatus status);

// Clear low level endpoint config (clear strings, do not clear user managers)
void SOPC_ServerInternal_ClearEndpoint(SOPC_Endpoint_Config* epConfig);

// Callback instance to be used on client application key / certificate pair update
void SOPC_ServerInternal_KeyCertPairUpdateCb(uintptr_t updateParam);

// Callback instance to be used on server PKI update
void SOPC_ServerInternal_PKIProviderUpdateCb(uintptr_t updateParam);

// Local service asynchronous internal version:
// it differs from ::SOPC_ServerHelper_LocalServiceAsyncCustom
// in that the provided context in \p asyncRespCb call is internal ::SOPC_HelperConfigInternal_Ctx instead of \p userCtx
bool SOPC_ServerInternal_LocalServiceAsync(SOPC_LocalServiceAsyncResp_Fct* asyncRespCb,
                                           void* request,
                                           uintptr_t userCtx,
                                           const char* errorMsg);

/**
 * \brief Triggers an audit event from the Server node and log a trace in audit log entry.
 *
 * \param event  The audit event to be triggered
 *
 *  \return SOPC_STATUS_OK in case of success,
 *          SOPC_STATUS_INVALID_PARAMETERS, SOPC_STATUS_INVALID_STATE or
 *          SOPC_STATUS_NOT_SUPPORTED otherwise.
 */
SOPC_ReturnStatus SOPC_ServerInternal_TriggerAuditEvent(SOPC_Event* event);

/**
 * \brief Callback called when an internal Write request updating server runtime variables completes.
 *
 * A result OpcUa_BadNodeIdUnknown is expected (runtime variable node absent from the address space): no warning
 * is traced for it. If the context contains a copy of the write request, absent nodes are traced in debug level
 * with an info level summary. Any other failure (service result or write result) is traced as a warning.
 *
 * \param encType Encodeable type of the received response.
 * \param response Pointer to the received service response.
 * \param context Internal context associated with the asynchronous local service request.
 */
void SOPC_HelperInternal_RuntimeVariableSetResponseCb(SOPC_EncodeableType* encType, void* response, uintptr_t context);

/**
 * \brief Internal behavior callback called on successful write of a node Value attribute.
 *
 * Called synchronously from the services thread: it shall not block, shall not call synchronous local services
 * and shall not keep references to its parameters after returning.
 *
 * \param nodeId     The NodeId of the written node
 * \param prevValue  The value before the write operation
 * \param newValue   The value after the write operation (Good status code)
 * \param auxParam   The auxiliary parameter provided on registration
 */
typedef void SOPC_ServerInternal_WriteBehavior_Fct(const SOPC_NodeId* nodeId,
                                                   const SOPC_DataValue* prevValue,
                                                   const SOPC_DataValue* newValue,
                                                   uintptr_t auxParam);

/**
 * \brief Registers an internal behavior callback called on write of the given node Value attribute.
 *        Only one callback can be registered per node.
 *
 * \param nodeId    The NodeId of the node (copied)
 * \param callback  The callback to call on write of the node Value attribute
 * \param auxParam  The auxiliary parameter provided to the callback
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_STATE if the server is not in configuring state,
 *         SOPC_STATUS_INVALID_PARAMETERS if a parameter is NULL or a callback is already registered for the node,
 *         SOPC_STATUS_OUT_OF_MEMORY otherwise.
 */
SOPC_ReturnStatus SOPC_ServerInternal_RegisterWriteBehaviorCb(const SOPC_NodeId* nodeId,
                                                              SOPC_ServerInternal_WriteBehavior_Fct* callback,
                                                              uintptr_t auxParam);

/**
 * \brief Gets the internal behavior callback registered for the given node.
 *
 * \param nodeId          The NodeId of the node
 * \param[out] callback   The registered callback
 * \param[out] auxParam   The registered auxiliary parameter
 *
 * \return true if the server is started or stopping and a callback is registered for the node, false otherwise.
 */
bool SOPC_ServerInternal_GetRegisteredWriteBehaviorCb(const SOPC_NodeId* nodeId,
                                                      SOPC_ServerInternal_WriteBehavior_Fct** callback,
                                                      uintptr_t* auxParam);

#endif
