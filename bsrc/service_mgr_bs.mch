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
    service_mgr_bs

SEES
    constants,
    channel_mgr

DEFINITIONS
    d_variables ==
    a_client_discovery_req_to_send
    ;

    d_typing ==
    a_client_discovery_req_to_send : t_channel_config_idx +-> t_msg

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    d_variables :(d_typing)

OPERATIONS

    /* Needs UNINIT to deallocate non sent messages on uninit */
    service_mgr_bs_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    /* Discovery request without connected channel: keep request to send it later */
    bres <-- client_async_discovery_request_without_channel (channel_config_idx, req_msg, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        req_msg            : t_msg_i &
        req_msg            : t_msg &
        app_context        : t_application_context_i
    THEN
        bres :: BOOL
    END
    ;

    /* Generates an event to send the discovery request */
    client_channel_connected_event_discovery (channel_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END
    ;

    client_discovery_req_failures_on_final_connection_failure (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END

END
