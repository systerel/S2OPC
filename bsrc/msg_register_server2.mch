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
    msg_register_server2

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

ABSTRACT_VARIABLES
    RegisterServer2_NbDiscoveryConfigs,
    RegisterServer2_MdnsDiscoveryConfigIdx

INVARIANT
    RegisterServer2_NbDiscoveryConfigs : NAT &
    RegisterServer2_MdnsDiscoveryConfigIdx : NAT &
    RegisterServer2_MdnsDiscoveryConfigIdx : 0..RegisterServer2_NbDiscoveryConfigs

INITIALISATION
    RegisterServer2_NbDiscoveryConfigs := 0 ||
    RegisterServer2_MdnsDiscoveryConfigIdx := 0

OPERATIONS

    p_sc, p_is_online, p_nb_discovery_config, p_mdns_discovery_config_index, p_registered_server, p_mdns_config
      <-- check_register_server2_req (p_req) =
    PRE
        p_req : t_msg_i &
        p_req = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_register_server2_req
    THEN
        p_sc,
        p_is_online,
        p_nb_discovery_config,
        p_mdns_discovery_config_index,
        p_registered_server,
        p_mdns_config,
        RegisterServer2_NbDiscoveryConfigs,
        RegisterServer2_MdnsDiscoveryConfigIdx
        :(p_sc : t_StatusCode_i &
          p_sc : {e_sc_ok,
                  e_sc_bad_invalid_argument,
                  e_sc_bad_server_uri_invalid,
                  e_sc_bad_server_name_missing,
                  e_sc_bad_discovery_url_missing,
                  e_sc_bad_semaphore_file_missing,
                  e_sc_bad_not_supported} &
          p_is_online : BOOL &
          p_nb_discovery_config : NAT &
          p_mdns_discovery_config_index : NAT &
          p_registered_server : t_RegisteredServer_i &
          p_mdns_config : t_MdnsDiscoveryConfig_i &
          RegisterServer2_NbDiscoveryConfigs : NAT &
          RegisterServer2_MdnsDiscoveryConfigIdx : NAT &
          (p_sc = e_sc_ok =>
              p_nb_discovery_config > 0 &
              p_mdns_discovery_config_index : 0 .. p_nb_discovery_config - 1 &
              p_registered_server : t_RegisteredServer &
              p_mdns_config : t_MdnsDiscoveryConfig &
              RegisterServer2_NbDiscoveryConfigs = p_nb_discovery_config &
              RegisterServer2_MdnsDiscoveryConfigIdx = p_mdns_discovery_config_index
          ) &
          (p_sc /= e_sc_ok =>
              RegisterServer2_NbDiscoveryConfigs = 0 &
              RegisterServer2_MdnsDiscoveryConfigIdx = 0)
        )
    END
    ;

    /* Set the unique mDNS config to GOOD status and all others to UNSUPPORTED */
    p_sc <-- set_register_server2_resp_configuration_results (p_resp, p_nb_discovery_config, p_mdns_discovery_config_index) =
    PRE
        p_resp : t_msg_i &
        p_resp = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_register_server2_resp &
        p_nb_discovery_config : NAT &
        p_nb_discovery_config > 0 &
        p_nb_discovery_config = RegisterServer2_NbDiscoveryConfigs &
        p_mdns_discovery_config_index : NAT &
        p_mdns_discovery_config_index = RegisterServer2_MdnsDiscoveryConfigIdx
    THEN
        p_sc :: {e_sc_ok, e_sc_bad_out_of_memory}
    END

END
