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
 * Implements the structures behind the address space.
 */

#include "service_find_servers_bs.h"

#include <assert.h>
#include <stdbool.h>

#include "b2c.h"
#include "opcua_statuscodes.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_find_servers_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static bool has_none_security_mode(SOPC_Endpoint_Config* epConfig)
{
    for (int i = 0; i < epConfig->nbSecuConfigs; i++)
    {
        if ((epConfig->secuConfigurations[i].securityModes & SOPC_SECURITY_MODE_NONE_MASK) != 0)
        {
            return true;
        }
    }
    return false;
}

void service_find_servers_bs__treat_find_servers_request(
    const constants__t_msg_i service_find_servers_bs__req_msg,
    const constants__t_msg_i service_find_servers_bs__resp_msg,
    const constants__t_endpoint_config_idx_i service_find_servers_bs__endpoint_config_idx,
    constants_statuscodes_bs__t_StatusCode_i* const service_find_servers_bs__ret)
{
    OpcUa_FindServersRequest* req = service_find_servers_bs__req_msg;
    OpcUa_FindServersResponse* response = service_find_servers_bs__resp_msg;

    SOPC_Endpoint_Config* endpoint_config =
        SOPC_ToolkitServer_GetEndpointConfig(service_find_servers_bs__endpoint_config_idx);

    if (endpoint_config == NULL)
    {
        *service_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_internal_error;
        return;
    }

    // Part 4 §5.4.2.1: The server shall always return a record that describes
    // itself.
    //
    // Since we're the only server here, we will always return a single record

    response->Servers = SOPC_Calloc(1, sizeof(OpcUa_ApplicationDescription));
    OpcUa_ApplicationDescription_Initialize(response->Servers);

    if (response->Servers == NULL)
    {
        *service_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        return;
    }

    response->NoOfServers = 1;

    const OpcUa_ApplicationDescription* src_desc = &endpoint_config->serverConfigPtr->serverDescription;
    OpcUa_ApplicationDescription* dst_desc = &response->Servers[0];

    if (endpoint_config->hasDiscoveryEndpoint || has_none_security_mode(endpoint_config))
    {
        // There is an endpoint with SecurityMode None allowed or an implicit discovery endpoint is added
        dst_desc->DiscoveryUrls = SOPC_Calloc(1, sizeof(SOPC_String));
        SOPC_String_Initialize(dst_desc->DiscoveryUrls);

        if (dst_desc->DiscoveryUrls == NULL)
        {
            *service_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            return;
        }

        if (SOPC_STATUS_OK != SOPC_String_CopyFromCString(&dst_desc->DiscoveryUrls[0], endpoint_config->endpointURL))
        {
            *service_find_servers_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            return;
        }

        dst_desc->NoOfDiscoveryUrls = 1;
    }
    dst_desc->ApplicationType = src_desc->ApplicationType;

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    char** preferredLocales = (char**) SOPC_String_GetRawCStringArray(req->NoOfLocaleIds, req->LocaleIds);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    bool ok = (SOPC_String_Copy(&dst_desc->ApplicationUri, &src_desc->ApplicationUri) == SOPC_STATUS_OK) &&
              (SOPC_String_Copy(&dst_desc->ProductUri, &src_desc->ProductUri) == SOPC_STATUS_OK) &&
              (SOPC_LocalizedText_GetPreferredLocale(&dst_desc->ApplicationName, preferredLocales,
                                                     &src_desc->ApplicationName) == SOPC_STATUS_OK);

    SOPC_Free(preferredLocales);

    *service_find_servers_bs__ret =
        (ok ? constants_statuscodes_bs__e_sc_ok : constants_statuscodes_bs__e_sc_bad_out_of_memory);
}
