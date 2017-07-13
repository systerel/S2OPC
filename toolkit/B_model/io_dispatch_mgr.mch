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
    io_dispatch_mgr

SEES
    constants

DEFINITIONS
    d_variables ==
    c_msg_in,
    a_msg_in_type,
    c_msg_out;

    d_typage ==
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type &
    c_msg_out : t_msg_i

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typage

INITIALISATION
    c_msg_in  :: t_msg_i            ||
    a_msg_in_type :: t_msg_type     ||
    c_msg_out  :: t_msg_i

OPERATIONS

    msgs_memory_changed =
    BEGIN
        d_variables :( d_typage )
    END
    ;

    receive_msg (channel, msg) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        msg      : t_msg_i &
        msg      = c_msg_in &
        c_msg_in : t_msg &
        c_msg_out = c_msg_indet
    THEN
        skip
    END
    ;
    
    // TODO: add connection id / sc parameter
    channel_connected_event (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;
    
    ret <-- activate_new_session (channel_config_idx, user) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        user : t_user_i &
        user : t_user &
        c_msg_out = c_msg_indet
    THEN
        ret :: {e_sc_ok, e_sc_nok, e_sc_bad_out_of_memory}
        
    END
    ;

    
    /* Only to change user on an active channel */
    ret <-- activate_session (session, user) =
    PRE
        session : t_session_i &
        session : t_session &
        user : t_user_i &
        user : t_user &
        c_msg_out = c_msg_indet
    THEN
        ret :: {e_sc_ok,
            e_sc_bad_out_of_memory,
            e_sc_bad_invalid_argument,
            e_sc_bad_invalid_state,          /* session not in a valid state to activate a user on it */
            e_sc_bad_unexpected_error,
            e_sc_bad_identity_token_invalid, /* user identity invalid */
            e_sc_bad_encoding_error,
            e_sc_bad_secure_channel_closed,
            e_sc_bad_connection_closed,
            e_sc_bad_encoding_error}
    END
    ;

    ret <-- close_session (session) =
    PRE
        session : t_session_i &
        session : t_session &
        c_msg_out = c_msg_indet
    THEN
        ret :: {e_sc_ok,
            e_sc_bad_encoding_error,
            e_sc_bad_out_of_memory,
            e_sc_bad_unexpected_error,
            e_sc_bad_invalid_argument,
            e_sc_bad_invalid_state,
            e_sc_bad_secure_channel_closed,
            e_sc_bad_connection_closed}
    END
    ;

    secure_channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        c_msg_out = c_msg_indet
    THEN
        skip
    END
    ;

    ret <-- send_service_request_msg (session, req_msg) =
    PRE
        session         : t_session_i &
        session         : t_session &
        req_msg         : t_msg_i &
        req_msg         = c_msg_in &
        c_msg_in        : t_msg
    THEN
        ret :: {e_sc_ok,
            e_sc_bad_invalid_state,  /* session is not in a valid state */
            e_sc_bad_out_of_memory,
            e_sc_bad_unexpected_error,
            e_sc_bad_session_closed,
            e_sc_bad_secure_channel_closed,
            e_sc_bad_connection_closed,
            e_sc_bad_invalid_argument, /* message has not expected type */
            e_sc_bad_encoding_error} ||
        c_msg_out  :: t_msg_i
    END
    ;

    close_all_active_connections = skip;

    state <-- get_session_state_or_closed (session) =
    PRE
        session : t_session_i
    THEN
        state :: t_sessionState
    END

END
