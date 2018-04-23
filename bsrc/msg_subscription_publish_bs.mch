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
    msg_subscription_publish_bs
    
SEES
    constants,
    message_in_bs,
    message_out_bs,
    request_handle_bs
    
OPERATIONS
    
    /* TODO: move into message_in_bs: + need type for header ? */
    req_expiration_time <-- get_msg_header_expiration_time (p_req_header) =
    PRE
        p_req_header : t_msg_header_i &
        p_req_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        a_msg_in_type = e_msg_subscription_publish_req
    THEN
        req_expiration_time :(req_expiration_time : t_timeref_i & req_expiration_time : t_timeref)
    END
    ;
    
    set_msg_publish_resp_subscription (p_resp_msg, p_subscription) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        skip
    END
    ;
    
    set_msg_publish_resp_notificationMsg (p_resp_msg, p_notifMsg) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_notifMsg : t_notif_msg_i &
        p_notifMsg : t_notif_msg
    THEN
        skip
    END
    ;
    
    generate_internal_send_publish_response_event (p_session, p_publish_resp_msg, p_req_handle, p_req_context) =
    PRE
        p_session : t_session_i & p_session : t_session &
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg &
        p_req_handle : t_server_request_handle_i & 
        p_req_context : t_request_context_i & p_req_context : t_request_context
    THEN
        skip
    END
    
END
