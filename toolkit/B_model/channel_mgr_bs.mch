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
    s_channel,
    s_is_client,
    a_config;
    
    d_typage == 
    s_channel <: t_channel &
    s_is_client <: s_channel &
    /* each created channel has a configuration
       and there is at most 1 channel per configuration */
    a_config : s_channel >-> t_channel_config_idx
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typage
    
INITIALISATION
    s_channel   :: {{}} ||
    s_is_client :: {{}} ||
    a_config    :: {{}}
    
OPERATIONS  
    
    nchannel, is_connected <-- open_secure_channel (config_idx) = 
    PRE
        config_idx : t_channel_config_idx_i &
        config_idx : t_channel_config_idx
    THEN
        IF s_channel /= t_channel & config_idx : t_channel_config_idx
        THEN
            IF config_idx : ran(a_config)
            THEN
                nchannel :(nchannel : t_channel_i & nchannel = a_config~(config_idx)) ||
                is_connected := TRUE
            ELSE                
                CHOICE
                    ANY l_channel WHERE
                        l_channel : t_channel_i &
                        l_channel : t_channel - s_channel
                    THEN
                        s_channel := s_channel \/ {l_channel} ||
                        s_is_client := s_is_client \/ {l_channel} ||
                        a_config(l_channel)  := config_idx ||
                        nchannel := l_channel ||
                        is_connected := FALSE
                    END
                OR
                    /* invalid parameters: URL, security, crypto, etc. */
                    nchannel := c_channel_indet ||
                    is_connected := FALSE
                END
            END
        ELSE
            nchannel := c_channel_indet ||
            is_connected := FALSE
        END    
    END
    ;
    
    close_secure_channel (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        IF channel : s_channel
        THEN
            s_channel := s_channel - {channel} ||
            s_is_client := s_is_client - {channel} ||
            a_config := {channel} <<| a_config
        END           
    END;
    
    close_all_channel =
    BEGIN
        s_channel :: {} ||
        s_is_client :: {} ||
        a_config :: {}
    END;    
    
    channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel
    THEN
        s_channel := s_channel - {channel} ||
        s_is_client := s_is_client - {channel} ||
        a_config := {channel} <<| a_config
    END
    ;
    
    receive_sc_msg (channel, msg) =
    PRE
        channel : t_channel_i & /* channel can be indetermined in case it is a new connection (HEL) */
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg
    THEN
        s_channel, s_is_client, a_config :(d_typage) /* channel set can be changed in case it is a new connection */
    END
    ;
    
    receive_hello_msg (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_tcpua_hello
    THEN
        s_channel, a_config :(d_typage) /* channel set can be changed in case it is a new connection */
    END
    ;
    
    receive_channel_msg (channel, msg) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : {e_msg_tcpua_ack,
            e_msg_sc_open_channel_req, e_msg_sc_open_channel_resp,
            e_msg_sc_close_channel_req, e_msg_sc_close_channel_resp,
            e_msg_public_service_req, e_msg_public_service_resp}
    THEN
        s_channel, s_is_client, a_config :(d_typage) /* channel set can be changed in case it is a new connection */
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
    
    bres <-- is_valid_channel (channel) =
    PRE
        channel : t_channel_i
    THEN
        bres := bool(channel : s_channel)
    END
    ;
    
    channel <-- get_valid_channel (config_idx) = /* security configuration concerns must be added*/
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
        IF channel /: s_channel
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
        channel         : s_channel
    THEN
        bres := bool(channel : s_is_client)
    END

END
