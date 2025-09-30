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
    channel_mgr_bs
SEES
    constants,
    constants_statuscodes_bs,
    channel_mgr_1

ABSTRACT_VARIABLES
    a_SecurityPolicy

INVARIANT
    a_SecurityPolicy : t_channel +-> t_SecurityPolicy

INITIALISATION
    a_SecurityPolicy :: {{}}

OPERATIONS

    bres <-- is_valid_channel_config_idx (p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i
    THEN
        /* Check a config index from application is non undetermined and has a configuration associated */
        bres := bool(p_config_idx : t_channel_config_idx)
    END
    ;

    bres <-- is_valid_endpoint_config_idx (p_config_idx) =
    PRE
        p_config_idx : t_endpoint_config_idx_i
    THEN
        bres := bool(p_config_idx : t_endpoint_config_idx)
    END
    ;

    prepare_cli_open_secure_channel(p_config_idx, p_reverse_endpoint_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx &
        p_reverse_endpoint_config_idx : t_reverse_endpoint_config_idx_i
    THEN
        skip
    END
    ;

    finalize_close_secure_channel(p_channel, p_statusCode) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel &
        p_statusCode : t_StatusCode_i &
        p_statusCode : t_StatusCode
    THEN
        skip
    END
    ;

    last_connected_channel_lost(p_clientOnly) =
    PRE
        p_clientOnly : BOOL
    THEN
        skip
    END
    ;

    notify_client_connection_closed(p_config_idx, p_close_ctx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx &
        p_close_ctx  : t_application_context_i &
        p_close_ctx /= c_no_application_context
    THEN
        skip
    END
    ;

    define_SecurityPolicy(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel   &
        p_channel : s_channel_connected
    THEN
        a_SecurityPolicy
        :(  a_SecurityPolicy : t_channel +-> t_SecurityPolicy &
            {p_channel} <<| a_SecurityPolicy = {p_channel} <<| a_SecurityPolicy$0 &
            p_channel : dom(a_SecurityPolicy)
         )
    END
    ;

    reset_SecurityPolicy(channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        a_SecurityPolicy := {channel} <<| a_SecurityPolicy
    END
    ;

    secpol <-- get_SecurityPolicy(channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        secpol := a_SecurityPolicy(channel)
    END
    ;

    channel_do_nothing(channel) =
    PRE
        channel : t_channel_i
    THEN
        skip
    END

END
