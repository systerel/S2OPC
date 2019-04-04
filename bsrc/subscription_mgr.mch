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
    subscription_mgr

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    request_handle_bs

OPERATIONS

    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : t_subscription)
    END
    ;

    StatusCode_service <-- treat_create_subscription_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_create_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_create_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    StatusCode_service <-- treat_modify_subscription_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_modify_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_modify_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    StatusCode_service <-- treat_delete_subscriptions_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_delete_subscriptions_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_delete_subscriptions_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    StatusCode_service <-- treat_publishing_mode_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_set_publishing_mode_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_set_publishing_mode_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
    END
    ;

    StatusCode_service, async_resp_msg <-- treat_subscription_publish_request
      (p_session, p_req_header, p_req_msg, p_req_handle, p_req_ctx, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_header : t_msg_header_i &
        p_req_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_publish_req &
        p_req_handle : t_server_request_handle_i &
        p_req_ctx : t_request_context_i &
        p_req_ctx : t_request_context &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp
    THEN
        CHOICE
            /* if service failed, response is synchronous */
            StatusCode_service,
            async_resp_msg
            :(  StatusCode_service : t_StatusCode_i &
                StatusCode_service : t_StatusCode &
                StatusCode_service /= e_sc_ok &
                async_resp_msg = FALSE
            )
        OR
            /* if service succeeded, response is synchronous or asynchronous */
            StatusCode_service,
            async_resp_msg
            :(  StatusCode_service : t_StatusCode_i &
                StatusCode_service = e_sc_ok &
                async_resp_msg : BOOL
            )
        END
    END
    ;

    StatusCode_service <-- treat_subscription_republish_request (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        c_msg_in_header : t_msg_header &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_republish_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_republish_resp
    THEN
        StatusCode_service
        :(StatusCode_service : t_StatusCode_i &
          StatusCode_service : {e_sc_ok, e_sc_bad_subscription_id_invalid, e_sc_bad_message_not_available, e_sc_bad_out_of_memory})
    END
    ;

    StatusCode_service <-- treat_subscription_create_monitored_items_req (p_session, p_user, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_user    : t_user_i  &
        p_user    : t_user    &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_create_resp
    THEN
        StatusCode_service
        :(  StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode
        )
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

    server_subscription_publish_timeout (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        /* Subscription transitions: could generate publish response sending events */
        skip
    END
    ;

    server_subscription_session_inactive (p_session, p_newSessionState) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_newSessionState : t_sessionState
    THEN
        skip
    END
    ;

    subscription_mgr_UNINITIALISATION =
    BEGIN
        skip
    END

END
