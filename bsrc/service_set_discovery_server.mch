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
    service_set_discovery_server
SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    ret <-- treat_find_servers_request (req_msg, resp_msg, endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_find_servers_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_find_servers_resp
    THEN
        ret :(ret : t_StatusCode_i & ret : t_StatusCode)
    END
    ;

    ret <-- treat_find_servers_on_network_request (req_msg, resp_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_find_servers_on_network_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_find_servers_on_network_resp
    THEN
        ret :(ret : t_StatusCode_i & ret : t_StatusCode)
    END
    ;

    ret <-- treat_register_server2_request (req_msg, resp_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_discovery_register_server2_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_discovery_register_server2_resp
    THEN
        ret :(ret : t_StatusCode_i & ret : t_StatusCode)
    END
    ;

    service_set_discovery_server_UNINITIALISATION =
    BEGIN
        skip
    END

END
