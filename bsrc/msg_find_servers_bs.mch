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
    msg_find_servers_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

ABSTRACT_VARIABLES
    FindServersReq,
    FindServersReq_LocaleIds,
    FindServersReq_NbServerUri,
    FindServersReq_ServerUris,

    FindServersResp,
    FindServersResp_NbServers,
    FindServersResp_ApplicationDescription,

    FindServers_RegisteredServerToApplicationDesc

INVARIANT
    FindServersReq : t_msg_i &
    FindServersReq_LocaleIds : t_LocaleIds_i &
    FindServersReq_NbServerUri : INT &
    FindServersReq_ServerUris : t_ServerUris &

    FindServersResp : t_msg_i &
    FindServersResp_NbServers : NAT &
    FindServersResp_ApplicationDescription : seq(t_ApplicationDescription) &
    size(FindServersResp_ApplicationDescription) = FindServersResp_NbServers &

    FindServers_RegisteredServerToApplicationDesc : t_RegisteredServer --> t_ApplicationDescription

INITIALISATION
    FindServersReq := c_msg_indet ||
    FindServersReq_LocaleIds := c_LocaleIds_indet ||
    FindServersReq_NbServerUri := 0 ||
    FindServersReq_ServerUris :(FindServersReq_ServerUris : t_ServerUris) ||

    FindServersResp := c_msg_indet ||
    FindServersResp_NbServers := 0 ||
    FindServersResp_ApplicationDescription :(FindServersResp_ApplicationDescription = {}) ||

    FindServers_RegisteredServerToApplicationDesc
      :(FindServers_RegisteredServerToApplicationDesc : t_RegisteredServer --> t_ApplicationDescription)

OPERATIONS

    p_LocaleIds, p_nbServerUri, p_ServerUris <-- get_find_servers_req_params (p_req) =
    PRE
        p_req : t_msg_i &
        p_req = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_find_servers_req
    THEN
        p_LocaleIds,
        p_nbServerUri,
        p_ServerUris,
        FindServersReq,
        FindServersReq_LocaleIds,
        FindServersReq_NbServerUri,
        FindServersReq_ServerUris
        :(
            FindServersReq = p_req &
            FindServersReq_LocaleIds : t_LocaleIds &
            p_LocaleIds = FindServersReq_LocaleIds &
            FindServersReq_NbServerUri : INT &
            p_nbServerUri = FindServersReq_NbServerUri &
            FindServersReq_ServerUris : t_ServerUris &
            p_ServerUris = FindServersReq_ServerUris
        )
    END
    ;

    p_allocSuccess <-- alloc_find_servers (p_resp, p_nb_servers) =
    PRE
        p_resp : t_msg_i &
        p_resp = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_find_servers_resp &
        FindServersReq = c_msg_in &
        p_nb_servers : NAT
    THEN
        p_allocSuccess,
        FindServersResp,
        FindServersResp_NbServers,
        FindServersResp_ApplicationDescription
        :(p_allocSuccess : BOOL &
          FindServersResp : t_msg_i &
          FindServersResp_NbServers : NAT &
          FindServersResp_ApplicationDescription : seq(t_ApplicationDescription) &
          (p_allocSuccess = TRUE =>
             FindServersResp = p_resp &
             FindServersResp_NbServers = p_nb_servers
          ) &
          (p_allocSuccess = FALSE =>
              FindServersResp_NbServers = 0
          )
        )
    END
    ;

    ret <-- set_find_servers_server (p_resp, p_srv_index, p_localeIds, p_registered_server) =
    PRE
        p_resp : t_msg_i &
        FindServersResp = p_resp &
        p_srv_index : NAT &
        p_srv_index < FindServersResp_NbServers &
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_localeIds = FindServersReq_LocaleIds &
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        ret,
        FindServersResp_ApplicationDescription
        :(ret : {e_sc_ok, e_sc_bad_out_of_memory} &
          FindServersResp_ApplicationDescription : seq(t_ApplicationDescription) &
          {p_srv_index} <<| FindServersResp_ApplicationDescription = {p_srv_index} <<| FindServersResp_ApplicationDescription$0 &
          FindServersResp_ApplicationDescription (p_srv_index) = FindServers_RegisteredServerToApplicationDesc(p_registered_server))
    END
    ;

    /* Duplicate of set_find_servers_server used only for setting the current endpoint application description */
    ret <-- set_find_servers_server_ApplicationDescription (p_resp, p_srv_index, p_localeIds, p_endpoint_config_idx, p_app_desc) =
    PRE
        p_resp : t_msg_i &
        FindServersResp = p_resp &
        p_srv_index : NAT &
        p_srv_index < FindServersResp_NbServers &
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_localeIds = FindServersReq_LocaleIds &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_app_desc : t_ApplicationDescription_i &
        p_app_desc : t_ApplicationDescription
    THEN
        ret,
        FindServersResp_ApplicationDescription
        :(ret : {e_sc_ok, e_sc_bad_out_of_memory} &
          FindServersResp_ApplicationDescription : seq(t_ApplicationDescription) &
          {p_srv_index} <<| FindServersResp_ApplicationDescription = {p_srv_index} <<| FindServersResp_ApplicationDescription$0 &
          FindServersResp_ApplicationDescription (p_srv_index) = p_app_desc
        )
    END

END
