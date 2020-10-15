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

// The global helper config variable (singleton), it shall not be accessed outside of wrapper code
typedef struct SOPC_Helper_Config
{
    // Flag atomically set when the structure is initialized during call to SOPC_Helper_Initialize
    // and singleton config is initialized
    int32_t initialized;
    // Flag atomically set when the structure is locked because server started
    // (SOPC_ServerHelper_Server or SOPC_ServerHelper_StartServer aleady called)
    int32_t locked;
    // Communication events callback
    SOPC_ComEvent_Fct* comEventCb;
    // Toolkit configuration structure
    SOPC_S2OPC_Config config;
    struct
    {
        // Address space instance
        SOPC_AddressSpace* addressSpace;

        // Application write notification callback record
        SOPC_WriteNotif_Fct* writeNotifCb;
        // Application asynchronous local service response callback record
        SOPC_LocalServiceAsyncResp_Fct* asyncRespCb;

        // Synchronous local service response management
        Condition syncLocalServiceCond;
        Mutex syncLocalServiceMutex;
        uint32_t syncLocalServiceId;
        void* syncResp;

        // Stop server management
        Condition serverStoppedCond;
        Mutex serverStoppedMutex;
        bool serverRequestedToStop;
        bool serverAllEndpointsClosed;
        SOPC_ReturnStatus serverStoppedStatus;
        // Server stopped notification callback record
        SOPC_ServerStopped_Fct* stoppedCb;
        // Server shutdown phase duration
        uint16_t secondsTillShutdown;

        // User authentication and authorization managers
        // Note: temporarily duplicated with SOPC_S2OPC_Config endpoints
        // until moved from SOPC_Endpoint_Config to SOPC_Server_Config
        SOPC_UserAuthentication_Manager* authenticationManager;
        SOPC_UserAuthorization_Manager* authorizationManager;

        // Software manufacturer name
        char* manufacturerName;

        // Configured endpoint indexes and opened state arrays
        uint8_t nbEndpoints;
        SOPC_Endpoint_Config*
            endpoints[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS]; // we do not use config.endpoints to avoid
                                                                     // pre-allocating structure
        uint32_t* endpointIndexes;
        bool* endpointOpened;

        // Runtime variables
        SOPC_Server_RuntimeVariables runtimeVariables;

        // Server started flag (atomic accesses only)
        int32_t started;
    } server; // additional configuration to config.serverConfig
    struct
    {
        // Communication events callback for client
        // Note: until client wrapper and server are merged this is a workaround to dispatch events to a raw client
        SOPC_ComEvent_Fct* clientComEventCb;
    } client;
} SOPC_Helper_Config;

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
            // internal management of local service response
            SOPC_LocalServiceAsyncResp_Fct* helperInternalCb;
            // message to display in case of internal local service failure (response NOK)
            const char* errorMsg;
        } localService;
    } eventCtx;
} SOPC_HelperConfigInternal_Ctx;

// The singleton configuration structure
extern SOPC_Helper_Config sopc_helper_config;
// The default value of the configuration structure
extern const SOPC_Helper_Config sopc_helper_config_default;

// Returns true if sopc_helper_config.initialized && !sopc_helper_config.locked, false otherwise
// Note: values are retrieved in an atomic way
bool SOPC_HelperConfig_IsInitAndUnlock(void);

// Returns true if sopc_helper_config.initialized && sopc_helper_config.locked, false otherwise
// Note: values are retrieved in an atomic way
bool SOPC_HelperConfig_IsInitAndLock(void);

// Atomically set the configuration as locked and check for configuration issues
bool SOPC_HelperConfig_LockState(void);

// Associate global user manager to existing endpoint configurations. It shall be called when user managers are set.
// Note: temporarily necessary until low-level configuration also define managers at server configuration instead of
//       endpoint configuration level.
void SOPC_HelperConfig_SetEndpointsUserMgr(void);

// Local service synchronous internal callback
void SOPC_HelperInternal_SyncLocalServiceCb(SOPC_EncodeableType* encType,
                                            void* response,
                                            SOPC_HelperConfigInternal_Ctx* helperCtx);

// Local service asynchronous internal callback
void SOPC_HelperInternal_AsyncLocalServiceCb(SOPC_EncodeableType* encType,
                                             void* response,
                                             SOPC_HelperConfigInternal_Ctx* helperCtx);

// Endpoint closed asynchronous callback
void SOPC_HelperInternal_ClosedEndpoint(uint32_t epConfigIdx, SOPC_ReturnStatus status);

// Clear low level endpoint config (clear strings, do not clear user managers)
void SOPC_HelperInternal_ClearEndpoint(SOPC_Endpoint_Config* epConfig);

#endif
