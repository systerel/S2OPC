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
    session_core
SEES
    constants,
    message_in_bs,
    message_out_bs,
    channel_mgr_bs,
    request_handle_bs
    
DEFINITIONS
    d_variables ==
    s_session,
    a_state,
    a_channel,
    a_orphaned,
    a_token,
    a_user;
    
    d_var_it ==
    /* iter */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned;
    
    d_var_imp ==
    a_cli_pending_requests;
    
    d_typing ==    
    s_session <: t_session &    
    a_state : s_session --> t_sessionState & // a session shall be associated to a state    
    a_channel : s_session +-> t_channel & // a session may be associated to a valid channel    
    a_orphaned : s_session +-> t_channel & // a client session may be orphaned of a channel (precedently lost)    
    a_token : t_session_token >+> s_session & // a session has an authentication token once create response is produced / received    
    a_user : s_session +-> t_user;  //  a session may be associated to a valid user
    
    d_typing_it ==
    av_sessions_orphaned <: t_session &
    av_sessions_to_iterate <: av_sessions_orphaned &
    av_sessions_iterated <: av_sessions_orphaned &
    av_sessions_to_iterate /\ av_sessions_iterated = {};
    
    d_typing_imp == 
    a_cli_pending_requests : t_request_handle +-> s_session;
    
    d_close_session (session) ==
    a_channel := {session} <<| a_channel               || // If session had a channel, remove this relation
    a_orphaned := {session} <<| a_orphaned             || // If session was orphaned, remove this relation
    a_user := {session} <<| a_user                     || // If session had a user, remove this relation
    a_token := a_token |>> {session}                   || // If session had a token, remove this relation
    a_cli_pending_requests := a_cli_pending_requests |>> {session}  ||// If session had pending requests, remove those relations
    a_state(session) := e_session_closed // keep session token as used
    
ABSTRACT_VARIABLES
    d_variables,
    d_var_it,
    d_var_imp
    
INVARIANT
    d_typing &
    
    d_typing_it &
    
    d_typing_imp &
    
    // Closed or Init sessions shall not have pending requests
    a_cli_pending_requests |> (dom(a_state |> {e_session_closed, e_session_init})) = {} &
    
    // All sessions not (orphaned or closed or init) have a channel associated and only those sessions
    dom(a_state |>> {e_session_scOrphaned, e_session_closed, e_session_init}) = dom(a_channel) &
    
    // All sessions not (created or creating or closed or init) have a user associated and only those sessions
    dom(a_state |>> {e_session_created, e_session_creating, e_session_closed, e_session_init}) = dom(a_user) &
    
    dom(a_state |> {e_session_scOrphaned}) = dom(a_orphaned) &
    
    // All sessions have a token (except when in state creating, init or closed (since it creating can be predecessor))
    dom(a_state |>> {e_session_creating, e_session_closed, e_session_init}) = ran(a_token)    
    
    
INITIALISATION
    s_session := {}          ||
    a_state := {}            ||
    a_channel := {}          ||
    a_orphaned := {}         ||
    a_token := {}            ||
    a_user := {} ||
    
    /* it */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned :(av_sessions_orphaned <: t_session &
        av_sessions_to_iterate <: av_sessions_orphaned &
        av_sessions_iterated <: av_sessions_orphaned &
        av_sessions_to_iterate /\ av_sessions_iterated = {}) ||
    
    /* imp */
    a_cli_pending_requests := {}
    
OPERATIONS
    
    session <-- get_session_from_req_handle (req_handle) =
    PRE
        req_handle : t_request_handle_i
    THEN
        CHOICE
            session :(session : t_session_i & session : s_session)
        OR
            session := c_session_indet
        END
    END
    ;
    
    session <-- get_session_from_token (session_token) =
    PRE
        session_token : t_session_token_i
    THEN
        IF session_token : t_session_token &
            session_token : dom(a_token)
        THEN
            session :(session : t_session_i & session = a_token(session_token))
        ELSE
            session := c_session_indet
        END
    END
    ;
    
    channel <-- get_session_channel (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        IF session : dom(a_channel)
        THEN
            channel :(channel : t_channel_i & channel = a_channel(session))
        ELSE
            channel := c_channel_indet
        END
    END
    ;
    
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : s_session)
    END
    ;
    
    // Initialize a session to be created
    nsession <-- cli_init_session = 
    BEGIN
        IF s_session = t_session
        THEN
            // Maximum number of sessions already allocated
            nsession := c_session_indet
        ELSE
            ANY l_nsession WHERE
                l_nsession : t_session_i &
                l_nsession : t_session - s_session
            THEN
                s_session := s_session \/ {l_nsession} ||
                a_state(l_nsession) := e_session_init  ||
                nsession := l_nsession
            END
        END
    END
    ;
    
    // Client sends create session request on channel
    ret <-- cli_create_req (session, channel, req_handle, create_req_msg) = 
    PRE
        session : t_session_i &
        channel : t_channel_i &
        channel : s_channel &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        IF session : s_session		    
        THEN
            IF a_state(session) : {e_session_init}
            THEN
                IF req_handle /: dom (a_cli_pending_requests)
                THEN
                    CHOICE
                        a_channel(session) := channel                                                ||
                        a_state(session) := e_session_creating                                       ||
                        a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                        ret := e_sc_ok
                    OR
                        // no request handle available, no enough memory for new message, etc.
                        ret := e_sc_bad_out_of_memory
                    END
                ELSE
                    ret := e_sc_bad_invalid_argument
                END // end check valid req handle
            ELSE
                ret := e_sc_bad_invalid_state
            END // end check state
        ELSE
            ret := e_sc_bad_invalid_argument
        END // end check valid session
    END
    ;
    
    // Server receives create session request on channel and sends response
    nsession <-- srv_create_req_and_resp (channel, req_handle, create_req_msg, create_resp_msg) = 
    PRE
        channel : t_channel_i  &
        channel : s_channel &
        req_handle : t_request_handle_i &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_req &
        create_resp_msg : t_msg_i &
        create_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_resp
    THEN
        IF s_session = t_session
        THEN
            // Maximum number of sessions already allocated
            nsession := c_session_indet
        ELSE
            ANY l_nsession, l_nsession_token WHERE
                l_nsession : t_session_i &
                l_nsession : t_session - s_session &
                l_nsession_token : t_session_token_i &
                l_nsession_token : t_session_token - dom(a_token)
            THEN
                CHOICE
                    s_session := s_session \/ {l_nsession}                  || // add new session to created sessions
                    a_token := a_token <+ {l_nsession_token |-> l_nsession} ||
                    a_channel(l_nsession) := channel                        || // associate current channel
                    a_state(l_nsession) := e_session_created                        ||
                    nsession := l_nsession
                    // sends response
                OR
                    // sends error response
                    nsession := c_session_indet
                END
            END
        END
    END
    ;
    
    // Client receive create session response from server
    cli_create_resp (channel, session, req_handle, session_token, create_resp_msg) =
    PRE
        channel       : t_channel_i &
        channel       : s_channel   &
        session       : t_session_i &
        session_token : t_session_token_i &
        req_handle    : t_request_handle_i &
        create_resp_msg : t_msg_i &
        create_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        IF session : s_session
        THEN        
            IF a_state(session) = e_session_creating & // session : s_session <=> session : dom(a_state)
                a_channel(session) = channel  & // current channel is the channel associated to the current session
                session_token : t_session_token &
                session_token /: dom(a_token) & // check it is a fresh session token
                session /: ran(a_token) & // check it is a fresh session
                req_handle : t_request_handle &
                req_handle : dom(a_cli_pending_requests)
            THEN
                CHOICE
                    a_state(session) := e_session_created                     ||
                    a_token := a_token <+ {session_token |-> session} ||
                    a_cli_pending_requests := a_cli_pending_requests - {req_handle |-> session}
                OR
                    // Constraints on message content not verified
                    d_close_session(session)
                END
            ELSE
                d_close_session(session)
            END
        ELSE
            // No state to change, session is not created
            skip
        END
        
    END
    ;
    
    // Client sends a user activate request for current session
    ret, channel, session_token <-- cli_user_activate_req (session, req_handle, user, activate_req_msg) =
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
        IF session : s_session
        THEN
            IF a_state(session) : {e_session_created, e_session_userActivated}
            THEN
                CHOICE
                    IF req_handle /: dom (a_cli_pending_requests)
                    THEN
                        a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                        a_state(session) := e_session_userActivating ||
                        a_user(session) := user              ||
                        ret := e_sc_ok                          ||
                        channel :(channel : t_channel_i & channel : s_channel) ||
                        session_token :(session_token : t_session_token_i & session_token : dom(a_token))
                    ELSE
                        ret := e_sc_bad_invalid_argument ||
                        channel := c_channel_indet ||
                        session_token := c_session_token_indet
                    END
                OR
                    // applicative code provided invalid user: no change
                    // no more request handle available
                    // ...
                    ret :: {e_sc_bad_identity_token_invalid,
                        e_sc_bad_unexpected_error,
                        e_sc_bad_out_of_memory} ||
                    channel := c_channel_indet ||
                    session_token := c_session_token_indet
                END
            ELSE
                ret := e_sc_bad_invalid_state ||
                channel := c_channel_indet ||
                session_token := c_session_token_indet
            END
        ELSE
            ret := e_sc_bad_invalid_argument ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
        END
    END
    ;
    
    // Client sends activate request for new channel: keep for possible applicative initiative on sc activate ?
    ret, session_token <-- cli_sc_activate_req (session, req_handle, channel, activate_req_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        channel : t_channel_i &
        channel : s_channel &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        IF session : s_session
        THEN
            IF a_state(session) : {e_session_scOrphaned, e_session_userActivated}
            THEN
                CHOICE
                    IF req_handle /: dom (a_cli_pending_requests)
                    THEN
                        a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                        a_channel(session) := channel ||
                        a_state(session) := e_session_scActivating ||
                        a_orphaned := {session} <<| a_orphaned ||
                        ret := e_sc_ok ||
                        session_token :(session_token : t_session_token_i & session_token : dom(a_token))
                    ELSE
                        ret := e_sc_bad_invalid_argument ||
                        session_token := c_session_token_indet
                    END
                OR
                    ret :: {e_sc_bad_identity_token_invalid, 
                        e_sc_bad_invalid_argument,
                        e_sc_bad_unexpected_error,
                        e_sc_bad_out_of_memory} ||
                    session_token := c_session_token_indet
                END
            ELSE
                ret := e_sc_bad_invalid_state ||
                session_token := c_session_token_indet
            END
        ELSE
            ret := e_sc_bad_invalid_argument ||
            session_token := c_session_token_indet
        END
    END
    ;
    
    // Server receives a user activate request and sends response
    ret <-- srv_activate_req_and_resp (channel, session, req_handle, user, activate_req_msg, activate_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i &
        req_handle : t_request_handle_i &
        user : t_user_i &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_req &
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_resp
    THEN
        IF session : s_session &
            user    : t_user
        THEN
            IF a_state(session) : {e_session_created, e_session_userActivated} &
                a_channel(session) = channel // current channel is the channel associated to the current session
            THEN
                /* UserActivateReq */
                CHOICE
                    a_state(session) := e_session_userActivated ||
                    a_user(session) := user ||
                    ret := e_sc_ok
                    // modify credential if user was already associated, sends response
                OR
                    // Invalid user or parameters in received request: choice is to close the session in this case (spec does not indicate what to do)
                    d_close_session(session) ||
                    ret := e_sc_bad_invalid_argument
                    // sends response with error
                END
            ELSIF a_state(session) : {e_session_scOrphaned, e_session_userActivated} &
                a_user(session) = user // user provided in activate request is the user associated to the current session
            THEN
                /* ScActivateReq */      
                CHOICE
                    a_orphaned := {session} <<| a_orphaned ||
                    a_channel(session) := channel         ||
                    a_state(session) := e_session_userActivated
                OR
                    // Invalid parameters in received request: choice is to close the session in this case (spec does not indicate what to do)
                    d_close_session(session) ||
                    ret := e_sc_bad_invalid_argument
                    // sends response with error
                END
            ELSE
                d_close_session(session) ||
                ret :: {e_sc_bad_invalid_argument,
                    e_sc_bad_invalid_state}
            END
        ELSIF session : s_session
        THEN
            d_close_session(session) ||
            ret :: {e_sc_bad_invalid_argument,
                    e_sc_bad_unexpected_error}
        ELSE
            // Session is not a created session
            ret := e_sc_bad_invalid_argument
        END
    END
    ;
    
    cli_activate_resp (channel, session, req_handle, activate_resp_msg) = 
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i &
        req_handle : t_request_handle_i &
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_resp
    THEN
        IF session : s_session
        THEN
            IF a_state(session) : {e_session_userActivating, e_session_scActivating} &
                a_channel(session) = channel &
                req_handle : t_request_handle &
                req_handle : dom(a_cli_pending_requests) 
                // current channel is the channel associated to the current session
            THEN
                CHOICE
                    a_state(session) := e_session_userActivated ||
                    a_cli_pending_requests := a_cli_pending_requests - {req_handle |-> session}
                    // modify credential if user was already associated, sends response
                OR
                    // Invalid user or parameters in received request: choice is to close the session in this case (spec does not indicate what to do)
                    d_close_session(session)
                    // sends response with error
                END
            ELSE
                d_close_session(session)
            END
        ELSE
            // Session is not a created session
            skip
        END
    END
    ;
    
    // Client channel lost
    cli_secure_channel_lost (lost_channel, new_channel) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : s_channel &
        new_channel : t_channel_i
    THEN
        IF new_channel : s_channel
        THEN
            // We have a new channel to provide (waiting for connection)
            LET active_sessions, inactive_sessions BE
                active_sessions = dom(a_channel |> {lost_channel}) /\ dom(a_state |>  {e_session_userActivated}) &
                inactive_sessions = dom(a_channel |> {lost_channel}) /\ dom(a_state |>>  {e_session_userActivated})
            IN           
                a_state := a_state <+ (active_sessions*{e_session_scOrphaned})
                // sessions on lost channel that were in activated state are now channel orphaned,
                // other created sessions on channel are closed
                <+ (inactive_sessions *{e_session_closed}) ||
                a_channel := a_channel |>> {lost_channel} ||
                // channel is not the channel of any session
                a_user := inactive_sessions <<| a_user ||
                // sessions CLOSED due to channel loss do not have user anymore
                a_orphaned := a_orphaned <+ active_sessions*{new_channel}
                // sessions ORPHANED of a channel are recorded
            END
        ELSE
            // We do not have new channel to provide: close all session that lost channel
            LET sessions BE
                sessions = dom(a_channel |> {lost_channel})
            IN
                a_state := a_state <+ (sessions *{e_session_closed}) ||
                a_channel := a_channel |>> {lost_channel} ||
                // channel is not the channel of any session
                a_user := sessions <<| a_user
                // sessions CLOSED due to channel loss do not have user anymore
            END
        END
    END
    ;
    
    // Server channel lost
    srv_secure_channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel
    THEN
        LET active_sessions, inactive_sessions BE
            active_sessions = dom(a_channel |> {channel}) /\ dom(a_state |>  {e_session_userActivated}) &
            inactive_sessions = dom(a_channel |> {channel}) /\ dom(a_state |>>  {e_session_userActivated})
        IN           
            a_state := a_state <+ (active_sessions*{e_session_scOrphaned})
            // sessions on lost channel that were in activated state are now channel orphaned,
            // other created sessions on channel are closed
            <+ (inactive_sessions *{e_session_closed}) ||
            a_channel := a_channel |>> {channel} ||
            // channel is not the channel of any session
            a_user := inactive_sessions <<| a_user
            // sessions CLOSED due to channel loss do not have user anymore
            
            // No  need to record channel lost since session will be associated to a new channel by client
            // TODO: trigger a timer on session ?
        END
    END
    ;
    
    ret, channel, session_token <-- cli_close_req (session, req_handle, close_req_msg) =
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
        IF session : s_session &
            a_state(session) = e_session_userActivated
        THEN
            CHOICE
                IF req_handle /: dom (a_cli_pending_requests)
                THEN
                    a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                    a_state(session) := e_session_closing ||
                    ret := e_sc_ok ||
                    channel :(channel : t_channel_i & channel : s_channel) ||
                    session_token :(session_token : t_session_token_i & session_token : dom(a_token))
                ELSE
                    ret := e_sc_bad_invalid_argument ||
                    channel := c_channel_indet ||
                    session_token := c_session_token_indet
                END
            OR
                ret :: {e_sc_bad_out_of_memory, 
                    e_sc_bad_unexpected_error} ||
                channel := c_channel_indet ||
                session_token := c_session_token_indet
            END
        ELSE
            ret :: {e_sc_bad_invalid_state, e_sc_bad_invalid_argument} ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
            // Session is not created or in invalid state for closing session
        END
    END
    ;
    
    
    ret <-- srv_close_req_and_resp (channel, session, req_handle, close_req_msg, close_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i &
        req_handle : t_request_handle_i &
        close_req_msg : t_msg_i &
        close_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_close_req &
        close_resp_msg : t_msg_i &
        close_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_close_resp
    THEN
        IF session : s_session
        THEN
            // In any case session is closed, but behavior different:
            // - invalid channel => just close
            // - invalid state => close and send error response
            // - other => respond to pending requests with error, close and send error response
            d_close_session(session) ||
            ret :: {e_sc_ok,
                e_sc_bad_invalid_argument,
                e_sc_bad_invalid_state}                
        ELSE
            ret := e_sc_bad_invalid_argument // Session is not created
        END
    END
    ;
    
    cli_close_resp (channel, session, req_handle, close_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i &
        req_handle : t_request_handle_i &
        close_resp_msg : t_msg_i &
        close_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_close_resp
    THEN
        IF session : s_session
        THEN
            // In any case session is closed, invalid cases or valid cases have same result
            d_close_session(session)
        ELSE
            skip // Session is not created
        END
    END
    ;
    
    cli_close_session (session) =
    PRE
        session : t_session_i
    THEN
        IF session : s_session
        THEN
            // In any case session is closed, invalid cases or valid cases have same result
            d_close_session(session)
        ELSE
            skip // Session is not created
        END
    END
    ;
    
    srv_close_session (session) =
    PRE
        session : t_session_i
    THEN
        IF session : s_session
        THEN
            // In any case session is closed, invalid cases or valid cases have same result
            d_close_session(session)
        ELSE
            skip // Session is not created
        END
    END
    ;
    
    ret, session_token <-- cli_new_session_service_req (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        a_state(session) = e_session_userActivated
    THEN
        CHOICE
            ret := e_sc_ok ||
            session_token :(session_token : t_session_token_i & session_token : dom(a_token))
        OR
            ret :: {e_sc_bad_invalid_argument,
                e_sc_bad_unexpected_error} ||
            session_token := c_session_token_indet
        END
    END
    ;

    bres <-- cli_record_session_service_resp (session, msg, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) = e_session_userActivated &
        msg : t_msg_i &
        msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type : {e_msg_session_read_resp} &
        req_handle : t_request_handle_i
    THEN
        CHOICE
            bres := TRUE
        OR
            /* invalid req handle */
            bres := FALSE
        END
    END
    ;
    
    ret, snd_err <-- srv_is_session_valid_for_service (channel, session) =
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i
    THEN
        IF session : s_session
        THEN
            IF a_channel(session) = channel  &
                channel : s_channel
            THEN
                IF a_state(session) = e_session_userActivated 
                THEN
                        ret := TRUE ||
                        snd_err := FALSE
                ELSE 
                    ret := FALSE || // Invalid state
                    snd_err := TRUE ||
                    d_close_session(session)
                END
            ELSE
                ret := FALSE || // Invalid channel
                snd_err := FALSE ||
                d_close_session(session)
            END
        ELSE
            ret := FALSE || // Session is not created
            snd_err := FALSE
        END
    END;
    
    ret <-- cli_is_session_valid_for_service (channel, session) =
    PRE
        channel : t_channel_i &
        channel : s_channel   &
        session : t_session_i
    THEN
        IF session : s_session
        THEN
            IF a_channel(session) = channel  &
                channel : s_channel &
                a_state(session) = e_session_userActivated 
            THEN
                ret := TRUE
            ELSE
                ret := FALSE ||
                d_close_session(session)
            END
        ELSE
            ret := FALSE // Session is not created
        END
    END;
    
    continue <-- init_iter_orphaned_t_session (lost_channel) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : t_channel
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
        IF session : s_session
        THEN
            state := a_state(session)
        ELSE
            state := e_session_closed
        END
    END
    ;
    
    user <-- get_session_user_or_indet (session) =
    PRE
        session : t_session_i
    THEN
        IF session : s_session & a_state(session) = e_session_userActivated
        THEN
            user :( user : t_user_i & user = a_user (session))
        ELSE
            user := c_user_indet
        END
    END
    ;
    
    delete_session (session) = 
    PRE
        session : t_session_i &
        session : s_session
    THEN
        s_session := s_session - {session} ||
        a_channel := {session} <<| a_channel ||
        a_state := {session} <<| a_state ||
        a_user := {session} <<| a_user ||
        a_token := a_token |>> {session} ||
        a_orphaned := {session} <<| a_orphaned ||
        a_cli_pending_requests := a_cli_pending_requests |>> {session}
    END
    
END
