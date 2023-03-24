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
 * \brief Internal module used to manage the server runtime node variable changes (Server information nodes)
 */

#ifndef SOPC_RUNTIME_VARIABLES_H
#define SOPC_RUNTIME_VARIABLES_H

#include "sopc_common_build_info.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

typedef struct SOPC_Server_RuntimeVariables
{
    SOPC_Server_Config* serverConfig;
    uint32_t secondsTillShutdown;
    SOPC_LocalizedText shutdownReason;
    OpcUa_ServerState server_state;
    SOPC_DateTime startTime;
    OpcUa_BuildInfo build_info;
    SOPC_Byte service_level;
    bool auditing;
    uint32_t maximum_operations_per_request;
    uint32_t maximum_heavy_operations_per_request;
} SOPC_Server_RuntimeVariables;

/**
 * \brief Builds (without custom ::OpcUa_BuildInfo) the structure containing the values for runtime variables in the
 *        address space
 *
 * Information on software are extracted from server configuration. Manufacturer and build date and version are those of
 * S2OPC by default. Otherwise ::SOPC_RuntimeVariables_Build should be used with custom ::OpcUa_BuildInfo.
 *
 * \param build_info  Toolkit build information structure
 * \param server_config Server configuration data
 *
 * \return structure containing all runtime variables.
 */
SOPC_Server_RuntimeVariables SOPC_RuntimeVariables_BuildDefault(SOPC_Toolkit_Build_Info build_info,
                                                                SOPC_Server_Config* server_config);

/**
 * \brief Builds the structure containing the values for runtime variables in the address space.
 *
 * Use the custom ::OpcUa_BuildInfo structure content provided, content is copied.
 *
 * \param build_info  Server custom build information
 * \param server_config Server configuration data
 *
 * \return structure containing all runtime variables.
 *
 * \warning Coherence between build information and server configuration (application description, etc.) is not enforced
 *
 */
SOPC_Server_RuntimeVariables SOPC_RuntimeVariables_Build(OpcUa_BuildInfo* build_info,
                                                         SOPC_Server_Config* server_config);

/**
 * \brief Build a write request to write the values for runtime variables in the address space.
 *
 * \param vars                 Values of the runtime variables.
 *
 * \return the WriteRequest to use with local services in case of success, NULL in case of failure.
 *
 * This function gathers all the runtime values passed as parameters into a
 * single write request that should be used with ::SOPC_ToolkitServer_AsyncLocalServiceRequest
 * to update the runtime variables of the server.
 */
OpcUa_WriteRequest* SOPC_RuntimeVariables_BuildWriteRequest(SOPC_Server_RuntimeVariables* vars);

/**
 * \brief Build a write request to update the server status values runtime variables in the address space.
 *
 * \param vars                 Values of the runtime variables.
 * \return the WriteRequest to use with local services in case of success, NULL in case of failure.
 *
 * This function gathers all the server status values passed as parameters into a
 * single write request that should be used with ::SOPC_ToolkitServer_AsyncLocalServiceRequest
 * to update the runtime variables of the server.
 */
OpcUa_WriteRequest* SOPC_RuntimeVariables_BuildUpdateServerStatusWriteRequest(SOPC_Server_RuntimeVariables* vars);

/**
 * \brief Build a write request to update the server status current time variables value in the address space.
 *
 * \param var Values of the runtime variables.
 * \return the WriteRequest to use with local services in case of success, NULL in case of failure.
 *
 * This function gathers the 2 variables nodes containing the server status current time (ServerStatus and CurrentTime)
 * into a single write request that should be used with ::SOPC_ToolkitServer_AsyncLocalServiceRequest
 * to update the runtime variables of the server.
 */
OpcUa_WriteRequest* SOPC_RuntimeVariables_UpdateCurrentTimeWriteRequest(SOPC_Server_RuntimeVariables* vars);

#endif
