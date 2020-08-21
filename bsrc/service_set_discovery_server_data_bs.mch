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
    service_set_discovery_server_data_bs

SEES
    constants

ABSTRACT_VARIABLES
    RegisteredServer_ServerUri,
    MdnsDiscoveryConfig_HasServerCapabilities,
    ServerUriArray_HasServerUri
INVARIANT
    RegisteredServer_ServerUri : t_RegisteredServer --> t_ServerUri &
    MdnsDiscoveryConfig_HasServerCapabilities : t_MdnsDiscoveryConfig <-> t_ServerCapabilities &
    ServerUriArray_HasServerUri : t_ServerUris <-> t_ServerUri
INITIALISATION
    RegisteredServer_ServerUri :(RegisteredServer_ServerUri : t_RegisteredServer --> t_ServerUri) ||
    MdnsDiscoveryConfig_HasServerCapabilities
     :(MdnsDiscoveryConfig_HasServerCapabilities : t_MdnsDiscoveryConfig <-> t_ServerCapabilities) ||
    ServerUriArray_HasServerUri :(ServerUriArray_HasServerUri : t_ServerUris <-> t_ServerUri)

OPERATIONS

    p_server_uri <-- get_RegisteredServer_ServerUri(p_reg_server) =
    PRE
        p_reg_server : t_RegisteredServer_i &
        p_reg_server : t_RegisteredServer
    THEN
        p_server_uri := RegisteredServer_ServerUri(p_reg_server)
    END
    ;

    p_bool <-- is_equal_ServerUri (p_left, p_right) =
    PRE
        p_left : t_ServerUri &
        p_right : t_ServerUri
    THEN
        p_bool := bool(p_left = p_right)
    END
    ;

    p_bool <-- is_empty_ServerUri (p_server_uri) =
    PRE
        p_server_uri : t_ServerUri
    THEN
        p_bool := bool(p_server_uri = c_ServerUri_empty)
    END
    ;

    p_bool <-- has_ServerCapabilities (p_mdns_config, p_server_capabilities) =
    PRE
        p_mdns_config : t_MdnsDiscoveryConfig_i &
        p_mdns_config : t_MdnsDiscoveryConfig &
        p_server_capabilities : t_ServerCapabilities
    THEN
        p_bool := bool(p_mdns_config |-> p_server_capabilities : MdnsDiscoveryConfig_HasServerCapabilities)
    END
    ;

    p_bool <-- has_ServerUri (p_singleServerUri, p_nbServerUri, p_ServerUris) =
    PRE
        p_singleServerUri : t_ServerUri &
        p_nbServerUri : INT &
        p_nbServerUri > 0 &
        p_ServerUris : t_ServerUris
    THEN
        p_bool := bool(p_ServerUris |-> p_singleServerUri : ServerUriArray_HasServerUri)
    END
    ;

    p_app_desc <-- get_ApplicationDescription (p_endpoint_config_idx) =
    PRE
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_app_desc
        :(p_app_desc : t_ApplicationDescription_i &
          p_app_desc : t_ApplicationDescription)
    END
    ;

    p_ServerUri <-- get_ApplicationDescription_ServerUri (p_app_desc) =
    PRE
        p_app_desc : t_ApplicationDescription_i &
        p_app_desc : t_ApplicationDescription
    THEN
        p_ServerUri :: t_ServerUri
    END

END
