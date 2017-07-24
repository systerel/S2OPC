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
    constants,
    message_in_bs
    
DEFINITIONS
    d_variables ==
    s_cli_channel_connecting,
    s_channel_connected,
    a_config,
    a_endpoint;
    
    d_typage == 
    s_cli_channel_connecting <: t_channel_config_idx &
    s_channel_connected <: t_channel &
    /* each created channel has a configuration
       and there is at most 1 channel per configuration */
    a_config : s_channel_connected >-> t_channel_config_idx &
    a_endpoint : s_channel_connected +-> t_endpoint_config_idx
       
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typage
    
INITIALISATION
    s_cli_channel_connecting  :: {{}} ||
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
            s_channel_connected := s_channel_connected - {channel} ||
            a_config := {channel} <<| a_config ||
            a_endpoint := {channel} <<| a_endpoint
        END           
    END;
    
    close_all_channel =
    BEGIN
        s_channel_connected :: {} ||
        a_config :: {} ||
        a_endpoint :: {}
    END;    
    
    channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        s_channel_connected := s_channel_connected - {channel} ||
        a_config := {channel} <<| a_config ||
        a_endpoint := {channel} <<| a_endpoint
    END
    ;
    
    ret <-- send_channel_msg (channel, msg) =
    PRE
        channel         : t_channel_i &
        channel         : t_channel &
        msg             : t_msg_i &
        msg             : t_msg
    THEN
        ret :: {e_sc_ok, 
            e_sc_bad_invalid_state,  /* channel is not in a valid state*/
            e_sc_bad_secure_channel_closed, e_sc_bad_connection_closed,
            e_sc_bad_encoding_error}
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
    
    bres <-- is_connected_channel (channel) =
    PRE
        channel : t_channel_i
    THEN
        bres := bool(channel : s_channel_connected)
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

END
