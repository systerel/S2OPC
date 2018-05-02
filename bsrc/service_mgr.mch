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
    service_mgr

SEES
    channel_mgr,
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
    a_msg_out_type,
    local_service_treatment;

    d_msg_typing ==
    a_buffer_in_state : t_buffer_in_state_i &
    c_msg_in_header : t_msg_header_i &
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type_i &
    a_buffer_out_state : t_buffer_out_state_i &
    c_msg_out_header : t_msg_header_i &
    c_msg_out : t_msg_i &
    a_msg_out_type : t_msg_type_i &
    local_service_treatment : BOOL

ABSTRACT_VARIABLES
    d_msg_variables

INVARIANT
    d_msg_typing

INITIALISATION
    d_msg_variables :( d_msg_typing )

OPERATIONS

    buffer_out <-- server_receive_session_treatment_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_session_treatment_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        c_msg_in_header   :: t_msg_header_i ||
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read} ||
        CHOICE
            a_buffer_out_state := c_buffer_out_state_indet ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet)
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer)
        END
    END
    ;

    client_receive_session_treatment_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_session_treatment_resps &
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
        c_msg_in_header   :: t_msg_header_i ||
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;

    buffer_out <-- server_receive_session_service_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read} ||
        c_msg_in_header   :: t_msg_header_i ||
        CHOICE
            a_buffer_out_state := c_buffer_out_state_indet ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet)
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer)
        END
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
        c_msg_in_header   :: t_msg_header_i ||
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;

    buffer_out <-- server_receive_discovery_service_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_discovery_service_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        c_msg_in_header   :: t_msg_header_i ||
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read} ||
        CHOICE
            a_buffer_out_state := c_buffer_out_state_indet ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet)
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer)
        END
    END
    ;

    client_receive_discovery_service_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_discovery_service_resps &
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
        c_msg_in_header   :: t_msg_header_i ||
        a_buffer_in_state :: {c_buffer_in_state_indet, e_buffer_in_msg_read}
    END
    ;

    /* Local services on server side: bres <=> out of memory case / unexpected case */
    ret <-- server_receive_local_service_req (endpoint_config_idx, req_class, req_typ, req_msg, app_context) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx &
        req_class : t_msg_service_class_i &
        req_class : t_msg_service_class &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_reqs \/ s_discovery_service_reqs &
        req_msg : t_msg_i &
        req_msg : t_msg &
        app_context : t_application_context_i &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet
    THEN
        a_msg_in_type    :: t_msg_type_i   ||
        c_msg_out_header :: t_msg_header_i ||
        c_msg_out        :: t_msg_i        ||
        ret              :: {e_sc_ok,
                             e_sc_bad_out_of_memory}
    END
    ;

    /* Only called through an internal event sent by session mgr itself */
    buffer_out, req_handle <-- client_service_create_session (session, channel) =
    PRE
        session : t_session_i &
        session : t_session &
        channel : t_channel_i &
        channel : t_channel &
        channel : s_channel_connected &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        CHOICE
            a_buffer_out_state := c_buffer_out_state_indet ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    /* Only to change user on an active channel */
    ret, channel, buffer_out, req_handle <-- client_service_activate_session (session, user) =
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
        CHOICE
            ret :: {e_sc_bad_out_of_memory,
                e_sc_bad_invalid_argument,
                e_sc_bad_invalid_state,          /* session not in a valid state to activate a user on it */
                e_sc_bad_unexpected_error,
                e_sc_bad_identity_token_invalid, /* user identity invalid */
                e_sc_bad_encoding_error,
                e_sc_bad_secure_channel_closed,
                e_sc_bad_connection_closed,
                e_sc_bad_encoding_error} ||
            channel :(channel : t_channel_i & channel = c_channel_indet) ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet) ||
            req_handle := c_client_request_handle_indet
        OR
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    /* Only to set new channel on an orphaned session: called through internal event sent by session mgr itself */
    buffer_out, req_handle  <-- client_service_activate_orphaned_session (session, channel) =
    PRE
        session : t_session_i &
        session : t_session &
        channel : t_channel_i &
        channel : s_channel_connected &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        CHOICE
            a_buffer_out_state := c_buffer_out_state_indet ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    ret, channel, buffer_out, req_handle <-- client_service_close_session (session) =
    PRE
        session : t_session_i &
        session : t_session &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        CHOICE
            ret :: {e_sc_bad_encoding_error,
                e_sc_bad_out_of_memory,
                e_sc_bad_unexpected_error,
                e_sc_bad_invalid_argument,
                e_sc_bad_invalid_state,
                e_sc_bad_secure_channel_closed,
                e_sc_bad_connection_closed} ||
            channel    := c_channel_indet ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            ret        := e_sc_ok ||
            channel    :(channel : t_channel_i & channel : s_channel_connected) ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    ret, channel, buffer_out, req_handle <-- client_service_request (session, req_msg, app_context) =
    PRE
        session         : t_session_i &
        session         : t_session &
        req_msg         : t_msg_i &
        app_context     : t_application_context_i &
        /* message is not yet set as output before call and deallocated after operation */
        c_msg_out        = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type   = c_msg_type_indet
    THEN
        CHOICE
            ret :: {e_sc_bad_invalid_state,  /* session is not in a valid state */
                e_sc_bad_out_of_memory,
                e_sc_bad_unexpected_error,
                e_sc_bad_session_closed,
                e_sc_bad_secure_channel_closed,
                e_sc_bad_connection_closed,
                e_sc_bad_invalid_argument, /* message has not expected type */
                e_sc_bad_encoding_error} ||
            channel    := c_channel_indet ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            ret        := e_sc_ok ||
            channel    :(channel : t_channel_i & channel : s_channel_connected) ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;
    
    ret, buffer_out, req_handle <-- client_discovery_service_request (channel, req_msg, app_context) =
    PRE
        channel     : t_channel_i &
        channel     : t_channel &
        req_msg     : t_msg_i &
        req_msg     : t_msg &
        app_context : t_application_context_i &
        /* message is not yet set as output before call and deallocated after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        CHOICE
            ret :: {e_sc_bad_out_of_memory,
                e_sc_bad_invalid_argument, /* message has not expected type */
                e_sc_bad_encoding_error} ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            ret        := e_sc_ok ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;
    
    client_snd_msg_failure (channel, request_handle, error_status) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        request_handle : t_client_request_handle_i &
        error_status : t_StatusCode_i
    THEN
        skip
    END
    ;
    
    server_evaluate_session_timeout (session) =
    PRE
        session : t_session_i &
        session : t_session
    THEN
        skip
    END
    ;
    
    internal_client_request_timeout (channel, request_handle) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        request_handle : t_client_request_handle_i
    THEN
        skip
    END
    ;

    /* PROMOTES */

    bres <-- client_async_discovery_request_without_channel (channel_config_idx, req_msg, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        req_msg            : t_msg_i &
        req_msg            : t_msg &
        app_context        : t_application_context_i
    THEN
        bres :: BOOL
    END
    ;
    
    /* Generates an event to send the discovery request */
    client_channel_connected_event_discovery (channel_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END
    ;

    /* Generates events of discovery request sending failures if connection failed definitely */
    client_discovery_req_failures_on_final_connection_failure (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;
    
    /* New session: creation and activation async */
    bres <-- client_async_activate_new_session_without_channel (channel_config_idx, user, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        user : t_user_i &
        user : t_user &
        app_context : t_application_context_i &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;

    /* New session: creation sync and activation async */
    bres <-- client_async_activate_new_session_with_channel (channel_config_idx, channel, user, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected &
        user : t_user_i &
        user : t_user &
        app_context : t_application_context_i &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;

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
    
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : t_session)
    END
    ;
    
    client_secure_channel_lost_session_sm (p_lost_channel, p_channel_config_idx) =
    PRE
        p_lost_channel : t_channel_i &
        p_lost_channel : s_channel_connected &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    server_secure_channel_lost_session_sm (p_lost_channel) =
    PRE
        p_lost_channel : t_channel_i &
        p_lost_channel : s_channel_connected
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
    ;
    
    bres <-- is_valid_app_msg_out (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg : t_msg)
    END
    ;
    
    bres, msg_typ <-- is_valid_app_msg_in (msg) =
    PRE
        msg : t_msg_i
    THEN
        CHOICE
            bres := FALSE ||
            msg_typ := c_msg_type_indet
        OR
            bres := TRUE ||
            msg_typ :(msg_typ : t_msg_type_i & msg_typ : t_msg_type)
        END
    END
    ;
    
    bres <-- is_valid_buffer_out (buffer) =
    PRE
        buffer : t_byte_buffer_i
    THEN
        bres := bool(a_buffer_out_state = e_buffer_out_msg_written & buffer : t_byte_buffer)
    END
    ;
    
    bless_msg_out (msg) =
    PRE
        msg : t_msg_i &
        msg : t_msg
    THEN
        c_msg_out := msg ||
        a_msg_out_type :: t_msg_type
    END
    ;
    
    dealloc_msg_out (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type : t_msg_type
    THEN
        c_msg_out := c_msg_indet ||
        a_msg_out_type := c_msg_type_indet
    END
    ;

    bres <-- is_valid_request_context (req_context) =
    PRE
        req_context : t_request_context_i
    THEN
        bres := bool(req_context : t_request_context)
    END
    ;
    
    request_id <-- client_req_handle_to_request_id (req_handle) =
    PRE
        req_handle : t_client_request_handle_i
    THEN
        request_id :: t_request_context_i
    END
    ;
    
    request_handle <-- client_request_id_to_req_handle (request_id) =
    PRE
        request_id : t_request_context_i
    THEN
        request_handle :: t_client_request_handle_i
    END
    ;
    
    client_close_sessions_on_final_connection_failure (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END    
    ;

    client_close_session (session) =
    PRE
        session : t_session_i
    THEN
        skip
    END

END
