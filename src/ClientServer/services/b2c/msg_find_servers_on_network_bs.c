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

#include "msg_find_servers_on_network_bs.h"

#include <assert.h>

#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_find_servers_on_network_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_find_servers_on_network_bs__alloc_find_servers_on_network_servers(
    const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
    const t_entier4 msg_find_servers_on_network_bs__p_nb_servers,
    t_bool* const msg_find_servers_on_network_bs__p_allocSuccess)
{
    *msg_find_servers_on_network_bs__p_allocSuccess = false;
    assert(msg_find_servers_on_network_bs__p_nb_servers > 0);
    OpcUa_FindServersOnNetworkResponse* resp = msg_find_servers_on_network_bs__p_resp;
    resp->Servers = SOPC_Malloc(sizeof(*resp->Servers) * (size_t) msg_find_servers_on_network_bs__p_nb_servers);
    if (NULL != resp->Servers)
    {
        resp->NoOfServers = msg_find_servers_on_network_bs__p_nb_servers;
        for (int32_t i = 0; i < resp->NoOfServers; i++)
        {
            OpcUa_ServerOnNetwork_Initialize(&resp->Servers[i]);
        }
        *msg_find_servers_on_network_bs__p_allocSuccess = true;
    }
}

void msg_find_servers_on_network_bs__get_find_servers_on_network_req_params(
    const constants__t_msg_i msg_find_servers_on_network_bs__p_req,
    t_entier4* const msg_find_servers_on_network_bs__p_startingRecordId,
    t_entier4* const msg_find_servers_on_network_bs__p_maxRecordsToReturn,
    constants__t_ServerCapabilities* const msg_find_servers_on_network_bs__p_serverCapabilities)
{
    OpcUa_FindServersOnNetworkRequest* req = msg_find_servers_on_network_bs__p_req;
    if (req->StartingRecordId > INT32_MAX)
    {
        *msg_find_servers_on_network_bs__p_startingRecordId = INT32_MAX;
    }
    else
    {
        *msg_find_servers_on_network_bs__p_startingRecordId = (int32_t) req->StartingRecordId;
    }

    if (req->MaxRecordsToReturn > INT32_MAX)
    {
        *msg_find_servers_on_network_bs__p_maxRecordsToReturn = INT32_MAX;
    }
    else
    {
        *msg_find_servers_on_network_bs__p_maxRecordsToReturn = (int32_t) req->MaxRecordsToReturn;
    }

    msg_find_servers_on_network_bs__p_serverCapabilities->NoOfServerCapabilityFilter = req->NoOfServerCapabilityFilter;
    msg_find_servers_on_network_bs__p_serverCapabilities->ServerCapabilityFilter = req->ServerCapabilityFilter;
}

void msg_find_servers_on_network_bs__set_find_servers_on_network_resp(
    const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
    const constants__t_Timestamp msg_find_servers_on_network_bs__p_counterResetTime)
{
    OpcUa_FindServersOnNetworkResponse* resp = msg_find_servers_on_network_bs__p_resp;
    resp->LastCounterResetTime = msg_find_servers_on_network_bs__p_counterResetTime.timestamp;
}

void msg_find_servers_on_network_bs__set_find_servers_on_network_server(
    const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
    const t_entier4 msg_find_servers_on_network_bs__p_srv_index,
    const t_entier4 msg_find_servers_on_network_bs__p_recordId,
    const constants__t_RegisteredServer_i msg_find_servers_on_network_bs__p_registered_server,
    const constants__t_MdnsDiscoveryConfig_i msg_find_servers_on_network_bs__p_mdns_config)
{
    OpcUa_FindServersOnNetworkResponse* resp = msg_find_servers_on_network_bs__p_resp;
    assert(msg_find_servers_on_network_bs__p_srv_index < resp->NoOfServers); // Guaranteed by B model
    assert(msg_find_servers_on_network_bs__p_recordId >= 0);                 // Guaranteed by B model
    assert(msg_find_servers_on_network_bs__p_registered_server->NoOfDiscoveryUrls >
           0); // Guaranteed by construction in B model
    OpcUa_ServerOnNetwork* server = &resp->Servers[msg_find_servers_on_network_bs__p_srv_index];
    server->RecordId = (uint32_t) msg_find_servers_on_network_bs__p_recordId;
    SOPC_ReturnStatus status =
        SOPC_String_AttachFrom(&server->ServerName, &msg_find_servers_on_network_bs__p_mdns_config->MdnsServerName);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set ServerName in registered server of response");
    }
    status = SOPC_String_AttachFrom(&server->DiscoveryUrl,
                                    &msg_find_servers_on_network_bs__p_registered_server->DiscoveryUrls[0]);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set DiscoveryUrl in registered server of response");
    }
    if (SOPC_STATUS_OK == status && msg_find_servers_on_network_bs__p_mdns_config->NoOfServerCapabilities > 0)
    {
        server->ServerCapabilities =
            SOPC_Calloc(sizeof(*server->ServerCapabilities),
                        (size_t) msg_find_servers_on_network_bs__p_mdns_config->NoOfServerCapabilities);
        if (NULL != server->ServerCapabilities)
        {
            for (int32_t i = 0; i < msg_find_servers_on_network_bs__p_mdns_config->NoOfServerCapabilities; i++)
            {
                status = SOPC_String_AttachFrom(&server->ServerCapabilities[i],
                                                &msg_find_servers_on_network_bs__p_mdns_config->ServerCapabilities[i]);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "Failed to set ServerCapability in registered server of response");
                }
            }
            server->NoOfServerCapabilities = msg_find_servers_on_network_bs__p_mdns_config->NoOfServerCapabilities;
        } // else: array is empty but we do not fail
    }
}
