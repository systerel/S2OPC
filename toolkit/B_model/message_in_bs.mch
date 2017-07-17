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
    message_in_bs
SEES
    constants
    
DEFINITIONS
    d_variables ==
    c_msg_in,
    a_msg_in_type;
    
    d_typing == 
    c_msg_in : t_msg_i &
    a_msg_in_type : t_msg_type
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typing
    
INITIALISATION
    d_variables :( d_typing )
    
OPERATIONS
    
    msg_in_memory_changed =
    BEGIN
        d_variables :( d_typing )
    END
    ;
    
    dealloc_msg_in (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg
    THEN
        c_msg_in := c_msg_indet ||
        a_msg_in_type :: t_msg_type
    END
    ;
    
    bres <-- is_valid_msg_in (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg = c_msg_in & c_msg_in : t_msg)
    END
    ;
    
    msgtype <-- get_msg_in_type (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg
    THEN
        msgtype := a_msg_in_type
    END
    ;
    
    session_token <-- read_msg_req_header_session_token (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : 
        {e_msg_session_create_req, /* should return an indet token here OR not be used */
         e_msg_session_activate_req,
            e_msg_session_close_req,
            e_msg_session_read_req}
    THEN
        session_token :: t_session_token_i
    END
    ;
    
    handle <-- read_msg_header_req_handle (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg
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
    
    user <-- read_activate_msg_user (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_req
    THEN
        user :: t_user_i
    END
    ;
    
    status <-- read_msg_resp_header_service_status (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type :
        {e_msg_session_create_resp,
            e_msg_session_activate_resp, 
            e_msg_session_close_resp, 
            e_msg_session_read_resp}
    THEN
        status :( status : t_StatusCode_i & status : t_StatusCode)
    END
    ;
    
    payload <-- get_msg_payload (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type :
        {/*e_msg_session_create_req,
            e_msg_session_activate_req, 
            e_msg_session_close_req, 
            e_msg_session_read_req,*/
        	e_msg_session_write_req}
    THEN
        payload :( payload : t_ByteString_i & payload : t_ByteString)
    END
    
END
