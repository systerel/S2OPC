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
    channel_mgr

SEES
    constants,
    constants_statuscodes_bs

DEFINITIONS
    d_variables ==
    s_cli_channel_connecting,
    s_cli_channel_disconnecting,
    s_channel_connected,
    a_channel_connected_time,
    a_config,
    a_endpoint,
    a_SecurityPolicy;

    d_typage ==
    s_cli_channel_connecting <: t_channel_config_idx &
    s_cli_channel_disconnecting <: t_channel_config_idx &
    s_channel_connected <: t_channel &
    a_channel_connected_time : t_channel +-> t_timeref &
    /* each created channel has a configuration
       and there is at most 1 channel per configuration */
    a_config : s_channel_connected >-> t_channel_config_idx &
    /* a_endpoint is used to retrieve the endpoint configuration index
       on which channel is connected (server side only):
       channel :  dom(a_endpoint) => channels connected onto current server instance &
       channel /: dom(a_endpoint) => channels of the current client instance
    */
    a_endpoint : s_channel_connected +-> t_endpoint_config_idx &
    a_SecurityPolicy : s_channel_connected --> t_SecurityPolicy


ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typage

INITIALISATION
    d_variables :(d_typage)

OPERATIONS

    bres <-- cli_open_secure_channel (config_idx, is_one_sc_auto_closing) =
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx &
        is_one_sc_auto_closing : BOOL /* TRUE => a SC has been closed by auto-close prior to calling this operation */
    THEN
        bres,
        s_cli_channel_connecting
        :(  bres : BOOL &
            s_cli_channel_connecting <: t_channel_config_idx
         )
    END
    ;

    bres <-- srv_new_secure_channel (endpoint_config_idx, channel_config_idx, channel, is_one_sc_auto_closing) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : t_channel &
        is_one_sc_auto_closing : BOOL /* TRUE => a SC has been closed by auto-close prior to calling this operation */
    THEN
       bres,
       s_channel_connected,
       a_config,
       a_endpoint,
       a_SecurityPolicy
       :(bres : BOOL &
         s_channel_connected <: t_channel &
         a_config : s_channel_connected >-> t_channel_config_idx &
         a_endpoint : s_channel_connected +-> t_endpoint_config_idx &
         a_SecurityPolicy : s_channel_connected --> t_SecurityPolicy
        )
    END;

    close_secure_channel (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        s_cli_channel_disconnecting
        :(  s_cli_channel_disconnecting <: t_channel_config_idx
         )
    END;

    bres <-- close_all_channel (p_clientOnly) =
    PRE
        p_clientOnly : BOOL
    THEN
        s_cli_channel_disconnecting,
        bres
        :(  s_cli_channel_disconnecting <: t_channel_config_idx &
            bres : BOOL
         )
    END;

    channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        s_channel_connected,
        s_cli_channel_disconnecting,
        a_config,
        a_endpoint,
        a_SecurityPolicy
        :(  s_channel_connected <: t_channel &
            s_cli_channel_disconnecting <: t_channel_config_idx &
            a_config : s_channel_connected >-> t_channel_config_idx &
            a_endpoint : s_channel_connected +-> t_endpoint_config_idx &
            a_SecurityPolicy : s_channel_connected --> t_SecurityPolicy
         )
    END
    ;

    bres <-- is_valid_channel_config_idx (p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i
    THEN
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
        /* No return status code, it is an asynchronous action */
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

    bres <-- cli_set_connected_channel (config_idx, channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        bres,
        s_cli_channel_connecting,
        s_channel_connected,  /* TODO FBA invariant sur a_endpoint qui dépend de s_channel_connected non prouvable en l'état */
        a_config,
        a_SecurityPolicy
        :(  bres : BOOL &
            s_cli_channel_connecting <: t_channel_config_idx &
            s_channel_connected <: t_channel &
            a_config : s_channel_connected >-> t_channel_config_idx &
            a_SecurityPolicy : s_channel_connected --> t_SecurityPolicy
         )
    END
    ;

    bres <-- cli_set_connection_timeout_channel (config_idx) =
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        s_cli_channel_connecting,
        bres
        :(  s_cli_channel_connecting <: t_channel_config_idx &
            bres : BOOL
         )
    END
    ;

    bres <-- is_connected_channel (channel) =
    PRE
        channel : t_channel_i
    THEN
        bres := bool(channel : s_channel_connected)
    END
    ;

    bres <-- is_disconnecting_channel (config_idx) =
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        bres := bool(config_idx : s_cli_channel_disconnecting)
    END
    ;

    channel <-- get_connected_channel (config_idx) = /* security configuration concerns must be added*/
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        IF config_idx /: dom(a_config~)
        THEN
            channel :(channel : t_channel_i & channel = c_channel_indet)
        ELSE
            channel := a_config~(config_idx)
        END
    END
    ;

    config_idx <-- get_channel_info (channel) = /* security configuration concerns must be added*/
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        IF channel /: s_channel_connected
        THEN
            config_idx := c_channel_config_idx_indet
        ELSE
            config_idx := a_config(channel)
        END
    END
    ;

    bres <-- is_client_channel (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        bres := bool(channel /: dom(a_endpoint))
    END
    ;

    endpoint_config_idx <-- server_get_endpoint_config (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        IF channel /: dom(a_endpoint) THEN
           endpoint_config_idx := c_endpoint_config_idx_indet
        ELSE
           endpoint_config_idx := a_endpoint(channel)
        END
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
    ;

    p_timeref <-- get_connection_time(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : s_channel_connected
    THEN
        p_timeref :(p_timeref : t_timeref_i & p_timeref = a_channel_connected_time(p_channel))
    END
    ;

    p_auto_closed_active <-- is_auto_close_channel_active =
    BEGIN
        p_auto_closed_active := bool(card(s_cli_channel_connecting) + card(s_channel_connected) >= c_max_channels_connected)
    END
END
