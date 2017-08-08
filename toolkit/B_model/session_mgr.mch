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
    session_mgr
    
SEES
    constants,
    message_in_bs,
    message_out_bs,
    channel_mgr_bs,
    request_handle_bs
    
DEFINITIONS
   d_var_it ==
   	/* sessions to auto activate on session created state obtained */
   	s_session_async_activate_session, /* TODO: If users not anonymous, partial function to user config to use ! */
   	/* sessions to auto create on secure channel connection */
    s_session_async_create_session, /* TODO: Need to could activate several sessions (with same or different user), partial function to ? */
    /* iter */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned;
    
    d_typing_it ==
    s_session_async_activate_session <: t_session &
	s_session_async_create_session <: t_channel_config_idx &
	/* iter */
    av_sessions_orphaned <: t_session &
    av_sessions_to_iterate <: av_sessions_orphaned &
    av_sessions_iterated <: av_sessions_orphaned &
    av_sessions_to_iterate /\ av_sessions_iterated = {}
    
ABSTRACT_VARIABLES
    d_var_it
    
INVARIANT
    d_typing_it
    
INITIALISATION
    s_session_async_activate_session := {} ||
    s_session_async_create_session := {} ||
    /* it */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned :(av_sessions_orphaned <: t_session &
        av_sessions_to_iterate <: av_sessions_orphaned &
        av_sessions_iterated <: av_sessions_orphaned &
        av_sessions_to_iterate /\ av_sessions_iterated = {})

OPERATIONS
    
    session <-- client_receive_session_resp (channel, req_handle, resp_typ, resp_header, resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        resp_typ : t_msg_type_i &
        resp_typ = a_msg_in_type &
        a_msg_in_type : {e_msg_session_create_resp,
            e_msg_session_activate_resp,
            e_msg_session_close_resp} &
        resp_header : t_msg_header_i &
        resp_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        resp_msg : t_msg_i &
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
    
    ret, channel, session_token <-- client_validate_session_service_req (session, req_handle, req_msg) =
    PRE
        session : t_session_i &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        req_msg : t_msg_i &
        req_msg : t_msg &
        req_msg = c_msg_out &
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
            channel := c_channel_indet
        END
    END
    ;
    
    bres <-- client_validate_session_service_resp (channel, req_handle) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_resps
    THEN
        bres :: BOOL
    END
    ;
    
    is_valid_res, session, snd_err <-- server_validate_session_service_req (channel, req_handle, session_token) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        session_token : t_session_token_i &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_reqs
    THEN
        CHOICE
            is_valid_res := TRUE ||
            session :( session : t_session_i & session : t_session) ||
            snd_err := FALSE
        OR
            is_valid_res := FALSE ||
            snd_err := TRUE ||
            session := c_session_indet
        END
    END
    ;
    
    is_valid_res, snd_err <-- server_validate_session_service_resp (channel, session, req_handle) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session  : t_session_i &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        c_msg_in : t_msg &
        c_msg_out : t_msg  &
        a_msg_in_type : s_service_on_session_reqs &
        a_msg_out_type : s_service_on_session_resps
    THEN
        CHOICE
            is_valid_res := TRUE ||
            snd_err := FALSE
        OR
            is_valid_res := FALSE ||
            snd_err := TRUE
        END
    END
    ;
    
    ret <-- client_create_req (session, channel, req_handle, create_req_msg) = 
    PRE
        session : t_session_i &
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        ret :: {e_sc_bad_invalid_argument, e_sc_bad_invalid_state, e_sc_bad_out_of_memory, e_sc_ok}
    END
    ;
    
    ret, channel, session_token <-- client_user_activate_req (session, req_handle, user, activate_req_msg) =
    PRE
        session : t_session_i & // Here session is provided by applicative but still can point to invalid session (ref on closed session)
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        user : t_user_i &
        user : t_user &
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
    
    ret, session_token <-- client_sc_activate_req (session, req_handle, channel, activate_req_msg) =
    PRE
        session : t_session_i &
        session : t_session & // provided by dispatch manager => not an invalid session
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        channel : t_channel_i &
        channel : s_channel_connected &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        ret :: {e_sc_ok, e_sc_bad_invalid_state,
            e_sc_bad_invalid_argument,
            e_sc_bad_identity_token_invalid,
            e_sc_bad_unexpected_error,
            e_sc_bad_out_of_memory} ||
        session_token :: t_session_token_i
    END
    ;
    
    ret, channel, session_token <-- client_close_req (session, req_handle, close_req_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
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
    
    client_session_mgr_close_session (session) =
    PRE
        session : t_session_i
    THEN
        skip
    END
    ;
    
    /*PROMOTES*/
    nsession <-- client_init_session = 
    BEGIN
        nsession :: t_session_i
    END
    ;
    
    client_secure_channel_lost (lost_channel, channel_config_idx) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : s_channel_connected &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;
    
    server_secure_channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
    THEN
        skip
    END
    ;
    
    server_close_session (session) =
    PRE
        session : t_session_i
    THEN
        skip
    END
    ;
    
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :: BOOL
    END;
    
    continue <-- init_iter_orphaned_t_session (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        continue :: BOOL
    END
    ;
    
    session, continue <-- continue_iter_orphaned_t_session =
    PRE
        av_sessions_to_iterate /= {}
    THEN
        session :(session : t_session_i & session : t_session) ||
        continue :: BOOL
    END
    ;
    
    state <-- get_session_state_or_closed (session) =
    PRE
        session : t_session_i
    THEN
        state :: t_sessionState
    END
    ;
    
    user <-- get_session_user_or_indet (session) =
    PRE
        session : t_session_i
    THEN
        user :: t_user_i
    END
    ;

    delete_session (session) = 
    PRE
        session : t_session_i
    THEN
        skip
    END
    
END
