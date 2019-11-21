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
    channel_mgr_1

SEES
    constants

DEFINITIONS
    d_variables ==
    s_cli_channel_connecting,
    s_cli_channel_disconnecting,
    s_channel_connected,
    a_channel_connected_time,
    a_config,
    a_endpoint
    ;

    d_typage ==
    s_cli_channel_connecting    <: t_channel_config_idx                &
    s_cli_channel_disconnecting <: t_channel_config_idx                &
    s_channel_connected         <: t_channel                           &
    a_channel_connected_time     : t_channel +-> t_timeref             &
    dom(a_channel_connected_time) = s_channel_connected                &
    a_config                     : t_channel +-> t_channel_config_idx  &
    a_endpoint                   : t_channel +-> t_endpoint_config_idx

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typage

INITIALISATION
    s_cli_channel_connecting    := {} ||
    s_cli_channel_disconnecting := {} ||
    s_channel_connected         := {} ||
    a_channel_connected_time    := {} ||
    a_config                    := {} ||
    a_endpoint                  := {}

OPERATIONS

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

    channel <-- get_connected_channel (config_idx) =
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        IF config_idx /: dom(a_config~)
        THEN
            channel := c_channel_indet
        ELSE
            channel := a_config~(config_idx)
        END
    END
    ;

    config_idx <-- get_channel_info (channel) =
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

    p_dom, p_channel <-- getall_config_inv(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        p_dom := bool(p_config_idx : dom(a_config~)) ||
        IF  p_config_idx /: dom(a_config~) THEN
            p_channel := c_channel_indet
        ELSE
            p_channel := a_config~(p_config_idx)
        END
    END
    ;

    p_card_connecting <-- get_card_cli_channel_connecting =
    BEGIN
        p_card_connecting := card(s_cli_channel_connecting)
    END
    ;

    p_card_connected <-- get_card_channel_connected =
    BEGIN
        p_card_connected := card(s_channel_connected)
    END
    ;

    p_card_used <-- get_card_channel_used =
    BEGIN
        p_card_used := card(s_cli_channel_connecting) + card(s_channel_connected)
    END
    ;

    add_cli_channel_connecting(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        s_cli_channel_connecting := s_cli_channel_connecting \/ {p_config_idx}
    END
    ;

    p_con <-- is_channel_connected(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        p_con := bool(p_channel : s_channel_connected)
    END
    ;

    add_channel_connected(p_channel, p_timeref) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel   &
        p_channel : s_channel_connected &

        p_timeref : t_timeref_i &
        p_timeref : t_timeref
    THEN
        s_channel_connected,
        a_channel_connected_time
        :(s_channel_connected = s_channel_connected$0 \/ {p_channel} &
          a_channel_connected_time = a_channel_connected_time$0 <+ {p_channel |-> p_timeref})
    END
    ;

    set_config(p_channel, p_channel_config_idx) =
    PRE
        p_channel            : t_channel_i            &
        p_channel            : t_channel              &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        a_config(p_channel) := p_channel_config_idx
    END
    ;

    set_endpoint(p_channel, p_endpoint_config_idx) =
    PRE
        p_channel             : t_channel_i             &
        p_channel             : t_channel               &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        a_endpoint(p_channel) := p_endpoint_config_idx
    END
    ;

    p_dom, p_config_idx <-- getall_channel_connected(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        p_dom := bool(p_channel : dom(a_config)) ||
        IF  p_channel /: dom(a_config) THEN
            p_config_idx := c_channel_config_idx_indet
        ELSE
            p_config_idx := a_config(p_channel)
        END
    END
    ;

    p_is_channel_connecting <-- is_cli_channel_connecting(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        p_is_channel_connecting := bool(p_config_idx : s_cli_channel_connecting)
    END
    ;

    add_cli_channel_disconnecting(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        s_cli_channel_disconnecting := s_cli_channel_disconnecting \/ {p_config_idx}
    END
    ;

    remove_channel_connected(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        s_channel_connected := s_channel_connected - {p_channel} ||
        a_channel_connected_time := {p_channel} <<| a_channel_connected_time
    END
    ;

    remove_cli_channel_disconnecting(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        s_cli_channel_disconnecting := s_cli_channel_disconnecting - {p_config_idx}
    END
    ;

    reset_config(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        a_config := {p_channel} <<| a_config
    END
    ;

    reset_endpoint(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        a_endpoint := {p_channel} <<| a_endpoint
    END
    ;

    remove_cli_channel_connecting(p_config_idx) =
    PRE
        p_config_idx : t_channel_config_idx_i &
        p_config_idx : t_channel_config_idx
    THEN
        s_cli_channel_connecting := s_cli_channel_connecting - {p_config_idx}
    END
    ;

    p_timeref <-- get_connection_time(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : s_channel_connected
    THEN
        p_timeref :(p_timeref : t_timeref_i & p_timeref = a_channel_connected_time(p_channel))
    END

END
