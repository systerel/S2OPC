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
    io_dispatch_mgr

SEES
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

    valid_msg <-- receive_msg_buffer (channel, buffer, request_context) =
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
        c_msg_in_header    :: t_msg_header_i ||
        /* End of receive execution lead to a buffer in invalid state (error) or a buffer fully read */
        a_buffer_in_state  :: {c_buffer_in_state_indet, e_buffer_in_msg_read, e_buffer_in_msg_type_read, a_buffer_in_state} ||
        a_buffer_out_state :: {c_buffer_out_state_indet, e_buffer_out_msg_written, a_buffer_out_state} ||
        a_msg_in_type      :: t_msg_type_i ||
        /* c_msg_in instance could be provided to app but should be not referenced anymore in c_msg_in */
        valid_msg :: BOOL
    END
    ;

    snd_msg_failure (channel, request_id, error_status) =
    PRE
        channel  : t_channel_i &
        channel  : t_channel &
        request_id : t_request_context_i &
        error_status : t_StatusCode_i
    THEN
        skip
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
    bres <-- client_activate_new_session (channel_config_idx, p_user_token, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        app_context : t_session_application_context_i &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        bres :: BOOL
    END
    ;

    /* Only to change user_token on an active channel */
    client_reactivate_session_new_user (session, p_user_token) =
    PRE
        session : t_session_i &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        a_buffer_out_state = c_buffer_out_state_indet &
        /* No message allocation before and remaining after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        skip
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

    ret <-- server_treat_local_service_request (endpoint_config_idx, req_msg, app_context) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        req_msg         : t_msg_i &
        app_context : t_application_context_i &
        /* in message (request) is unkown before call and deallocated after operation */
        c_msg_in = c_msg_indet &
        c_msg_in_header = c_msg_header_indet &
        a_msg_in_type = c_msg_type_indet &
        /* out message (response) is unkown before call and deallocated after operation */
        c_msg_out = c_msg_indet &
        c_msg_out_header = c_msg_header_indet &
        a_msg_out_type = c_msg_type_indet
    THEN
        ret :: {e_sc_ok,
                e_sc_bad_out_of_memory,
                e_sc_bad_invalid_argument}
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

    internal_server_evaluate_session_timeout (session) =
    PRE
        session : t_session_i
    THEN
        skip
    END
    ;

    client_request_timeout (channel, request_handle) =
    PRE
        channel : t_channel_i &
        channel : t_channel &
        request_handle : t_client_request_handle_i
    THEN
        skip
    END
    ;

    /* SUBSCRIPTIONS */

    bres <-- internal_server_data_changed (p_old_write_value_pointer, p_new_write_value_pointer) =
    PRE
        p_old_write_value_pointer : t_WriteValuePointer_i &
        p_new_write_value_pointer : t_WriteValuePointer_i
    THEN
        bres :: BOOL
    END;

    bres <-- internal_server_subscription_publish_timeout (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        bres :: BOOL
    END;

    /* Events leading to call this entrypoint shall be enqueued as prioritary event (next to be evaluated)
       => guarantee of performance on subscription
    */
    bres <-- internal_server_send_publish_response_prio_event (p_session, p_req_handle, p_req_context, p_publish_resp_msg, p_statusCode) =
    PRE
        p_session : t_session_i &
        p_req_handle : t_server_request_handle_i &
        p_req_context : t_request_context_i &
        p_publish_resp_msg : t_msg_i &
        p_statusCode : t_StatusCode_i
    THEN
        bres :: BOOL
    END
    ;

    /* Events leading to call this entrypoint shall be enqueued as prioritary event (next to be evaluated)
       => guarantee of state synchronisation between subscription <=> session */
    bres <-- internal_server_inactive_session_prio_event (p_session, p_newSessionState) =
    PRE
        p_session : t_session_i &
        p_session : t_session & /* shall be considered in t_session since is_valid_session will fail if not in s_session */
        p_newSessionState : t_sessionState
    THEN
        bres :: BOOL
    END
    ;

    /* CLEAR OPERATIONS */

    // TODO: add close all sessions / subscriptions

    bres <-- close_all_active_connections (p_clientOnly) =
    PRE
        p_clientOnly : BOOL
    THEN
        bres :: BOOL // indicates if active connection will be closed
    END
    ;

    UNINITIALISATION =
    BEGIN
        skip
    END

END
