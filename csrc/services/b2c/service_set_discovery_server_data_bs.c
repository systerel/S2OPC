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

#include "service_set_discovery_server_data_bs.h"

#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"

#include <assert.h>

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_set_discovery_server_data_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(
    const constants__t_RegisteredServer_i service_set_discovery_server_data_bs__p_reg_server,
    constants__t_ServerUri* const service_set_discovery_server_data_bs__p_server_uri)
{
    *service_set_discovery_server_data_bs__p_server_uri =
        &service_set_discovery_server_data_bs__p_reg_server->ServerUri;
}

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

void service_set_discovery_server_data_bs__get_ApplicationDescription(
    const constants__t_endpoint_config_idx_i service_set_discovery_server_data_bs__p_endpoint_config_idx,
    t_bool* const service_set_discovery_server_data_bs__p_bres,
    constants__t_ApplicationDescription_i* const service_set_discovery_server_data_bs__p_app_desc)
{
    *service_set_discovery_server_data_bs__p_app_desc = constants__c_ApplicationDescription_indet;
    *service_set_discovery_server_data_bs__p_bres = false;

    SOPC_Endpoint_Config* endpoint_config =
        SOPC_ToolkitServer_GetEndpointConfig(service_set_discovery_server_data_bs__p_endpoint_config_idx);

    if (endpoint_config == NULL)
    {
        return;
    }

    OpcUa_ApplicationDescription* dst_desc = &endpoint_config->serverConfigPtr->serverDescription;

    if (dst_desc->NoOfDiscoveryUrls <= 0)
    {
        /* If no discovery URL already defined for the current endpoint, try to find one.
         * It has one, if either the current endpoint has an implicit discovery endpoint
         * or the current endpoint accepts None security mode.
         * Otherwise we are not able to define automatically a discovery endpoint.
         *
         */
        if (endpoint_config->hasDiscoveryEndpoint || has_none_security_mode(endpoint_config))
        {
            // There is an endpoint with SecurityMode None allowed or an implicit discovery endpoint is added
            dst_desc->DiscoveryUrls = SOPC_Calloc(1, sizeof(SOPC_String));
            SOPC_String_Initialize(dst_desc->DiscoveryUrls);

            if (dst_desc->DiscoveryUrls == NULL)
            {
                *service_set_discovery_server_data_bs__p_bres = false;
                return;
            }

            if (SOPC_STATUS_OK !=
                SOPC_String_CopyFromCString(&dst_desc->DiscoveryUrls[0], endpoint_config->endpointURL))
            {
                *service_set_discovery_server_data_bs__p_bres = false;
                SOPC_Free(dst_desc->DiscoveryUrls);
                dst_desc->DiscoveryUrls = NULL;
                return;
            }

            dst_desc->NoOfDiscoveryUrls = 1;
        }
    }
    *service_set_discovery_server_data_bs__p_bres = true;
    *service_set_discovery_server_data_bs__p_app_desc = dst_desc;
}

void service_set_discovery_server_data_bs__get_ApplicationDescription_ServerUri(
    const constants__t_ApplicationDescription_i service_set_discovery_server_data_bs__p_app_desc,
    constants__t_ServerUri* const service_set_discovery_server_data_bs__p_ServerUri)
{
    assert(NULL != service_set_discovery_server_data_bs__p_app_desc);
    *service_set_discovery_server_data_bs__p_ServerUri =
        &service_set_discovery_server_data_bs__p_app_desc->ApplicationUri;
}

void service_set_discovery_server_data_bs__has_ServerCapabilities(
    const constants__t_MdnsDiscoveryConfig_i service_set_discovery_server_data_bs__p_mdns_config,
    const constants__t_ServerCapabilities service_set_discovery_server_data_bs__p_server_capabilities,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    *service_set_discovery_server_data_bs__p_bool = true;
    int32_t nb_capabilities = service_set_discovery_server_data_bs__p_server_capabilities.NoOfServerCapabilityFilter;
    SOPC_String* capabilities = service_set_discovery_server_data_bs__p_server_capabilities.ServerCapabilityFilter;
    OpcUa_MdnsDiscoveryConfiguration* mdnsConfig = service_set_discovery_server_data_bs__p_mdns_config;
    if (nb_capabilities <= 0)
    {
        return; // No filtering to do
    }
    else if (nb_capabilities > mdnsConfig->NoOfServerCapabilities)
    {
        // More required capabilities than capabilities in the mDNS configuration: cannot be compliant
        *service_set_discovery_server_data_bs__p_bool = false;
        return;
    }
    // Otherwise all capabilities shall be present in the mDNS config
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool allCapabilitiesFound = true;
    for (int32_t i = 0; allCapabilitiesFound && i < nb_capabilities; i++)
    {
        SOPC_String* reqCapability = &capabilities[i];
        bool capabilityAvailable = false;
        for (int32_t j = 0; !capabilityAvailable && j < mdnsConfig->NoOfServerCapabilities; j++)
        {
            status = SOPC_String_Compare(reqCapability, &mdnsConfig->ServerCapabilities[j], true, &comparison);
            assert(SOPC_STATUS_OK == status);
            capabilityAvailable = (0 == comparison);
        }
        allCapabilitiesFound &= capabilityAvailable;
    }
    *service_set_discovery_server_data_bs__p_bool = allCapabilitiesFound;
}

void service_set_discovery_server_data_bs__has_ServerUri(
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_singleServerUri,
    const t_entier4 service_set_discovery_server_data_bs__p_nbServerUri,
    const constants__t_ServerUris service_set_discovery_server_data_bs__p_ServerUris,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    assert(service_set_discovery_server_data_bs__p_nbServerUri > 0);
    assert(service_set_discovery_server_data_bs__p_ServerUris != NULL);
    bool hasServerUri = false;
    for (int32_t i = 0; !hasServerUri && i < service_set_discovery_server_data_bs__p_nbServerUri; i++)
    {
        service_set_discovery_server_data_bs__is_equal_ServerUri(
            service_set_discovery_server_data_bs__p_singleServerUri,
            &service_set_discovery_server_data_bs__p_ServerUris[i], &hasServerUri);
    }
    *service_set_discovery_server_data_bs__p_bool = hasServerUri;
}

void service_set_discovery_server_data_bs__is_empty_ServerUri(
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_server_uri,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    *service_set_discovery_server_data_bs__p_bool = service_set_discovery_server_data_bs__p_server_uri->Length <= 0;
}

void service_set_discovery_server_data_bs__is_equal_ServerUri(
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_left,
    const constants__t_ServerUri service_set_discovery_server_data_bs__p_right,
    t_bool* const service_set_discovery_server_data_bs__p_bool)
{
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_String_Compare(service_set_discovery_server_data_bs__p_left,
                                                   service_set_discovery_server_data_bs__p_right, true, &comparison);
    *service_set_discovery_server_data_bs__p_bool = (SOPC_STATUS_OK == status && 0 == comparison);
}
