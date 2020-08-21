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

MACHINE
    msg_find_servers_on_network_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

ABSTRACT_VARIABLES
    FindServersOnNetworkReq,
    FindServersOnNetworkReq_StartingRecordId,
    FindServersOnNetworkReq_MaxRecordsToReturn,
    FindServersOnNetworkReq_ServerCapabilities,

    FindServersOnNetworkResp,
    FindServersOnNetworkResp_LastCounterResetTime,
    FindServersOnNetworkResp_NbServers,
    FindServersOnNetworkResp_ServerRecordId,
    FindServersOnNetworkResp_ServerRegisteredConfig,
    FindServersOnNetworkResp_ServerMdnsConfig

INVARIANT
    FindServersOnNetworkReq : t_msg_i &
    FindServersOnNetworkReq_StartingRecordId : NAT &
    FindServersOnNetworkReq_MaxRecordsToReturn : NAT &
    FindServersOnNetworkReq_ServerCapabilities : t_ServerCapabilities &

    FindServersOnNetworkResp : t_msg_i &
    FindServersOnNetworkResp_LastCounterResetTime : t_Timestamp &
    FindServersOnNetworkResp_NbServers : NAT &
    FindServersOnNetworkResp_ServerRecordId : seq(NAT) &
    size(FindServersOnNetworkResp_ServerRecordId) = FindServersOnNetworkResp_NbServers &
    FindServersOnNetworkResp_ServerRegisteredConfig : seq(t_RegisteredServer) &
    size(FindServersOnNetworkResp_ServerRegisteredConfig) = FindServersOnNetworkResp_NbServers &
    FindServersOnNetworkResp_ServerMdnsConfig : seq(t_MdnsDiscoveryConfig) &
    size(FindServersOnNetworkResp_ServerMdnsConfig) = FindServersOnNetworkResp_NbServers

INITIALISATION
    FindServersOnNetworkReq := c_msg_indet ||
    FindServersOnNetworkReq_StartingRecordId := 0 ||
    FindServersOnNetworkReq_MaxRecordsToReturn := 0 ||
    FindServersOnNetworkReq_ServerCapabilities :(FindServersOnNetworkReq_ServerCapabilities : t_ServerCapabilities) ||

    FindServersOnNetworkResp := c_msg_indet ||
    FindServersOnNetworkResp_LastCounterResetTime := c_Timestamp_null ||
    FindServersOnNetworkResp_NbServers := 0 ||
    FindServersOnNetworkResp_ServerRecordId :(FindServersOnNetworkResp_ServerRecordId = {}) ||
    FindServersOnNetworkResp_ServerRegisteredConfig :(FindServersOnNetworkResp_ServerRegisteredConfig = {}) ||
    FindServersOnNetworkResp_ServerMdnsConfig :(FindServersOnNetworkResp_ServerMdnsConfig = {})
OPERATIONS

    p_startingRecordId, p_maxRecordsToReturn, p_serverCapabilities <-- get_find_servers_on_network_req_params (p_req) =
    PRE
        p_req : t_msg_i &
        p_req = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_find_servers_on_network_req
    THEN
        p_startingRecordId,
        p_maxRecordsToReturn,
        p_serverCapabilities,
        FindServersOnNetworkReq,
        FindServersOnNetworkReq_StartingRecordId,
        FindServersOnNetworkReq_MaxRecordsToReturn,
        FindServersOnNetworkReq_ServerCapabilities
        :(
            FindServersOnNetworkReq = p_req &
            FindServersOnNetworkReq_StartingRecordId : NAT &
            p_startingRecordId = FindServersOnNetworkReq_StartingRecordId &
            FindServersOnNetworkReq_MaxRecordsToReturn : NAT &
            p_maxRecordsToReturn = FindServersOnNetworkReq_MaxRecordsToReturn &
            FindServersOnNetworkReq_ServerCapabilities : t_ServerCapabilities &
            p_serverCapabilities = FindServersOnNetworkReq_ServerCapabilities
        )
    END
    ;

    set_find_servers_on_network_resp (p_resp, p_counterResetTime) =
    PRE
        FindServersOnNetworkReq = c_msg_in &
        p_resp : t_msg_i &
        p_resp = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_find_servers_on_network_resp &
        p_counterResetTime : t_Timestamp
    THEN
        FindServersOnNetworkResp := p_resp ||
        FindServersOnNetworkResp_LastCounterResetTime := p_counterResetTime
    END
    ;

    p_allocSuccess <-- alloc_find_servers_on_network_servers (p_resp, p_nb_servers) =
    PRE
        p_resp : t_msg_i &
        FindServersOnNetworkReq = c_msg_in &
        FindServersOnNetworkResp = p_resp &
        p_nb_servers : NAT &
        (FindServersOnNetworkReq_MaxRecordsToReturn /= 0 =>
            p_nb_servers <= FindServersOnNetworkReq_MaxRecordsToReturn)
    THEN
        p_allocSuccess,
        FindServersOnNetworkResp_NbServers,
        FindServersOnNetworkResp_ServerRecordId,
        FindServersOnNetworkResp_ServerRegisteredConfig,
        FindServersOnNetworkResp_ServerMdnsConfig
        :(p_allocSuccess : BOOL &
          FindServersOnNetworkResp_NbServers : NAT &
          FindServersOnNetworkResp_ServerRecordId : seq(NAT) &
          FindServersOnNetworkResp_ServerRegisteredConfig : seq(t_RegisteredServer) &
          FindServersOnNetworkResp_ServerMdnsConfig : seq(t_MdnsDiscoveryConfig) &
          (p_allocSuccess = TRUE =>
             FindServersOnNetworkResp_NbServers = p_nb_servers
          ) &
          (p_allocSuccess = FALSE =>
              FindServersOnNetworkResp_NbServers = 0
          )
        )
    END
    ;

    set_find_servers_on_network_server (p_resp, p_srv_index, p_recordId, p_registered_server, p_mdns_config) =
    PRE
        p_resp : t_msg_i &
        FindServersOnNetworkResp = p_resp &
        p_srv_index : NAT &
        p_srv_index < FindServersOnNetworkResp_NbServers &
        p_recordId : NAT &
        p_recordId >= FindServersOnNetworkReq_StartingRecordId &
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer &
        p_mdns_config : t_MdnsDiscoveryConfig_i &
        p_mdns_config : t_MdnsDiscoveryConfig
    THEN
        FindServersOnNetworkResp_ServerRecordId(p_srv_index) := p_recordId ||
        FindServersOnNetworkResp_ServerRegisteredConfig(p_srv_index) := p_registered_server ||
        FindServersOnNetworkResp_ServerMdnsConfig(p_srv_index) := p_mdns_config
    END

END
