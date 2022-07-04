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
    session_mgr

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    channel_mgr,
    request_handle_bs

OPERATIONS

    bres, channel <-- getall_valid_session_channel (session) =
    PRE
        session  : t_session_i
    THEN
        CHOICE
            bres := FALSE ||
            channel := c_channel_indet
        OR
            bres := TRUE ||
            channel :(channel : t_channel_i & channel : t_channel)
        END
    END
    ;

    session <-- client_receive_session_resp (channel, req_handle, resp_typ, resp_header, resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : s_session_treatment_resps &
        resp_header : t_msg_header_i &
        resp_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        resp_msg : t_msg_i &
        resp_msg : t_msg &
        resp_msg = c_msg_in
    THEN
        session :: t_session_i
    END
    ;

    session, service_ret <-- server_receive_session_req (channel, session_token, req_msg, req_typ, resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        session_token : t_session_token_i &
        req_msg : t_msg_i &
        req_msg : t_msg &
        req_msg = c_msg_in &
        req_typ : t_msg_type_i &
        req_typ : t_msg_type &
        req_typ = a_msg_in_type &
        a_msg_in_type : {e_msg_session_create_req,
            e_msg_session_activate_req,
            e_msg_session_close_req} &
        resp_msg : t_msg_i &
        resp_msg : t_msg &
        resp_msg = c_msg_out &
        a_msg_out_type : {e_msg_session_create_resp,
            e_msg_session_activate_resp,
            e_msg_session_close_resp}
    THEN
        service_ret :: t_StatusCode_i ||
        session :: t_session_i
    END
    ;

    ret, channel, session_token <-- client_validate_session_service_req (session, req_handle) =
    PRE
        session : t_session_i &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        a_msg_out_type : s_service_on_session_reqs
    THEN
        CHOICE
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : t_channel) ||
            session_token :(session_token : t_session_token_i & session_token : t_session_token)
        OR
            ret :: {e_sc_bad_invalid_state,
                e_sc_bad_invalid_argument,
                e_sc_bad_out_of_memory,
                e_sc_bad_unexpected_error} ||
            channel := c_channel_indet     ||
            session_token := c_session_token_indet
        END
    END
    ;

    bres, session <-- client_validate_session_service_resp (channel, req_handle) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_resps
    THEN
        CHOICE
            bres := TRUE ||
            session :( session : t_session_i & session : t_session)
        OR
            bres := FALSE ||
            session := c_session_indet
        END
    END
    ;

    is_valid_res, session, status_code_err <-- server_validate_session_service_req (channel, session_token) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        session_token : t_session_token_i &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_reqs
    THEN
        CHOICE
            is_valid_res := TRUE ||
            session :( session : t_session_i & session : t_session) ||
            status_code_err := e_sc_ok
        OR
            is_valid_res := FALSE ||
            status_code_err :: {e_sc_bad_session_id_invalid, e_sc_bad_session_not_activated,
                                e_sc_bad_invalid_state, e_sc_bad_secure_channel_id_invalid} ||
            session := c_session_indet
        END
    END
    ;

    is_valid_res, status_code_err, channel <-- server_validate_session_service_resp (session) =
    PRE
        session  : t_session_i &
        /* Remove constraint due to asynchronous response */
        /* c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_reqs & */
        c_msg_out : t_msg  &
        a_msg_out_type : s_service_on_session_resps
    THEN
        CHOICE
            is_valid_res := TRUE ||
            status_code_err := e_sc_ok ||
            channel :(channel : t_channel_i & channel : t_channel)
        OR
            is_valid_res := FALSE ||
            status_code_err :: {e_sc_bad_session_id_invalid, e_sc_bad_invalid_state, e_sc_bad_secure_channel_id_invalid} ||
            channel :(channel : t_channel_i  & channel = c_channel_indet)
        END
    END
    ;

    bret <-- client_create_session_req (session, channel, req_handle, create_req_msg) =
    PRE
        session : t_session_i &
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        bret :: BOOL
    END
    ;

    /* New session: creation and activation async */
    bres <-- client_async_activate_new_session_without_channel (channel_config_idx, p_user_token, app_context) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        app_context : t_session_application_context_i
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

    ret, channel, session_token <-- client_user_activate_session_req (session, req_handle, p_user_token, activate_req_msg) =
    PRE
        session : t_session_i & // Here session is provided by applicative but still can point to invalid session (ref on closed session)
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        CHOICE
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
            session_token :(session_token : t_session_token_i & session_token : t_session_token)
        OR
            ret :: {e_sc_bad_identity_token_invalid,
                e_sc_bad_unexpected_error,
                e_sc_bad_out_of_memory,
                e_sc_bad_invalid_state,
                e_sc_bad_invalid_argument} ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
        END
    END
    ;

    ret, session_token <-- client_sc_activate_session_req (session, req_handle, channel, activate_req_msg) =
    PRE
        session : t_session_i &
        session : t_session & // provided by dispatch manager => not an invalid session
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        channel : t_channel_i &
        channel : s_channel_connected &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        ret,
        session_token
        :(  ret : t_StatusCode_i &
            ret : {e_sc_ok,
                   e_sc_bad_invalid_state,
                   e_sc_bad_invalid_argument,
                   e_sc_bad_identity_token_invalid,
                   e_sc_bad_unexpected_error,
                   e_sc_bad_out_of_memory} &
            session_token : t_session_token_i &
            (ret = e_sc_ok
             =>
             session_token : t_session_token)
        )
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

    ret, channel, session_token <-- client_close_session_req (session, req_handle, close_req_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        req_handle : t_client_request_handle_i &
        req_handle : s_client_request_handle &
        close_req_msg : t_msg_i &
        close_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_close_req
    THEN
        CHOICE
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
            session_token :(session_token : t_session_token_i & session_token : t_session_token)
        OR
            ret :: {e_sc_bad_invalid_state,
                e_sc_bad_invalid_argument,
                e_sc_bad_out_of_memory,
                e_sc_bad_unexpected_error} ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
        END
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

    server_evaluate_session_timeout (session) =
    PRE
        session : t_session_i
    THEN
        skip
    END
    ;

    server_close_session (session, sc_reason) =
    PRE
        session : t_session_i &
        sc_reason : t_StatusCode_i &
        sc_reason : t_StatusCode
    THEN
        skip
    END
    ;

    /*PROMOTES*/
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
        p_channel_config_idx : t_channel_config_idx_i
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

    p_localeIds <-- get_server_session_preferred_locales(p_session) =
    PRE
        p_session    : t_session_i &
        p_session    : t_session
    THEN
        p_localeIds :(p_localeIds : t_LocaleIds_i & p_localeIds : t_LocaleIds)
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

    endpoint_config_idx <-- session_get_endpoint_config(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        endpoint_config_idx :: t_endpoint_config_idx_i
    END

END
