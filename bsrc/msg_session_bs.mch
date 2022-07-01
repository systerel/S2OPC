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
    msg_session_bs
SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

OPERATIONS

    write_create_session_req_msg_sessionTimeout (create_req_msg) =
    PRE
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        skip
    END
    ;

    write_create_session_req_msg_serverUri (msg, channel_config_idx) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    write_create_session_req_msg_endpointUrl (msg, channel_config_idx) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    write_create_session_msg_session_token (msg, session, session_token) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        session : t_session_i &
        session : t_session &
        session_token : t_session_token_i &
        session_token : t_session_token &
        a_msg_out_type = e_msg_session_create_resp
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;

    /* ret is status code of get_endpoints-ish service for create session response */
    ret <-- write_create_session_msg_server_endpoints (req_msg, resp_msg, endpoint_config_idx) =
    PRE
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_session_create_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_resp &
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx
    THEN
        ret :(ret : t_StatusCode_i & ret : t_StatusCode)
    END
    ;

    write_create_session_msg_session_revised_timeout (req_msg, resp_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_session_create_req &
        resp_msg : t_msg_i &
        resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_resp
    THEN
        skip
    END
    ;

    /* Fills the Signature. Do not call when SC:Sec_pol is None */
    bret <-- write_create_session_resp_signature(p_msg, p_signature) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_out &
        a_msg_out_type = e_msg_session_create_resp &
        p_signature : t_SignatureData_i
    THEN
        bret :: BOOL
    END
    ;

    /* Fills the Certificate. Do not call when (SC:Sec_pol is None with UserToken:Sec_pol being all None) */
    bret <-- write_create_session_resp_cert(p_msg, p_channel_config_idx) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_out &
        a_msg_out_type = e_msg_session_create_resp &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        bret :: BOOL
    END
    ;

    /* Fills the server Nonce */
    write_create_session_resp_nonce(p_msg, p_nonce) =
    PRE
        p_msg : t_msg_i &
        p_msg = c_msg_out &
        a_msg_out_type = e_msg_session_create_resp &
        p_nonce : t_Nonce_i &
        p_nonce : t_Nonce
    THEN
        skip
    END
    ;

    bret <-- write_create_session_req_msg_crypto(p_req_msg, p_channel_config_idx, p_nonce) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_nonce : t_Nonce_i &
        p_nonce : t_Nonce
    THEN
        bret :: BOOL
    END
    ;

    write_activate_req_msg_locales(p_req_msg, p_channel_config_idx) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    bret <-- write_activate_session_req_msg_crypto(activate_req_msg, signature) =
    PRE
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req &
        signature : t_SignatureData_i &
        signature : t_SignatureData
    THEN
        bret :: BOOL
    END
    ;


    write_activate_session_resp_nonce(activate_resp_msg, nonce) =
    PRE
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_resp &
        nonce : t_Nonce_i &
        nonce : t_Nonce
    THEN
        skip
    END
    ;

    write_activate_msg_user (msg, p_user_token) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        /* TODO: add relation on p_user_token ? => need to retrieve after write ? */
        skip
    END
    ;

    write_create_session_req_msg_clientDescription (p_req_msg, p_channel_config_idx) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_out &
        c_msg_out : t_msg &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        a_msg_out_type = e_msg_session_create_req
    THEN
        skip
    END
    ;

    write_create_session_req_msg_sessionName (p_req_msg, p_app_context) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_out &
        c_msg_out : t_msg &
        p_app_context : t_session_application_context_i &
        a_msg_out_type = e_msg_session_create_req
    THEN
        skip
    END
    ;

    write_create_session_req_msg_maxResponseMessageSize (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        skip
    END;

    write_create_session_resp_msg_maxRequestMessageSize (p_resp_msg) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_resp
    THEN
        skip
    END
    ;

    valid <-- create_session_req_check_client_certificate (p_req_msg, p_channel_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_req
    THEN
        valid :: BOOL
    END
    ;

    valid <-- create_session_resp_check_server_certificate (p_resp_msg, p_channel_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        valid :: BOOL
    END
    ;

    valid <-- create_session_resp_check_server_endpoints (p_resp_msg, p_channel_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        valid :: BOOL
    END;

    create_session_req_export_maxResponseMessageSize (p_channel_config_idx, p_req_msg) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_req
    THEN
        skip
    END
    ;

    create_session_resp_export_maxRequestMessageSize (p_channel_config_idx, p_resp_msg) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        skip
    END

END
