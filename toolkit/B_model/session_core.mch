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
    a_orphaned : s_session +-> t_channel_config_idx_i & // a client session may be orphaned of a channel (precedently lost)    
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
        session : s_session &
        session : dom(a_channel)
    THEN
            channel :(channel : t_channel_i & channel = a_channel(session))
    END
    ;
    
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : s_session)
    END
    ;
    
    ret <-- is_valid_user (user) =
    PRE
        user : t_user_i
    THEN
        ret := bool(user : t_user)
    END
    ;
    
    // Initialize a session to be created
    nsession <-- client_init_session = 
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
        session : t_session &
        session : s_session &
        a_state(session) = e_session_init &
        channel : t_channel_i &
        channel : s_channel_connected &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        IF req_handle /: dom (a_cli_pending_requests)
        THEN
            CHOICE
                a_channel(session) := channel                                                ||
                a_state(session) := e_session_creating                                       ||
                a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                ret := e_sc_ok
            OR
                // invalid request handle, channel not connected, etc.
                ret :: {e_sc_bad_invalid_argument}
            END
        ELSE
            ret := e_sc_bad_invalid_argument
        END // end check valid req handle
    END
    ;
    
    // Server receives create session request on channel and sends response
    nsession, service_ret <-- srv_create_req_and_resp (channel, create_req_msg, create_resp_msg) = 
    PRE
        channel : t_channel_i  &
        channel : s_channel_connected &
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
                    a_state(l_nsession) := e_session_created                ||
                    nsession := l_nsession                                  ||
                    service_ret := e_sc_ok
                    // sends response
                OR
                    // sends error response
                    nsession := c_session_indet ||
                    // TODO: could return at least all result codes from table 15
                    service_ret := e_sc_bad_out_of_memory
                END
            END
        END
    END
    ;
    
    // Client receive create session response from server
    ret <-- cli_create_resp (channel, session, req_handle, session_token, create_resp_msg) =
    PRE
        channel       : t_channel_i &
        channel       : s_channel_connected   &
        session       : t_session_i &
        session       : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_creating &
        session_token : t_session_token_i &
        req_handle    : t_request_handle_i &
        create_resp_msg : t_msg_i &
        create_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        IF session_token : t_session_token &
            session_token /: dom(a_token) & // check it is a fresh session token
            session /: ran(a_token) & // check it is a fresh session
            req_handle : t_request_handle &
            req_handle : dom(a_cli_pending_requests)
        THEN
            CHOICE
                a_state(session) := e_session_created                     ||
                a_token := a_token <+ {session_token |-> session} ||
                a_cli_pending_requests := a_cli_pending_requests - {req_handle |-> session} ||
                ret := e_sc_ok
            OR
                // Constraints on message content not verified
                ret := e_sc_nok
            END
        ELSE
            ret := e_sc_bad_invalid_argument
        END
    END
    ;
    
    // Client sends a user activate request for current session
    ret, channel, session_token <-- cli_user_activate_req (session, req_handle, user, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_created, e_session_userActivated} &
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
            IF req_handle /: dom (a_cli_pending_requests)
            THEN
                a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                a_state(session) := e_session_userActivating ||
                a_user(session) := user              ||
                ret := e_sc_ok                          ||
                channel :(channel : t_channel_i & channel : s_channel_connected) ||
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
    END
    ;
    
    // Client sends activate request for new channel: keep for possible applicative initiative on sc activate ?
    ret, session_token <-- cli_sc_activate_req (session, req_handle, channel, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_scOrphaned, e_session_userActivated} & // last state never used in toolkit (no app API provided)
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        channel : t_channel_i &
        channel : s_channel_connected &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
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
    END
    ;
    
    // Server receives a user activate request and sends response
    ret <-- srv_activate_req_and_resp (channel, session, user, activate_req_msg, activate_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_created, e_session_userActivated,
        					e_session_scOrphaned} &
        user : t_user_i &
        user : t_user &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_req &
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_resp
    THEN
        // TODO: split in 2 operations sc/user activation determined by session_mgr ?
        
        IF a_channel(session) = channel // current channel is the channel associated to the current session
        THEN
            /* UserActivateReq */
            CHOICE
                a_state(session) := e_session_userActivated ||
                a_user(session) := user ||
                ret := e_sc_ok
                // modify credential if user was already associated, sends response
            OR
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
                a_state(session) := e_session_userActivated ||
                ret := e_sc_ok
            OR
                ret := e_sc_bad_invalid_argument
                // sends response with error
            END
        ELSE
            ret :: {e_sc_bad_invalid_argument,
                e_sc_bad_invalid_state}
        END
    END
    ;
    
    ret <-- cli_activate_resp (channel, session, req_handle, activate_resp_msg) = 
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) : {e_session_userActivating, e_session_scActivating} &
        req_handle : t_request_handle_i &
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_resp
    THEN
        IF req_handle : t_request_handle &
            req_handle : dom(a_cli_pending_requests) 
            // current channel is the channel associated to the current session
        THEN
            CHOICE
                a_state(session) := e_session_userActivated ||
                a_cli_pending_requests := a_cli_pending_requests - {req_handle |-> session} ||
                ret := e_sc_ok
                // modify credential if user was already associated, sends response
            OR
                // status nok in response
                ret := e_sc_nok
            END
        ELSE
            ret := e_sc_bad_invalid_argument
        END
    END
    ;
    
    // Client channel lost
    client_secure_channel_lost (lost_channel, channel_config_idx) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : s_channel_connected &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
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
            a_orphaned := a_orphaned <+ active_sessions*{channel_config_idx}
            // sessions ORPHANED of a channel are recorded
        END
    END
    ;
    
    // Server channel lost
    server_secure_channel_lost (channel) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected
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
        session : s_session &
        a_state(session) : {e_session_created, 
            e_session_userActivating, 
            e_session_userActivated} &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle &
        close_req_msg : t_msg_i &
        close_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_close_req
    THEN
        CHOICE
            IF req_handle /: dom (a_cli_pending_requests)
            THEN
                a_cli_pending_requests := a_cli_pending_requests <+ {req_handle |-> session} ||
                a_state(session) := e_session_closing ||
                ret := e_sc_ok ||
                channel :(channel : t_channel_i & channel : s_channel_connected) ||
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
    END
    ;
    
    
    ret <-- srv_close_req_and_resp (channel, session, close_req_msg, close_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) : {e_session_created, 
            e_session_userActivating, 
            e_session_userActivated} &
        close_req_msg : t_msg_i &
        close_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_close_req &
        close_resp_msg : t_msg_i &
        close_resp_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_close_resp
    THEN
        // TODO: respond to pending requests with error if close succeeded ?
        d_close_session(session) ||
        ret :: {e_sc_ok,
            e_sc_bad_unexpected_error}                
    END
    ;
    
    cli_close_resp (channel, session, req_handle, close_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_closing &
        req_handle : t_request_handle_i &
        close_resp_msg : t_msg_i &
        close_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_close_resp
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        d_close_session(session)
    END
    ;
    
    cli_close_session (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        d_close_session(session)
    END
    ;
    
    server_close_session (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        // In any case session is closed, but behavior different:
        // - other => respond to pending requests with error, close (TODO by caller)
        d_close_session(session)
    END
    ;
    
    ret, channel, session_token <-- cli_new_session_service_req (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) = e_session_userActivated &
        req_handle : t_request_handle_i &
        req_handle : s_request_handle
    THEN
        CHOICE
            ret := e_sc_ok ||
            session_token :(session_token : t_session_token_i & a_token(session_token) = session) ||
            channel :(channel : t_channel_i & channel = a_channel(session))
        OR
            ret :: {e_sc_bad_invalid_argument,
                e_sc_bad_unexpected_error} ||
            session_token := c_session_token_indet ||
            channel := c_channel_indet
        END
    END
    ;

    bres <-- cli_record_session_service_resp (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) = e_session_userActivated &
        c_msg_in : t_msg &
        a_msg_in_type : s_service_on_session_resps &
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
    
    ret <-- is_session_valid_for_service (channel, session) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_userActivated 
    THEN
        // Only to guarantee PRE are proved
        ret := TRUE
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
