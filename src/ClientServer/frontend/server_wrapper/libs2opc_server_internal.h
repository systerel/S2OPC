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
#include "sopc_mutexes.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_user_app_itf.h"

#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_runtime_variables.h"

#ifndef SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS
#define SOPC_HELPER_LOCAL_RESPONSE_TIMEOUT_MS 5000
#endif

#ifndef DEFAULT_SHUTDOWN_PHASE_IN_SECONDS
#define DEFAULT_SHUTDOWN_PHASE_IN_SECONDS 5
#endif

typedef enum
{
    SOPC_SERVER_STATE_INITIALIZING = 0,
    SOPC_SERVER_STATE_CONFIGURING = 1,
    SOPC_SERVER_STATE_CONFIGURED = 2,
    SOPC_SERVER_STATE_STARTED = 3,
    SOPC_SERVER_STATE_SHUTDOWN_PHASE = 4,
    SOPC_SERVER_STATE_STOPPING = 5,
    SOPC_SERVER_STATE_STOPPED = 6,
} SOPC_HelperServer_State;

// The server helper dedicated configuration in addition to configuration ::SOPC_S2OPC_Config
typedef struct SOPC_ServerHelper_Config
{
    // Flag atomically set when the structure is initialized during call to SOPC_HelperConfigServer_Initialize
    // and singleton config is initialized
    int32_t initialized;
    // Server state
    Mutex stateMutex;
    SOPC_HelperServer_State state;

    // Address space instance
    SOPC_AddressSpace* addressSpace;

    // Application write notification callback record
    SOPC_WriteNotif_Fct* writeNotifCb;
    // Application node availability for CreateMonitoreItem callback
    SOPC_CreateMI_NodeAvail_Fct* nodeAvailCb;
    // Application asynchronous local service response callback record
    SOPC_LocalServiceAsyncResp_Fct* asyncRespCb;

    // Synchronous local service response management
    Condition syncLocalServiceCond;
    Mutex syncLocalServiceMutex;
    uint32_t syncLocalServiceId;
    void* syncResp;

    // Stop server management:

    // Manage server stopping when server is running synchronously using SOPC_ServerHelper_Serve.
    struct
    {
        Condition serverStoppedCond;
        Mutex serverStoppedMutex;
        int32_t serverRequestedToStop;
        bool serverAllEndpointsClosed;
    } syncServeStopData;

    // Server stopped notification callback record
    SOPC_ServerStopped_Fct* stoppedCb;
    // Server stopped notification callback data
    SOPC_ReturnStatus serverStoppedStatus;

    // Server shutdown phase duration configuration
    uint16_t configuredSecondsTillShutdown;

    // User authentication and authorization managers
    // Note: temporarily duplicated with SOPC_S2OPC_Config endpoints
    // until moved from SOPC_Endpoint_Config to SOPC_Server_Config
    SOPC_UserAuthentication_Manager* authenticationManager;
    SOPC_UserAuthorization_Manager* authorizationManager;

    // Server build info
    OpcUa_BuildInfo* buildInfo;

    // Configured endpoint indexes and opened state arrays
    uint8_t nbEndpoints;
    SOPC_Endpoint_Config* endpoints[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS]; // we do not use config.endpoints to
                                                                                   // avoid pre-allocating structure
    uint32_t* endpointIndexes; // array of endpoint indexes provided by toolkit
    bool* endpointClosed;      // array of closed endpoint to keep track of endpoints notified closed by toolkit

    // Runtime variables
    SOPC_Server_RuntimeVariables runtimeVariables;
} SOPC_ServerHelper_Config;

// Define the structure used as context for asynchronous calls
typedef struct SOPC_HelperConfigInternal_Ctx
{
    // actual context for helper user (user application)
    uintptr_t userContext;

    // Discriminant
    SOPC_App_Com_Event event;
    union {
        struct LocalServiceCtx
        {
            // sync call management
            bool isSyncCall;
            uint32_t syncId;
            // internal use of local services (runtime variables)
            bool isHelperInternal;
            // message to display in case of internal local service failure (response NOK)
            const char* internalErrorMsg;
        } localService;
    } eventCtx;
} SOPC_HelperConfigInternal_Ctx;

// The default value of the configuration structure
extern const SOPC_ServerHelper_Config sopc_server_helper_config_default;

// The singleton configuration structure
extern SOPC_ServerHelper_Config sopc_server_helper_config;

// Returns true if the server is in configuring state, false otherwise
bool SOPC_ServerInternal_IsConfiguring(void);

// Returns true if the server is in started state, false otherwise
bool SOPC_ServerInternal_IsStarted(void);

// Returns true if the server is in stopped state, false otherwise
bool SOPC_ServerInternal_IsStopped(void);

// Check for configuration issues and set server state as configured in case of success
bool SOPC_ServerInternal_CheckConfigAndSetConfiguredState(void);

// Check current state and set server state as started in case of success
bool SOPC_ServerInternal_SetStartedState(void);

// Check current state and set server state as stopping in case of success
bool SOPC_ServerInternal_SetStoppingState(void);

// Set server state as stopped
void SOPC_ServerInternal_SetStoppedState(void);

// Associate global user manager to existing endpoint configurations. It shall be called when user managers are set.
// Note: temporarily necessary until low-level configuration also define managers at server configuration instead of
//       endpoint configuration level.
void SOPC_ServerInternal_SetEndpointsUserMgr(void);

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

#endif
