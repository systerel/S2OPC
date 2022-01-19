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

    send_channel_msg_buffer (channel, buffer, request_context) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected &
        buffer          : t_byte_buffer_i &
        buffer          : t_byte_buffer &
        /* a_buffer_out_state = e_buffer_out_msg_written */
        /* ISSUE: not possible to see message_out_bs with AtelierB => TC failure / m -tc OK */
        request_context : t_request_context_i
    THEN
        skip
    END
    ;

    send_channel_error_msg (channel, status_code, request_context) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected &
        status_code     : t_StatusCode_i &
        status_code     : t_StatusCode &
        request_context : t_request_context_i
    THEN
        /* No return status code, it is an asynchronous action */
        skip
    END
    ;

    prepare_cli_open_secure_channel(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    finalize_close_secure_channel(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
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
