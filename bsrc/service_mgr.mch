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
    service_mgr

SEES
    channel_mgr,
    constants,
    constants_statuscodes_bs

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

    service_class <-- get_msg_service_class (msg_typ) =
    PRE
        msg_typ : t_msg_type_i &
        msg_typ : t_msg_type
    THEN
        service_class
        :(  service_class : t_msg_service_class_i &
            service_class : t_msg_service_class   &

            (   msg_typ : s_discovery_service_reqs \/ s_discovery_service_resps
                =>
                service_class = e_msg_discovery_service_class) &
            (   msg_typ : s_service_on_session_reqs \/ s_service_on_session_resps
                =>
                service_class = e_msg_session_service_class) &
            (   msg_typ : s_session_treatment_reqs \/ s_session_treatment_resps
                =>
                service_class = e_msg_session_treatment_class) &
            (   not(msg_typ : s_discovery_service_reqs  \/ s_discovery_service_resps)  &
                not(msg_typ : s_service_on_session_reqs \/ s_service_on_session_resps) &
                not(msg_typ : s_session_treatment_reqs  \/ s_session_treatment_resps)
                =>
                service_class = e_msg_service_fault_class)
        )
    END
    ;

    valid, resp_typ <-- client_service_fault_to_resp_type (msg_buffer) =
    PRE
        a_msg_in_type = e_msg_service_fault_resp &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        c_msg_in_header = c_msg_header_indet &
        c_msg_in = c_msg_indet
    THEN
        CHOICE
            valid := TRUE ||
            resp_typ
            :(resp_typ : t_msg_type_i &
              resp_typ : s_service_resps - {e_msg_service_fault_resp})
        OR
            valid := FALSE ||
            resp_typ := c_msg_type_indet ||
            /* The input buffer may become invalid */
            a_buffer_in_state :: {a_buffer_in_state, e_buffer_in_msg_error}
        END
    END
    ;

    valid_req_header, valid_req, sc, buffer_out <-- server_receive_session_treatment_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        channel : dom(a_endpoint) &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_session_treatment_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        a_buffer_in_state :: {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} ||
        CHOICE
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet) ||
            sc, valid_req_header, valid_req
            :(sc : t_StatusCode_i & valid_req : BOOL & valid_req_header : BOOL &
              (valid_req = TRUE => sc :{e_sc_bad_request_too_large, e_sc_bad_response_too_large, e_sc_bad_out_of_memory}))
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            sc := e_sc_ok ||
            valid_req_header := TRUE ||
            valid_req := TRUE
        END
    END
    ;

    valid_resp <-- client_receive_session_treatment_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        channel /: dom(a_endpoint) &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_session_treatment_resps \/ {e_msg_service_fault_resp} &
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
        a_buffer_in_state, valid_resp
        :(a_buffer_in_state : t_buffer_in_state_i & valid_resp : BOOL &
          a_buffer_in_state : {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} &
          valid_resp =  bool(a_buffer_in_state = e_buffer_in_msg_read))
    END
    ;

    valid_req_header, valid_req, async_resp, sc, buffer_out <-- server_receive_session_service_req (channel, req_typ, req_context, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        channel : dom(a_endpoint) &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_reqs &
        req_context : t_request_context_i &
        req_context : t_request_context &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer  &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        a_buffer_in_state :: {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} ||
        valid_req_header, valid_req, async_resp, sc, buffer_out, a_buffer_out_state :(
            valid_req_header : BOOL &
            valid_req : BOOL &
            async_resp : BOOL &
            sc : t_StatusCode_i &
            buffer_out : t_byte_buffer_i &
            a_buffer_out_state : t_buffer_out_state_i &
            (buffer_out = c_byte_buffer_indet
             =>
             a_buffer_out_state = c_buffer_out_state_indet) &
            (buffer_out /= c_byte_buffer_indet
             =>
             buffer_out : t_byte_buffer &
             a_buffer_out_state = e_buffer_out_msg_written) &
            (valid_req = TRUE & async_resp = FALSE => sc : t_StatusCode) &
            (valid_req = TRUE & async_resp = FALSE & buffer_out = c_byte_buffer_indet => sc /= e_sc_ok)
        )
    END
    ;

    valid_resp <-- client_receive_session_service_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        channel /: dom(a_endpoint) &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_service_on_session_resps \/ {e_msg_service_fault_resp} &
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
        a_buffer_in_state, valid_resp
        :(a_buffer_in_state : t_buffer_in_state_i & valid_resp : BOOL &
          a_buffer_in_state : {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} &
          valid_resp =  bool(a_buffer_in_state = e_buffer_in_msg_read))
    END
    ;

    valid_req_header, valid_req, sc, buffer_out <-- server_receive_discovery_service_req (channel, req_typ, msg_buffer) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        channel : dom(a_endpoint) &
        req_typ : t_msg_type_i &
        req_typ = a_msg_in_type &
        a_msg_in_type : s_discovery_service_reqs &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        /* No message allocation before and remaining after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        a_buffer_in_state :: {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} ||
        CHOICE
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet) ||
            sc, valid_req_header, valid_req
            :(sc : t_StatusCode_i & valid_req : BOOL & valid_req_header : BOOL &
              (valid_req = TRUE => sc :{e_sc_bad_request_too_large, e_sc_bad_response_too_large, e_sc_bad_out_of_memory}))
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            sc := e_sc_ok ||
            valid_req_header := TRUE ||
            valid_req := TRUE
        END
    END
    ;

    valid_resp <-- client_receive_discovery_service_resp (channel, resp_typ, msg_buffer) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        channel /: dom(a_endpoint) &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_discovery_service_resps \/ {e_msg_service_fault_resp} &
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
        a_buffer_in_state, valid_resp
        :(a_buffer_in_state : t_buffer_in_state_i & valid_resp : BOOL &
          a_buffer_in_state : {e_buffer_in_msg_header_read, e_buffer_in_msg_read, e_buffer_in_msg_error} &
          valid_resp =  bool(a_buffer_in_state = e_buffer_in_msg_read))
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
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        app_context : t_application_context_i &
        /* The input message must have been blessed */
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_reqs \/ s_discovery_service_reqs &
        c_msg_in_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
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
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        CHOICE
            buffer_out := c_byte_buffer_indet ||
            req_handle :: t_client_request_handle_i
        OR
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    /* Only to change user on an active channel */
    ret, channel, buffer_out, req_handle <-- client_service_activate_session (session, p_user_token) =
    PRE
        session : t_session_i &
        session : t_session &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        CHOICE
            ret :: {e_sc_bad_out_of_memory,
                e_sc_bad_invalid_argument,
                e_sc_bad_invalid_state,          /* session not in a valid state to activate a user token on it */
                e_sc_bad_unexpected_error,
                e_sc_bad_identity_token_invalid, /* user token identity invalid */
                e_sc_bad_encoding_error,
                e_sc_bad_secure_channel_closed,
                e_sc_bad_connection_closed,
                e_sc_bad_encoding_error} ||
            channel :(channel : t_channel_i & channel = c_channel_indet) ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet) ||
            req_handle :: t_client_request_handle_i
        OR
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
            a_buffer_out_state := e_buffer_out_msg_written ||
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
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        CHOICE
            buffer_out := c_byte_buffer_indet ||
            req_handle :: t_client_request_handle_i
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
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet &
        a_buffer_out_state = c_buffer_out_state_indet
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
            req_handle :: t_client_request_handle_i
        OR
            ret        := e_sc_ok ||
            channel    :(channel : t_channel_i & channel : s_channel_connected) ||
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    ret, channel, buffer_out, req_handle <-- client_service_request (session, req_typ, req_msg, app_context) =
    PRE
        session         : t_session_i &
        session         : t_session &
        req_typ         : t_msg_type_i &
        req_msg         : t_msg_i &
        app_context     : t_application_context_i &
        /* The message must have been blessed */
        c_msg_out        = req_msg &
        c_msg_out        : t_msg &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type   = req_typ &
        a_msg_out_type   : t_msg_type &
        a_buffer_out_state = c_buffer_out_state_indet
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
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            req_handle :(req_handle : t_client_request_handle_i & req_handle : t_client_request_handle)
        END
    END
    ;

    ret, buffer_out, req_handle <-- client_discovery_service_request (channel, req_typ, req_msg, app_context) =
    PRE
        channel     : t_channel_i &
        channel     : t_channel &
        req_typ     : t_msg_type_i &
        req_msg     : t_msg_i &
        app_context : t_application_context_i &
        /* The message must have been blessed */
        c_msg_out        = req_msg &
        c_msg_out        : t_msg &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type   = req_typ &
        a_msg_out_type   : t_msg_type &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        CHOICE
            ret :: {e_sc_bad_out_of_memory,
                e_sc_bad_invalid_argument, /* message has not expected type */
                e_sc_bad_encoding_error} ||
            buffer_out := c_byte_buffer_indet ||
            req_handle := c_client_request_handle_indet
        OR
            ret        := e_sc_ok ||
            a_buffer_out_state := e_buffer_out_msg_written ||
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
        session : t_session_i
    THEN
        skip
    END
    ;

    server_evaluate_all_session_user_cert =
    BEGIN
        skip
    END
    ;

    bres, sc, buffer_out, channel <-- server_send_publish_response (session, req_handle, statusCode, resp_typ, publish_resp_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        req_handle : t_server_request_handle_i &
        statusCode : t_StatusCode_i &
        statusCode : t_StatusCode &
        resp_typ : t_msg_type_i &
        publish_resp_msg : t_msg_i &
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        /* The message must have been blessed */
        c_msg_out = publish_resp_msg &
        c_msg_out : t_msg &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = e_msg_subscription_publish_resp &
        a_buffer_out_state = c_buffer_out_state_indet
    THEN
        CHOICE
            bres := TRUE ||
            a_buffer_out_state := e_buffer_out_msg_written ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out : t_byte_buffer) ||
            channel :(channel : t_channel_i & channel : t_channel) ||
            sc := e_sc_ok
        OR
            bres := FALSE ||
            buffer_out :(buffer_out : t_byte_buffer_i & buffer_out = c_byte_buffer_indet) ||
            channel :(channel : t_channel_i) ||
            sc :(sc : t_StatusCode_i & sc :{e_sc_bad_session_id_invalid, e_sc_bad_request_too_large, e_sc_bad_response_too_large, e_sc_bad_out_of_memory})
        END
    END
    ;

    /* PROMOTES */

    bres <-- client_async_discovery_request_without_channel (channel_config_idx, req_typ, req_msg, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        req_typ            : t_msg_type_i &
        req_typ            : t_msg_type &
        req_msg            : t_msg_i &
        req_msg            : t_msg &
        app_context        : t_application_context_i
    THEN
        bres :: BOOL
    END
    ;

    /* Generates an event to send the discovery request */
    client_channel_connected_event_discovery (channel_config_idx, reverse_endpoint_config_idx, channel) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        reverse_endpoint_config_idx : t_reverse_endpoint_config_idx_i &
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

    /* Sends asynchronously the buffer on the secure channel */
    send_channel_msg_buffer (channel, buffer, request_context) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected &
        buffer          : t_byte_buffer_i &
        buffer          : t_byte_buffer &
        request_context : t_request_context_i &
        a_buffer_out_state = e_buffer_out_msg_written
    THEN
        /* The output buffer is no longer available. */
        a_buffer_out_state := c_buffer_out_state_indet
    END
    ;

    /* Sends asynchronously an error message on the secure channel */
    send_channel_error_msg (channel, status_code, request_context) =
    PRE
        channel         : t_channel_i &
        channel         : s_channel_connected &
        status_code     : t_StatusCode_i &
        status_code     : t_StatusCode &
        request_context : t_request_context_i &
        status_code /= e_sc_ok
    THEN
        skip
    END
    ;

    /* New session: creation and activation async */
    bres <-- client_async_activate_new_session_without_channel (channel_config_idx, p_user_token, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        app_context : t_session_application_context_i &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;

    /* New session: creation sync and activation async */
    bres <-- client_async_activate_new_session_with_channel (channel_config_idx, channel, p_user_token, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        channel : t_channel_i &
        channel : s_channel_connected &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        app_context : t_session_application_context_i &
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

    bres, msg_typ <-- decode_msg_type (msg_buffer) =
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
            bres := TRUE ||
            a_msg_in_type, msg_typ
            :(a_msg_in_type : t_msg_type_i & a_msg_in_type : t_msg_type &
              msg_typ = a_msg_in_type)
        OR
            a_buffer_in_state := e_buffer_in_msg_error ||
            bres := FALSE ||
            msg_typ := c_msg_type_indet
        END
    END
    ;

    dealloc_msg_in (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : t_msg_type
    THEN
        c_msg_in := c_msg_indet
    END
    ;

    dealloc_msg_in_buffer (msg_buffer) =
    PRE
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer
    THEN
        a_buffer_in_state := c_buffer_in_state_indet ||
        a_msg_in_type := c_msg_type_indet
    END
    ;

    msg_typ <-- bless_msg_in (msg) =
    PRE
        msg : t_msg_i &
        c_msg_in = c_msg_indet &
        a_msg_in_type = c_msg_type_indet
    THEN
        CHOICE
            c_msg_in :(c_msg_in : t_msg_i & c_msg_in : t_msg & c_msg_in = msg) ||
            msg_typ, a_msg_in_type :(msg_typ : t_msg_type_i &
                                     msg_typ = a_msg_in_type &
                                     a_msg_in_type : t_msg_type)
        OR
            msg_typ := c_msg_type_indet
        END
    END
    ;

    msg_typ <-- bless_msg_out (msg) =
    PRE
        msg : t_msg_i &
        c_msg_out = c_msg_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        CHOICE
            c_msg_out :(c_msg_out : t_msg_i & c_msg_out : t_msg & c_msg_out = msg) ||
            msg_typ, a_msg_out_type :(msg_typ : t_msg_type_i &
                                      msg_typ : t_msg_type &
                                      a_msg_out_type = msg_typ)
        OR
            msg_typ := c_msg_type_indet
        END
    END
    ;

    forget_msg_out (msg) =
    PRE
        msg : t_msg_i &
        c_msg_out = msg &
        c_msg_out : t_msg &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type : t_msg_type
    THEN
        c_msg_out := c_msg_indet ||
        a_msg_out_type := c_msg_type_indet
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

    client_close_session (session, sc_reason) =
    PRE
        session : t_session_i &
        sc_reason : t_StatusCode_i &
        sc_reason : t_StatusCode
    THEN
        skip
    END
    ;

    bres <-- internal_server_inactive_session_prio_event (p_session, p_newSessionState) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_newSessionState : t_sessionState_i
    THEN
        bres :: BOOL
    END
    ;

    /* Promotions */

    p_user <-- get_session_user_server (session) =
    PRE
        session : t_session_i &
        session : t_session
    THEN
        p_user :( p_user : t_user_i &
                  p_user : t_user )
    END
    ;

    p_user <-- get_local_user (endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user :( p_user : t_user_i &
                  p_user : t_user )
    END
    ;

    /* SUBSCRIPTION PROMOTED */

    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : t_subscription)
    END
    ;

    bres <-- server_subscription_event_triggered (p_notifierId, p_event, p_session, p_sub_id, p_mi_id) =
    PRE
        p_notifierId : t_NodeId_i &
        p_notifierId : t_NodeId &
        p_event : t_Event_i &
        p_event : t_Event &
        p_session : t_session_i &
        p_sub_id : t_subscription_i &
        p_mi_id : t_monitoredItemId_i
    THEN
        bres :: BOOL
    END
    ;

    server_subscription_data_changed (p_old_write_value_pointer, p_new_write_value_pointer) =
    PRE
        p_old_write_value_pointer : t_WriteValuePointer_i &
        p_old_write_value_pointer : t_WriteValuePointer &
        p_new_write_value_pointer : t_WriteValuePointer_i &
        p_new_write_value_pointer : t_WriteValuePointer
    THEN
        skip
    END
    ;

    server_subscription_node_changed (p_node_added, p_nid) =
    PRE
        p_node_added : BOOL & /* TRUE => added , FALSE => deleted */
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        skip
    END
    ;

    server_subscription_publish_timeout (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        skip
    END
    ;


    p_has_channel_to_close, p_channel_to_close <-- find_channel_to_close =
    BEGIN
        p_has_channel_to_close,
        p_channel_to_close
        :(p_has_channel_to_close : BOOL &
            p_channel_to_close : t_channel_i &
            (p_has_channel_to_close = TRUE => p_channel_to_close : s_channel_connected) &
            (p_has_channel_to_close = FALSE => p_channel_to_close = c_channel_indet)
        )
    END
    ;

    service_mgr_UNINITIALISATION =
    BEGIN
        skip
    END

END
