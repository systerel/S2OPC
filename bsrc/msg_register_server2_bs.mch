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
    msg_register_server2_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    p_registered_server <-- get_register_server2_req_registered_server (p_req) =
    PRE
        p_req : t_msg_i &
        p_req = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_register_server2_req
    THEN
        p_registered_server :(p_registered_server : t_RegisteredServer_i &
                              p_registered_server : t_RegisteredServer)
    END
    ;

    p_valid_server_uri <-- check_registered_server_uri (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_server_uri :: BOOL
        /* <=> serverUri not empty (TODO: has a valid URI format ?) */
    END;

    p_valid_product_uri <-- check_registered_product_uri (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_product_uri :: BOOL
        /* <=> productUri not empty (TODO: has a valid URI format ?) */
    END;

    p_valid_server_names <-- check_registered_server_names (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_server_names :: BOOL
        /* <=> at least first server name is not empty */
    END;

    p_valid_server_type <-- check_registered_server_type (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_server_type :: BOOL
        /* <=> is a valid server type (not invalid and not client) */
    END;

    p_valid_discovery_url <-- check_registered_discovery_url (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_discovery_url :: BOOL
        /* <=> at least first discovery URL is  not empty (TODO: has a valid URL format ?) */
    END;

    p_valid_semaphore_file <-- check_registered_semaphore_file (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_valid_semaphore_file :: BOOL
        /* <=> semaphore file is not defined (otherwise not supported) */
    END;

    p_is_online <-- get_registered_server_is_online (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        p_is_online :: BOOL
        /* <=> IsOnline */
    END;

    p_has_discovery_configuration, p_has_one_and_only_one_mdns_config,
    p_mdns_discovery_configuration, p_nb_discovery_config, p_mdns_discovery_config_index
      <-- getall_register_server2_req_msdn_discovery_config (p_req) =
    PRE
        p_req : t_msg_i &
        p_req = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_register_server2_req
    THEN
        p_has_discovery_configuration,
        p_has_one_and_only_one_mdns_config,
        p_mdns_discovery_configuration,
        p_nb_discovery_config,
        p_mdns_discovery_config_index
        :(
            p_has_discovery_configuration : BOOL &
            p_has_one_and_only_one_mdns_config : BOOL &
            p_mdns_discovery_configuration : t_MdnsDiscoveryConfig_i &
            p_nb_discovery_config : NAT &
            p_mdns_discovery_config_index : NAT &
            (not (p_has_discovery_configuration = TRUE & p_has_one_and_only_one_mdns_config = TRUE) =>
                p_mdns_discovery_configuration = c_MdnsDiscoveryConfig_indet) &
            (p_has_discovery_configuration = TRUE & p_has_one_and_only_one_mdns_config = TRUE =>
                p_mdns_discovery_configuration : t_MdnsDiscoveryConfig &
                /* There must be a unique mdns config but not supported configuration might exist too */
                p_nb_discovery_config > 0 &
                p_mdns_discovery_config_index : 0 .. p_nb_discovery_config - 1
            )
        )
    END;

    p_valid_mdns_server_name <-- check_mdns_server_name (p_mdns_discovery_configuration) =
    PRE
        p_mdns_discovery_configuration : t_MdnsDiscoveryConfig_i &
        p_mdns_discovery_configuration : t_MdnsDiscoveryConfig
    THEN
        p_valid_mdns_server_name :: BOOL
        /* <=> not empty name && name length < 64 bytes */
    END;

    p_valid_server_capabilities <-- check_mdns_server_capabilities (p_mdns_discovery_configuration) =
    PRE
        p_mdns_discovery_configuration : t_MdnsDiscoveryConfig_i &
        p_mdns_discovery_configuration : t_MdnsDiscoveryConfig
    THEN
        p_valid_server_capabilities :: BOOL
        /* <=> valid server capabilities identifiers from spec part 12 */
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
        p_mdns_discovery_config_index : NAT
    THEN
        p_sc :: {e_sc_ok, e_sc_bad_out_of_memory}
    END

END
