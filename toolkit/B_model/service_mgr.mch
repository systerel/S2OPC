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
    service_mgr
    
SEES
    channel_mgr_bs,
    constants

DEFINITIONS
    d_msg_variables ==
    a_buffer_in_state,
    c_msg_in_header,
    c_msg_in,
    a_msg_in_type,
    c_msg_out_header,
    c_msg_out,
    a_msg_out_type;
    
    d_msg_typing == 
    a_buffer_in_state : t_buffer_in_state_i &
    c_msg_in_header : t_msg_header_i &
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type_i &
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
    
    server_receive_session_treatment_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_service_treatment_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    client_receive_session_treatment_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_service_treatment_resps &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    server_receive_session_service_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    client_receive_session_service_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_resps &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    server_receive_public_service_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_public_service_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    client_receive_public_service_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_public_service_resps &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;
    
    /* New session: creation and activation async */
    bres <-- client_async_activate_new_session_without_channel (channel_config_idx, user) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        user : t_user_i &
        user : t_user &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;
    
    /* New session: creation sync and activation async */
    bres <-- client_async_activate_new_session_with_channel (channel_config_idx, channel, user) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected &
        user : t_user_i &
        user : t_user &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;
    
    /* Only called through an internal event sent by session mgr itself */
    client_create_session (session, channel) =
    PRE
        session : t_session_i &
        session : t_session &
        channel : t_channel_i &
        channel : t_channel &
        channel : s_channel_connected &
        c_msg_out = c_msg_indet
    THEN
        skip
    END
    ;
    
    /* Only to change user on an active channel */
    ret <-- client_activate_session (session, user) =
    PRE
        session : t_session_i &
        session : t_session &
        user : t_user_i &
        user : t_user &
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
    
    /* Only to set new channel on an orphaned session: called through internal event sent by session mgr itself */
    client_activate_orphaned_session (session, channel) =
    PRE
        session : t_session_i &
        session : t_session &
        channel : t_channel_i &
        channel : s_channel_connected &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
    END
    ;
    
    ret <-- client_close_session (session) =
    PRE
        session : t_session_i &
        session : t_session &
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
    
    ret <-- client_send_service_request_msg (session, req_msg) =
    PRE
        session         : t_session_i &
        session         : t_session &
        req_msg         : t_msg_i &
        req_msg         : t_msg &
        /* No message allocation before and remaining after operation */
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
    
    /* PROMOTES */
    client_channel_connected_event_session (channel_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END
    ;
      
    client_secure_channel_lost (lost_channel, channel_config_idx) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : s_channel_connected &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;
    
    server_secure_channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END
    ;
    
    msg_typ <-- decode_msg_type (msg_buffer) =
    PRE
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_not_read &
        c_msg_in_header = c_msg_header_indet &
        c_msg_in = c_msg_indet &
        a_msg_in_type = c_msg_type_indet
    THEN
        CHOICE
            a_buffer_in_state := e_buffer_in_msg_type_read ||
            a_msg_in_type :(a_msg_in_type : t_msg_type_i & a_msg_in_type : t_msg_type) ||
            msg_typ := a_msg_in_type
        OR
            a_buffer_in_state := c_buffer_in_state_indet ||
            msg_typ := c_msg_type_indet
        END
    END
    ;
    
    bres <-- is_valid_msg_in_type (msg_typ) =
    PRE
        msg_typ : t_msg_type_i
    THEN
        bres := bool(msg_typ = a_msg_in_type & a_msg_in_type : t_msg_type)
    END

END
