/*
 *  Copyright (C) 2018 Systerel and others.
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
    
    last_connected_channel_lost =
    BEGIN
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
