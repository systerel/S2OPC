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
    c_msg_out,
    a_msg_out_type;
    
    d_typing == 
    c_msg_out : t_msg_i &
    a_msg_out_type : t_msg_type
    
ABSTRACT_VARIABLES
    d_variables 
    
INVARIANT
    d_typing
    
INITIALISATION
    d_variables :( d_typing )
    
OPERATIONS
    
    msg_out_memory_changed =
    BEGIN
        d_variables :( d_typing )
    END
    ;
    
    /* Accept a message that was allocated by another part of the program */
    bless_msg_out (msg, msg_type) =
    PRE
        msg      : t_msg_i &
        msg      : t_msg &
        msg_type : t_msg_type
    THEN
        c_msg_out := msg ||
        a_msg_out_type := msg_type
    END
    ;

    nmsg <-- alloc_req_msg (msg_type) =
    PRE
        msg_type : t_msg_type
    THEN
        CHOICE
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg := c_msg_indet
        END
    END
    ;
    
    nmsg <-- alloc_resp_msg (msg_type, req_msg_ctx) =
    PRE
        msg_type : t_msg_type &
        req_msg_ctx : t_msg_i &
        req_msg_ctx = c_msg_in &
        c_msg_in : t_msg
    THEN
        CHOICE
            nmsg, c_msg_out :( nmsg : t_msg_i & nmsg = c_msg_out & c_msg_out : t_msg ) ||
            a_msg_out_type := msg_type
        OR
            nmsg := c_msg_indet
        END
    END
    ;
    
    /* TODO: rename ? does not free message anymore, managed by SC. 
       PB: nothing done here except in model ! => msg shall be set to NULL after this operation
    */
    dealloc_msg_out (msg) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg
    THEN
        c_msg_out := c_msg_indet ||
        a_msg_out_type :: t_msg_type
    END
    ;
    
    bres <-- is_valid_msg_out (msg) =
    PRE
        msg : t_msg_i
    THEN
        bres := bool(msg = c_msg_out & c_msg_out : t_msg)
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
    
    write_msg_out_header_session_token (msg, session_token) = 
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        session_token : t_session_token_i &
        session_token : t_session_token &
        a_msg_out_type : 
        {e_msg_session_activate_req, 
            e_msg_session_close_req} \/ s_service_reqs
    THEN
        /* TODO: add relation on session token ? => need to retrieve after write ? */
        skip
    END
    ;
    
    write_msg_out_header_req_handle (msg, req_handle) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        c_msg_out : t_msg &
        req_handle : t_request_handle_i
    THEN
        /* TODO: add relation on request handle ? => need to retrieve after write ? */
        /* TODO ?: check request handle is a valid handle ? => on server side no need / on client it is preferable ? */
        skip
    END
    ;
    
    write_msg_resp_header_service_status (msg, status_code) =
    PRE
        msg : t_msg_i &
        msg = c_msg_out &
        a_msg_out_type : 
        {e_msg_session_create_resp,
            e_msg_session_activate_resp, 
            e_msg_session_close_resp, 
            e_msg_session_read_resp} &
        status_code : t_StatusCode_i &
        status_code : t_StatusCode
    THEN
        /* TODO: add relation on status ? => need to retrieve after write ? */
        skip
    END
    ;
    
    
    /* SESSION MESSAGES */
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
