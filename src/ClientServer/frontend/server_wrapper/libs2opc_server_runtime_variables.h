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
    OpcUa_BuildInfo build_info;
    SOPC_Byte service_level;
    bool auditing;
    uint32_t maximum_operation_per_request;
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
 * \brief Sets the values for runtime variables in the address space.
 *
 * \param endpoint_config_idx  Config index of the endpoint where to send the
 *                             write request.
 * \param vars                 Values of the runtime variables.
 *
 * \param asyncRespContext     Context to use when sending asynchronous local service request
 *                             to set the runtime variables.
 *
 * \return \c TRUE on success, \c FALSE in case of failure.
 *
 * This function gathers all the runtime values passed as parameters into a
 * single write request sent to the given endpoint. This write request may fail,
 * the application should watch the \c SE_LOCAL_SERVICE_RESPONSE event to be
 * informed of success or failure.
 *
 * This function returns as soon as the write request is sent, which means there
 * might be a delay between when this function returns, and when the values are
 * observable in the address space.
 */
bool SOPC_RuntimeVariables_Set(uint32_t endpoint_config_idx,
                               SOPC_Server_RuntimeVariables* vars,
                               uintptr_t asyncRespContext);

/**
 * \brief Update the server status values runtime variables in the address space.
 *
 * \param endpoint_config_idx  Config index of the endpoint where to send the
 *                             write request.
 * \param vars                 Values of the runtime variables.
 * \return \c TRUE on success, \c FALSE in case of failure.
 *
 * This function gathers all the server status values passed as parameters into a
 * single write request sent to the given endpoint. This write request may fail,
 * the application should watch the \c SE_LOCAL_SERVICE_RESPONSE event to be
 * informed of success or failure.
 *
 * This function returns as soon as the write request is sent, which means there
 * might be a delay between when this function returns, and when the values are
 * observable in the address space.
 */
bool SOPC_RuntimeVariables_UpdateServerStatus(uint32_t endpoint_config_idx,
                                              SOPC_Server_RuntimeVariables* vars,
                                              uintptr_t asyncRespContext);

#endif
