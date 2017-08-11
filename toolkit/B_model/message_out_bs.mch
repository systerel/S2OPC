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
    message_out_bs
SEES
    constants,
    message_in_bs
    
DEFINITIONS
    d_variables ==
    a_buffer_out_state,
    c_msg_out_header,
    c_msg_out,
    a_msg_out_type;
    
    d_typing == 
    a_buffer_out_state : t_buffer_out_state_i &
    c_msg_out_header : t_msg_header_i &
    c_msg_out : t_msg_i &
    a_msg_out_type : t_msg_type_i
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typing
    
INITIALISATION
    d_variables :( d_typing )
    
OPERATIONS
    
    nmsg_header <-- alloc_app_req_msg_header (msg_type, req_msg) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        req_msg : t_msg_i &
        req_msg : t_msg &
        c_msg_out_header = c_msg_header_indet
    THEN
        CHOICE
            nmsg_header, c_msg_out_header 
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header)
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header)
        END
    END
    ;
    
    /* Accept a message that was allocated by another part of the program */
    bless_msg_out (msg_type, msg_header, msg) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        msg_header : t_msg_header_i &
        msg_header : t_msg_header &
        msg      : t_msg_i &
        msg      : t_msg
    THEN
        c_msg_out_header := msg_header ||
        c_msg_out := msg ||
        a_msg_out_type := msg_type
    END
    ;
        
    nmsg_header, nmsg <-- alloc_req_msg (msg_type) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        c_msg_out_header = c_msg_header_indet &
        c_msg_out = c_msg_indet
    THEN
        CHOICE
            nmsg_header, c_msg_out_header 
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header) || 
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header) ||
            nmsg :( nmsg = c_msg_indet & nmsg = c_msg_out)
        END
    END
    ;
    
    nmsg_header, nmsg <-- alloc_resp_msg (msg_type, req_msg_ctx) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type & 
        c_msg_out_header = c_msg_header_indet &
        c_msg_out = c_msg_indet &
        req_msg_ctx : t_msg_i &
        req_msg_ctx = c_msg_in &
        c_msg_in : t_msg
    THEN
        CHOICE
            nmsg_header, c_msg_out_header 
            :( nmsg_header : t_msg_header_i & nmsg_header = c_msg_out_header & c_msg_out_header : t_msg_header) ||
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg_header :( nmsg_header = c_msg_header_indet & nmsg_header = c_msg_out_header) ||
            nmsg :( nmsg = c_msg_indet & nmsg = c_msg_out)
        END
    END
    ;
    
    dealloc_msg_header_out (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header
    THEN
        c_msg_out_header := c_msg_header_indet
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
    
    buffer <-- encode_msg (msg_type, msg_header, msg) = 
    PRE
        a_buffer_out_state = c_buffer_out_state_indet &
        msg_type : t_msg_type_i &
        msg_type = a_msg_out_type &
        a_msg_out_type : t_msg_type &
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        CHOICE
            buffer :(buffer : t_byte_buffer_i & buffer = c_byte_buffer_indet)
        OR
            buffer :(buffer : t_byte_buffer_i & buffer : t_byte_buffer) ||
            a_buffer_out_state := e_buffer_out_msg_written
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
        
    bres <-- is_valid_msg_out (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg = c_msg_out & c_msg_out : t_msg)
    END
    ;
    
    bres <-- is_valid_msg_out_header (msg_header) =
    PRE
        msg_header : t_msg_header_i
    THEN
        bres := bool(msg_header = c_msg_out_header & c_msg_out_header : t_msg_header)
    END
    ;
    
    msgtype <-- get_msg_out_type (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        msgtype := a_msg_out_type
    END
    ;
    
    write_msg_out_header_session_token (msg_header, session_token) = 
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        session_token : t_session_token_i &
        session_token : t_session_token &
        a_msg_out_type : 
        {e_msg_session_activate_req, 
            e_msg_session_close_req} \/ s_service_on_session_reqs
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;
    
    write_msg_out_header_req_handle (msg_header, req_handle) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        req_handle : t_request_handle_i
    THEN
        /* TODO: add relation on request handle ? => need to retrieve after write ? */
        /* TODO ?: check request handle is a valid handle ? => on server side no need / on client it is preferable ? */
        skip
    END
    ;
    
    write_msg_resp_header_service_status (msg_header, status_code) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_out_header &
        c_msg_out_header : t_msg_header &
        a_msg_out_type : s_service_resps &
        status_code : t_StatusCode_i &
        status_code : t_StatusCode
    THEN
        /* TODO: add relation on status ? => need to retrieve after write ? */
        skip
    END
    ;
    
    
    /* SESSION MESSAGES */
    write_create_session_req_msg_endpointUrl (msg, channel_config_idx) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;
    
    write_create_session_msg_session_token (msg, session_token) = 
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        session_token : t_session_token_i &
        session_token : t_session_token &
        a_msg_out_type = e_msg_session_create_resp
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;
    
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
    
    write_activate_msg_user (msg, user) =
    PRE
	    msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        user : t_user_i &
        user : t_user &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        /* TODO: add relation on user ? => need to retrieve after write ? */
        skip
    END
    
END
