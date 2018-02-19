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
    message_in_bs
SEES
    constants
    
DEFINITIONS
    d_variables ==
    a_buffer_in_state,
    c_msg_in_header,
    c_msg_in,
    a_msg_in_type;
    
    d_typing == 
    a_buffer_in_state : t_buffer_in_state_i &
    c_msg_in_header : t_msg_header_i &
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type_i
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typing
    
INITIALISATION
    d_variables :( d_typing )
    
OPERATIONS

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

    msg_header <-- decode_msg_header (is_request, msg_buffer) =
    PRE
        is_request : BOOL &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        c_msg_in_header = c_msg_header_indet &
        c_msg_in = c_msg_indet &
        a_msg_in_type : t_msg_type_i &
        a_msg_in_type : t_msg_type
    THEN
        CHOICE
            a_buffer_in_state := e_buffer_in_msg_header_read ||
            c_msg_in_header :(c_msg_in_header : t_msg_header_i & c_msg_in_header : t_msg_header) ||
            msg_header := c_msg_in_header
        OR
            a_buffer_in_state := c_buffer_in_state_indet ||
            msg_header := c_msg_header_indet
        END
    END
    ;
    
    msg <-- decode_msg (msg_type, msg_buffer) =
    PRE
        msg_type : t_msg_type_i &
        msg_type : t_msg_type &
        msg_type = a_msg_in_type &
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer &
        a_buffer_in_state = e_buffer_in_msg_type_read &
        c_msg_in_header : t_msg_header_i &
        c_msg_in_header : t_msg_header &
        c_msg_in = c_msg_indet &
        a_msg_in_type : t_msg_type_i &
        a_msg_in_type : t_msg_type
    THEN
        CHOICE
            // TODO: free the msg buffer ! (except if lazy decoding linked to buffer)
            a_buffer_in_state := e_buffer_in_msg_read ||
            c_msg_in :(c_msg_in : t_msg_i & c_msg_in : t_msg) ||
            msg := c_msg_in
        OR
            a_buffer_in_state := c_buffer_in_state_indet ||
            msg := c_msg_indet
        END
    END
    ;
    
    dealloc_msg_in_header (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        c_msg_in_header : t_msg_header
    THEN
        c_msg_in_header := c_msg_header_indet
    END
    ;
    
    dealloc_msg_in (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : t_msg_type
    THEN
        c_msg_in := c_msg_indet ||
        a_msg_in_type := c_msg_type_indet
    END
    ;
    
    dealloc_msg_in_buffer (msg_buffer) =
    PRE
        msg_buffer : t_byte_buffer_i &
        msg_buffer : t_byte_buffer
    THEN
        a_buffer_in_state := c_buffer_in_state_indet
    END
    ;
    
    /* Accept a message that was allocated by another part of the program */
    bless_msg_in (msg) =
    PRE
        msg : t_msg_i &
        msg : t_msg
    THEN
        c_msg_in := msg ||
        a_msg_in_type :: t_msg_type
    END
    ;
    
    /* Consider response message as deallocated since responsibility was forwarded to application */
    forget_resp_msg_in (msg_header, msg) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        c_msg_in_header : t_msg_header &
        a_msg_in_type : s_service_resps
    THEN
        c_msg_in := c_msg_indet ||
        a_msg_in_type := c_msg_type_indet ||
        c_msg_in_header := c_msg_header_indet
    END
    ;
        
    bres <-- is_valid_msg_in_header (msg_header) =
    PRE
        msg_header : t_msg_header_i
    THEN
        bres := bool(msg_header = c_msg_in_header & c_msg_in_header : t_msg_header)
    END
    ;
    
    bres, msg_typ <-- is_valid_app_msg_in (msg) =
    PRE
        msg : t_msg_i
    THEN
        CHOICE
            bres := TRUE ||
            msg_typ :( msg_typ : t_msg_type_i & msg_typ : t_msg_type )
        OR
            bres := TRUE ||
            msg_typ := c_msg_type_indet
        END
    END
    ;
    
    bres <-- is_valid_msg_in (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg = c_msg_in & c_msg_in : t_msg)
    END
    ;
    
    bres <-- is_valid_msg_in_type (msg_typ) =
    PRE
        msg_typ : t_msg_type_i
    THEN
        bres := bool(msg_typ = a_msg_in_type & a_msg_in_type : t_msg_type)
    END
    ;
    
    bres <-- is_valid_request_context (req_context) =
    PRE
        req_context : t_request_context_i
    THEN
        bres := bool(req_context : t_request_context)
    END
    ;
    
    msgtype <-- get_msg_in_type (req_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg = c_msg_in &
        c_msg_in : t_msg
    THEN
        msgtype, a_msg_in_type :( msgtype : t_msg_type_i & msgtype = a_msg_in_type & a_msg_in_type : t_msg_type)
    END
    ;
    
    session_token <-- read_msg_req_header_session_token (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        a_msg_in_type : s_session_treatment_reqs \/ s_service_on_session_reqs
    THEN
        session_token :: t_session_token_i
    END
    ;
    
    handle <-- read_msg_header_req_handle (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        c_msg_in_header : t_msg_header
    THEN
        handle :: t_request_handle_i
    END
    ;
    
    session_token <-- read_create_session_msg_session_token (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        session_token :: t_session_token_i
    END
    ;
    
    valid_user_token, user <-- read_activate_req_msg_user (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_req
    THEN
        valid_user_token :: BOOL ||
        user :: t_user_i
    END
    ;
    
    status <-- read_msg_resp_header_service_status (msg_header) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        a_msg_in_type : s_service_resps
    THEN
        status :( status : t_StatusCode_i & status : t_StatusCode)
    END
    ;
    
    /* Operation to copy header content in response message 
       in order to be provided to client user application */
    copy_msg_resp_header_into_msg (msg_header, msg) =
    PRE
        msg_header : t_msg_header_i &
        msg_header = c_msg_in_header &
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        c_msg_in_header : t_msg_header &
        a_msg_in_type : s_service_resps
    THEN
        skip
    END
    
END
