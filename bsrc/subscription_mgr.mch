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
    subscription_mgr
    
SEES
    constants,
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
    
    StatusCode_service <-- treat_subscription_create_monitored_items_req (p_session, p_req_msg, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_publish_req &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp
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
