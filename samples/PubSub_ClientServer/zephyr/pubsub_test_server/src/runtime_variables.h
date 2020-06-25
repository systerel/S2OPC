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

#ifndef SOPC_RUNTIME_VARIABLES_H
#define SOPC_RUNTIME_VARIABLES_H

#include "sopc_common_build_info.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

typedef struct _RuntimeVariables
{
    SOPC_Server_Config* serverConfig;
    uint32_t secondsTillShutdown;
    SOPC_LocalizedText shutdownReason;
    OpcUa_ServerState server_state;
    OpcUa_BuildInfo build_info;
    SOPC_Byte service_level;
    bool auditing;
    uint32_t maximum_operation_per_request;
} RuntimeVariables;

/**
 * \brief Builds the structure containing the values for runtime variables in the address space.
 *
 * \param build_info  Toolkit build information structure
 *
 * \param server_config Server configuration data
 *
 * \param  manufacturer_name Sever manufacturer name.
 *
 * \return structure containing all runtime variables.
 *
 */
RuntimeVariables build_runtime_variables(SOPC_Toolkit_Build_Info build_info,
                                         SOPC_Server_Config* server_config,
                                         const char* manufacturer_name);

/**
 * \brief Sets the values for runtime variables in the address space.
 *
 * \param endpoint_config_idx  Config index of the endpoint where to send the
 *                             write request.
 * \param vars                 Values of the runtime variables.
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
bool set_runtime_variables(uint32_t endpoint_config_idx, RuntimeVariables* vars);

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
bool update_server_status_runtime_variables(uint32_t endpoint_config_idx, RuntimeVariables* vars);

#endif
