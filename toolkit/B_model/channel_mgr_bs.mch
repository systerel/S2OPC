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
    channel_mgr_bs
SEES
    constants
    
DEFINITIONS
    d_variables ==
    s_cli_channel_connecting,
    s_cli_channel_disconnecting,
    s_channel_connected,
    a_config,
    a_endpoint;
    
    d_typage == 
    s_cli_channel_connecting <: t_channel_config_idx &
    s_cli_channel_disconnecting <: t_channel_config_idx &
    s_channel_connected <: t_channel &
    /* each created channel has a configuration
       and there is at most 1 channel per configuration */
    a_config : s_channel_connected >-> t_channel_config_idx &
    a_endpoint : s_channel_connected +-> t_endpoint_config_idx /* TODO: add in invariants that it is empty for clients */
       
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typage
    
INITIALISATION
    s_cli_channel_connecting  :: {{}} ||
    s_cli_channel_disconnecting  :: {{}} ||
    s_channel_connected   :: {{}} ||
    a_config    :: {{}} ||
    a_endpoint  :: {{}}
    
OPERATIONS  
    
    bres <-- cli_open_secure_channel (config_idx) = 
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        IF config_idx : ran(a_config)
        THEN
            bres := FALSE
        ELSE 
            IF card(s_cli_channel_connecting) + card(s_channel_connected) /= card(t_channel)
            THEN
                s_cli_channel_connecting := s_cli_channel_connecting \/ {config_idx} ||
                bres := TRUE
            ELSE
                bres := FALSE
            END
        END
    END
    ;
    
    bres <-- srv_new_secure_channel (endpoint_config_idx, channel_config_idx, channel) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : t_channel
    THEN
        IF channel /: s_channel_connected &
           s_channel_connected /= t_channel
        THEN
            s_channel_connected := s_channel_connected \/ {channel} ||
            a_config(channel)  := channel_config_idx ||
            a_endpoint(channel) := endpoint_config_idx ||
            bres := TRUE
        ELSE
            bres := FALSE
        END
    END;
    
    close_secure_channel (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        IF channel : s_channel_connected
        THEN
            s_cli_channel_disconnecting := s_cli_channel_disconnecting \/ {a_config(channel)}
        END
    END;
    
    bres <-- close_all_channel =
    BEGIN
        s_cli_channel_disconnecting :(s_cli_channel_disconnecting <: t_channel_config_idx) ||
        bres :: BOOL
    END;    
    
    channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        s_channel_connected := s_channel_connected - {channel} ||
        s_cli_channel_disconnecting := s_cli_channel_disconnecting - {a_config(channel)} ||
        a_config := {channel} <<| a_config ||
        a_endpoint := {channel} <<| a_endpoint
    END
    ;
    
    send_channel_msg_buffer (channel, buffer) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected &
        buffer          : t_byte_buffer_i &
        buffer          : t_byte_buffer/* &
        a_buffer_out_state = e_buffer_out_msg_written */
        /* ISSUE: not possible to see message_out_bs */
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
        IF config_idx : s_cli_channel_connecting &
            channel /: s_channel_connected &
            s_channel_connected /= t_channel
        THEN
            s_cli_channel_connecting := s_cli_channel_connecting - {config_idx} ||
            s_channel_connected := s_channel_connected \/ {channel} ||
            a_config(channel)  := config_idx
        ELSE
            bres := FALSE
        END  
    END
    ;

    bres <-- cli_set_connection_timeout_channel (config_idx) =
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        IF config_idx : s_cli_channel_connecting
        THEN
            s_cli_channel_connecting := s_cli_channel_connecting - {config_idx} ||
            bres := TRUE
        ELSE
            bres := FALSE
        END  
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
        IF a_config~[{config_idx}] = {}
        THEN
            channel := c_channel_indet
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
        channel         : t_channel_i &
        channel         : s_channel_connected
    THEN
        bres := bool(channel /: dom(a_endpoint))
    END
    ;
    
    endpoint_config_idx <-- server_get_endpoint_config (channel) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected
    THEN
        endpoint_config_idx 
        :(endpoint_config_idx : t_endpoint_config_idx_i &
            endpoint_config_idx = a_endpoint(channel))
    END

END
