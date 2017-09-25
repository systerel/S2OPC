/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    a_config,
    a_endpoint
    ;

    d_typage ==
    s_cli_channel_connecting    <: t_channel_config_idx                &
    s_cli_channel_disconnecting <: t_channel_config_idx                &
    s_channel_connected         <: t_channel                           &
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
    a_config                    := {} ||
    a_endpoint                  := {}

OPERATIONS

    bres <-- is_connected_channel (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
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

    add_channel_connected(p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        s_channel_connected := s_channel_connected \/ {p_channel}
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
        s_channel_connected := s_channel_connected - {p_channel}
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

END
