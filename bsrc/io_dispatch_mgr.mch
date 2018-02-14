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
    io_dispatch_mgr

SEES
    constants

DEFINITIONS
    d_msg_variables ==
    a_buffer_in_state,
    c_msg_in_header,
    c_msg_in,
    a_msg_in_type,
    a_buffer_out_state,
    c_msg_out_header,
    c_msg_out,
    a_msg_out_type;

    d_msg_typing ==
    a_buffer_in_state : t_buffer_in_state_i &
    c_msg_in_header : t_msg_header_i &
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type_i &
    a_buffer_out_state : t_buffer_out_state_i &
    c_msg_out_header : t_msg_header_i &
    c_msg_out : t_msg_i &
    a_msg_out_type : t_msg_type_i

ABSTRACT_VARIABLES
    d_msg_variables

INVARIANT
    d_msg_typing

INITIALISATION
    d_msg_variables :( d_msg_typing )

OPERATIONS

    receive_msg_buffer (channel, buffer, request_context) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        buffer   : t_byte_buffer_i &
        buffer   : t_byte_buffer &
        request_context : t_request_context_i &
        a_buffer_in_state = e_buffer_in_msg_not_read &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        /* End of receive execution lead to a buffer in invalid state (error) or a buffer fully read */
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
        /* c_msg_in instance could be provided to app but should be not referenced anymore in c_msg_in */
    END
    ;

    client_channel_connected_event (channel_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : t_channel &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
    END
    ;

    client_secure_channel_timeout (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    bres <-- server_channel_connected_event (endpoint_config_idx, channel_config_idx, channel) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : t_channel
    THEN
        bres :: BOOL
    END
    ;

    /* Only one session could be in activation until the first session is notified as activated */
    bres <-- client_activate_new_session (channel_config_idx, user, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        user : t_user_i &
        user : t_user &
        app_context : t_application_context_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;

    /* Only to change user on an active channel */
    ret <-- client_reactivate_session_new_user (session, user) =
    PRE
        session : t_session_i &
        user : t_user_i &
        user : t_user &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
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

    ret <-- client_send_close_session_request (session) =
    PRE
        session : t_session_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
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
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
    END
    ;

    ret <-- client_send_service_request (session, req_msg, app_context) =
    PRE
        session         : t_session_i &
        req_msg         : t_msg_i &
        app_context : t_application_context_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* message is unkown before call and deallocated after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        ret :: {e_sc_ok,
            e_sc_bad_invalid_state,  /* session is not in a valid state */
            e_sc_bad_out_of_memory,
            e_sc_bad_unexpected_error,
            e_sc_bad_session_closed,
            e_sc_bad_secure_channel_closed,
            e_sc_bad_connection_closed,
            e_sc_bad_invalid_argument, /* message has not expected type */
            e_sc_bad_encoding_error}
    END
    ;
    
    
    ret <-- client_send_discovery_request (channel_config_idx, req_msg, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        req_msg            : t_msg_i &
        app_context        : t_application_context_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* message is unkown before call and deallocated after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        ret :: {e_sc_ok,
            e_sc_bad_out_of_memory,
            e_sc_bad_invalid_argument, /* message has not expected type */
            e_sc_bad_encoding_error,
            e_sc_bad_too_many_ops, /* An operation is already recorded to be sen when SC will be connected */
            e_sc_bad_generic}
    END
    ;

    internal_client_create_session (session, channel_config_idx) =
    PRE
        session : t_session_i &
        session : t_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
    END
    ;

    internal_client_activate_orphaned_session (session, channel_config_idx) =
    PRE
        session : t_session_i &
        session : t_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
    END
    ;

    bres <-- close_all_active_connections =
    BEGIN
        bres :: BOOL // indicates if active connection will be closed
    END

END
