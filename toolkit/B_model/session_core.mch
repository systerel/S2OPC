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
    a_client_token,
    a_server_token,
    a_user,
    a_NonceServer,
    a_NonceClient;

    d_var_it ==
    /* iter */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned;

    d_typing ==
    s_session <: t_session &
    a_state : s_session --> t_sessionState & // a session shall be associated to a state
    a_channel : s_session +-> t_channel & // a session may be associated to a valid channel
    a_orphaned : s_session +-> t_channel_config_idx_i & // a client session may be orphaned of a channel (precedently lost)
    a_client_token : s_session +-> t_session_token & // a client session has an associated session token once create response received
    a_server_token : t_session_token >+> s_session & // a server session has a unique authentication token in server once create response sent
    a_user : s_session +-> t_user & // a session may be associated to a valid user
    /* The NonceServer negotiated for a session in CreateSession or ActivateSession */
    a_NonceServer : s_session +-> t_Nonce &
    /* The NonceClient is remembered by the client in Sess_Creating */
    a_NonceClient : s_session +-> t_Nonce;

    d_typing_it ==
    av_sessions_orphaned <: t_session &
    av_sessions_to_iterate <: av_sessions_orphaned &
    av_sessions_iterated <: av_sessions_orphaned &
    av_sessions_to_iterate /\ av_sessions_iterated = {};

    d_close_session (session) ==
    a_channel := {session} <<| a_channel               || // If session had a channel, remove this relation
    a_orphaned := {session} <<| a_orphaned             || // If session was orphaned, remove this relation
    a_user := {session} <<| a_user                     || // If session had a user, remove this relation
    a_client_token := {session} <<| a_client_token     || // If session had a token, remove this relation
    a_server_token := a_server_token |>> {session}     || // If session had a token, remove this relation
    a_state(session) := e_session_closed               || // keep session token as used
    a_NonceServer := {session} <<| a_NonceServer       || /* If session had a server nonce, remove it */
    a_NonceClient := {session} <<| a_NonceClient          /* If session had a client nonce, remove it */

ABSTRACT_VARIABLES
    d_variables,
    d_var_it

INVARIANT
    d_typing &

    d_typing_it &

    // All sessions not (orphaned or closed or init) have a channel associated and only those sessions
    dom(a_state |>> {e_session_scOrphaned, e_session_closed, e_session_init}) = dom(a_channel) &

    // All sessions not (created or creating or closed or init) have a user associated and only those sessions
    dom(a_state |>> {e_session_created, e_session_creating, e_session_closed, e_session_init}) = dom(a_user) &

    dom(a_state |> {e_session_scOrphaned}) = dom(a_orphaned) &

    // All sessions have a token (except when in state creating, init or closed (since it creating can be predecessor))
    dom(a_state |>> {e_session_creating, e_session_closed, e_session_init}) = ran(a_server_token) \/ dom(a_client_token) &

    /* TODO: both nonces are not present when security policy is "None" */
    /* Sessions not (init or creating or closed) have a Nonce, even in security mode None */
    dom(a_state |>> {e_session_init, e_session_creating, e_session_closed}) = dom(a_NonceServer) &

    /* Sessions in Sess_Creating have a Nonce */
    dom(a_state |>> {e_session_creating}) = dom(a_NonceClient)


INITIALISATION
    s_session := {}          ||
    a_state := {}            ||
    a_channel := {}          ||
    a_orphaned := {}         ||
    a_client_token := {}     ||
    a_server_token := {}     ||
    a_user := {}             ||
    a_NonceServer := {}      ||
    a_NonceClient := {}      ||

    /* it */
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned :(av_sessions_orphaned <: t_session &
        av_sessions_to_iterate <: av_sessions_orphaned &
        av_sessions_iterated <: av_sessions_orphaned &
        av_sessions_to_iterate /\ av_sessions_iterated = {})

OPERATIONS

    /* Note: _sm suffix is for (sesion) State Machine, to indicate a transition */

    // Initialize a session to be created
    nsession <-- client_init_session_sm =
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
    valid <-- client_create_session_req_sm (session, channel, create_req_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        session : s_session &
        a_state(session) = e_session_init &
        channel : t_channel_i &
        channel : s_channel_connected &
        create_req_msg : t_msg_i &
        create_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_create_req
    THEN
        CHOICE
            valid := TRUE                           ||
            a_channel(session) := channel           ||
            a_state(session) := e_session_creating  ||
            a_NonceClient
            :(
                a_NonceClient : s_session +-> t_Nonce &
                {session} <<| a_NonceClient = {session} <<| a_NonceClient$0 &
                session : dom(a_NonceClient)
            )
        OR
            valid := FALSE /* Nonce creation can fail */
        END
    END
    ;

    // Server receives create session request on channel and sends response
    nsession, service_ret <-- server_create_session_req_and_resp_sm (channel, create_req_msg, create_resp_msg) =
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
            ANY l_nsession, l_nsession_token, l_nonce WHERE
                l_nsession : t_session_i &
                l_nsession : t_session - s_session &
                l_nsession_token : t_session_token_i &
                l_nsession_token : t_session_token - dom(a_server_token) &
                l_nonce : t_Nonce_i &
                l_nonce : t_Nonce
            THEN
                CHOICE
                    s_session := s_session \/ {l_nsession}                  || // add new session to created sessions
                    a_server_token := a_server_token <+ {l_nsession_token |-> l_nsession} ||
                    a_channel(l_nsession) := channel                        || // associate current channel
                    a_state(l_nsession) := e_session_created                ||
                    nsession := l_nsession                                  ||
                    service_ret := e_sc_ok                                  ||
                    a_NonceServer(l_nsession) := l_nonce
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
    ret <-- client_create_session_resp_sm (channel, session, session_token, create_resp_msg) =
    PRE
        channel       : t_channel_i &
        channel       : s_channel_connected   &
        session       : t_session_i &
        session       : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_creating &
        session_token : t_session_token_i &
        create_resp_msg : t_msg_i &
        create_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        // Note: session does not have associated token due to state guaranteed by PRE
        IF session_token : t_session_token
        THEN
            CHOICE
                a_state(session) := e_session_created       ||
                a_client_token(session) := session_token    ||
                ret := e_sc_ok                              ||
                a_NonceClient := {session} <<| a_NonceClient
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
    ret, channel, session_token <-- client_user_activate_session_req_sm (session, user, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_created, e_session_userActivated} &
        user : t_user_i &
        user : t_user &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        CHOICE
            a_state(session) := e_session_userActivating ||
            a_user(session) := user              ||
            ret := e_sc_ok                          ||
            channel :(channel : t_channel_i & channel : t_channel) ||
            session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
        OR
            // applicative code provided invalid user: no change
            // ...
            ret :: {e_sc_bad_identity_token_invalid,
                e_sc_bad_unexpected_error} ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
        END
    END
    ;

    // Client sends activate request for new channel: keep for possible applicative initiative on sc activate ?
    session_token <-- client_sc_activate_session_req_sm (session, channel, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_scOrphaned, e_session_userActivated} & // last state never used in toolkit (no app API provided)
        channel : t_channel_i &
        channel : s_channel_connected &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        a_channel(session) := channel ||
        a_state(session) := e_session_scActivating ||
        a_orphaned := {session} <<| a_orphaned ||
        session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
    END
    ;

    // Server receives a user activate request and sends response
    ret <-- server_activate_session_req_and_resp_sm (channel, session, user, activate_req_msg, activate_resp_msg) =
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

    ret <-- client_activate_session_resp_sm (channel, session, activate_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) : {e_session_userActivating, e_session_scActivating} &
        activate_resp_msg : t_msg_i &
        activate_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_resp
    THEN
        CHOICE
            a_state(session) := e_session_userActivated ||
            ret := e_sc_ok
            // modify credential if user was already associated, sends response
        OR
            // TBD: invalid response msg content
            ret := e_sc_nok
        END
    END
    ;

    // Client channel lost
    client_secure_channel_lost_session_sm (lost_channel, channel_config_idx) =
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
    server_secure_channel_lost_session_sm (channel) =
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

    ret, channel, session_token <-- client_close_session_req_sm (session, close_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_created,
            e_session_userActivating,
            e_session_userActivated} &
        close_req_msg : t_msg_i &
        close_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_close_req
    THEN
        CHOICE
            a_state(session) := e_session_closing ||
            ret := e_sc_ok ||
            channel :(channel : t_channel_i & channel : t_channel) ||
            session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
        OR
            ret :: {e_sc_bad_out_of_memory,
                e_sc_bad_unexpected_error} ||
            channel := c_channel_indet ||
            session_token := c_session_token_indet
        END
    END
    ;


    ret <-- server_close_session_req_and_resp_sm (channel, session, close_req_msg, close_resp_msg) =
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
        // TODO: respond to pending requests with error if requested in request
        d_close_session(session) ||
        ret :: {e_sc_ok,
            e_sc_bad_unexpected_error}
    END
    ;

    client_close_session_resp_sm (channel, session, close_resp_msg) =
    PRE
        channel : t_channel_i &
        channel : s_channel_connected   &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_closing &
        close_resp_msg : t_msg_i &
        close_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_close_resp
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        d_close_session(session)
    END
    ;


    client_close_session_sm (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        d_close_session(session)
    END
    ;

    server_close_session_sm (session) =
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

    /* PROMOTES */
    session <-- server_get_session_from_token (session_token) =
    PRE
        session_token : t_session_token_i
    THEN
        IF session_token : t_session_token &
            session_token : dom(a_server_token)
        THEN
            session :(session : t_session_i & session = a_server_token(session_token))
        ELSE
            session := c_session_indet
        END
    END
    ;

    session_token <-- client_get_token_from_session (session) =
    PRE
        session : t_session_i &
        session : t_session &
        session : dom(a_client_token)
    THEN
        // Guarantee only 1 session token by session provided by set_session_token PRE
        session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
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

    delete_session (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        s_session := s_session - {session} ||
        a_channel := {session} <<| a_channel ||
        a_state := {session} <<| a_state ||
        a_user := {session} <<| a_user ||
        a_client_token := {session} <<| a_client_token ||
        a_server_token := a_server_token |>> {session} ||
        a_orphaned := {session} <<| a_orphaned
    END


END
