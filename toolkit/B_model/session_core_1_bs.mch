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
    session_core_1_bs
    
SEES
    constants,
    request_handle_bs
    
ABSTRACT_VARIABLES
    s_session,
    a_state,
    a_channel,
    a_cli_pending_requests,
    a_token,
    a_orphaned,
    a_user
    
INVARIANT
    s_session <: t_session &   
    
    a_state : s_session --> t_sessionState &
    
    a_channel : s_session +-> t_channel & 
    
    a_orphaned : s_session +-> t_channel_config_idx &
    
    a_cli_pending_requests : t_request_handle +-> s_session & // 1 request handle is associated to only one session
    
    a_token : t_session_token >+> s_session &
    
    a_user : t_session +-> t_user
    
INITIALISATION
    s_session :: {{}}        ||
    a_state :: {{}}          ||
    a_channel :: {{}}        ||
    a_orphaned :: {{}}       ||
    a_cli_pending_requests :: {{}} ||
    a_token :: {{}}          ||
    a_user :: {{}}
    
OPERATIONS
    
    session <-- get_session_from_token (session_token) =
    PRE
        session_token : t_session_token_i
    THEN
        IF session_token : t_session_token &
            session_token : dom(a_token)
        THEN
            session :(session : t_session_i & session = a_token (session_token))
        ELSE
            session := c_session_indet
        END
    END
    ;

    session_token <-- get_token_from_session (session) =
    PRE
        session : t_session_i &
        session : t_session &
        session : ran(a_token)
    THEN
        // Guarantee only 1 session token by session provided by set_session_token PRE
        session_token :(session_token : t_session_token_i & session_token = a_token~(session))
    END;
    
    token <-- get_fresh_session_token (session) = 
    PRE
        session : t_session_i &
        session : s_session &
        session /: ran(a_token)
    THEN
        IF t_session_token = dom(a_token)
        THEN
            token := c_session_token_indet
        ELSE
            ANY l_ntoken WHERE
                l_ntoken : t_session_token - dom(a_token)
            THEN
                a_token (l_ntoken) := session ||
                token := l_ntoken
            END
        END
    END
    ;
    
    ret <-- is_fresh_session_token (session_token) =
    PRE
        session_token : t_session_token_i
    THEN
        ret := bool(session_token : t_session_token &
            session_token /: dom(a_token))
    END
    ;
    
    ret <-- is_valid_session_token (token) =
    PRE
        token : t_session_token_i
    THEN
        ret := bool(token : t_session_token & token : dom(a_token))
    END
    ;
    
    set_session_token (session, token) =
    PRE
        session : t_session_i &
        session : s_session &
        session /: ran(a_token) &
        token : t_session_token_i &
        token : t_session_token &
        token /: dom(a_token)
    THEN
        a_token (token) := session
    END
    ;
    
    delete_session_token (session_token) =
    PRE
        session_token : t_session_token_i &
        session_token : dom(a_token)
    THEN
        a_token := {session_token} <<| a_token
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
    END;
    
    session <-- init_new_session =
    BEGIN
        IF s_session = t_session
        THEN
            session := c_session_indet
        ELSE
            ANY l_nsession WHERE
                l_nsession : t_session_i &
                l_nsession : t_session &
                l_nsession /: s_session
            THEN
                s_session := s_session \/ {l_nsession} ||
                a_state (l_nsession) := e_session_init ||
                session := l_nsession
            END
        END
    END
    ;
    
    create_session (session, channel, state) =
    PRE
        session : t_session_i &
        session : s_session &
        channel    : t_channel_i &
        channel    : t_channel &
        state : {e_session_created, e_session_creating}
    THEN
        a_channel (session) := channel ||
        a_state (session) := state
    END
    ;
    
    create_session_failure (session) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state (session) : {e_session_created, e_session_creating}
    THEN
        a_channel := {session} <<| a_channel ||
        a_state (session) := e_session_init
    END;
    
    ret <-- cli_add_pending_request (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        req_handle : t_request_handle_i
    THEN
        IF req_handle : t_request_handle &
            req_handle : s_request_handle &
            req_handle : dom(a_cli_pending_requests)
        THEN
            ret := TRUE ||
            a_cli_pending_requests(req_handle) := session
        ELSE            
            ret := FALSE
        END
    END
    ;
    
    ret <-- cli_remove_pending_request (session, req_handle) =
    PRE
        session : t_session_i &
        session : s_session &
        req_handle : t_request_handle_i
    THEN
        IF req_handle : t_request_handle &
           req_handle : dom(a_cli_pending_requests) THEN
            a_cli_pending_requests := {req_handle} <| a_cli_pending_requests ||
            ret := TRUE
        ELSE
            ret := FALSE
        END
    END;
    
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : s_session)
    END
    ;
    
    state <-- get_session_state (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        state := a_state(session)
    END
    ;
    
    set_session_state (session, state) =
    PRE
        session : t_session_i &
        session : s_session &
        state : t_sessionState &
        state /: {e_session_closed}
    THEN
        a_state (session) := state
        // + notify  user app when session activated
    END
	;
        
    set_session_state_closed (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        a_channel := {session} <<| a_channel               || // If session had a channel, remove this relation
        a_orphaned := {session} <<| a_orphaned             || // If session was orphaned, remove this relation
        a_user := {session} <<| a_user                     || // If session had a user, remove this relation
        a_token := a_token |>> {session}                   || // If session had a token, remove this relation
        a_cli_pending_requests := a_cli_pending_requests |>> {session} ||
        a_state(session) := e_session_closed // keep session token as used
    END
    ;
    
    set_session_channel (session, channel) =
    PRE
        session : t_session_i &
        session : s_session &
        channel : t_channel_i &
        channel : t_channel
    THEN
        a_channel (session) := channel
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
    
    // new_channel is channel on which connection is awaited to re-associate session
    set_session_orphaned (session, lost_channel, channel_config_idx) =
    PRE
        session : t_session_i &
        session : s_session &
        lost_channel : t_channel_i &
        lost_channel : t_channel &
        channel_config_idx : t_channel_config_idx_i
    THEN
        IF channel_config_idx : t_channel_config_idx
        THEN 
            a_channel := a_channel |>> {lost_channel} ||
            a_orphaned(session) := channel_config_idx
        ELSE
            a_channel := a_channel |>> {lost_channel} // TODO: to be rechecked: is it ok ?
        END
    END
    ;
    
    ret <-- is_valid_user (user) =
    PRE
        user : t_user_i
    THEN
        ret := bool(user : t_user)
    END
    ;
    
    set_session_user (session, user) =
    PRE
        session : t_session_i &
        session : s_session &
        user : t_user_i &
        user : t_user
    THEN
        a_user (session) := user
    END
    ;
    
    user <-- get_session_user (session) =
    PRE
        session : t_session_i &
        session : s_session &
        session : dom(a_user)
    THEN
        user :(user : t_user_i & user = a_user (session))
    END

END
