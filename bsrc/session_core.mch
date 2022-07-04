/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

MACHINE
    session_core
SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    channel_mgr,
    request_handle_bs

DEFINITIONS
    d_variables ==
    s_session,
    a_state,
    a_channel,
    a_client_to_create,
    a_client_app_context,
    a_client_orphaned,
    a_client_token,
    a_server_token,
    a_user_server,
    a_user_client,
    a_NonceServer,
    a_NonceClient;

    d_typing ==
    s_session <: t_session &
    a_state : s_session --> t_sessionState & // a session shall be associated to a state
    a_channel : s_session +-> t_channel & // a session may be associated to a valid channel
    a_client_to_create : s_session +-> t_channel_config_idx_i & // a client session may be waiting to be created on a channel (not yet established)
    a_client_app_context : s_session +-> t_session_application_context_i & // a client session has an application context associated
    a_client_orphaned : s_session +-> t_channel_config_idx_i & // a client session may be orphaned of a channel (precedently lost)
    a_client_token : s_session +-> t_session_token & // a client session has an associated session token once create response received
    a_server_token : t_session_token >+> s_session & // a server session has a unique authentication token in server once create response sent
    a_user_server : s_session +-> t_user & // a session may be associated to a valid user
    a_user_client : s_session +-> t_user_token &
    /* The NonceServer negotiated for a session in CreateSession or ActivateSession */
    a_NonceServer : s_session +-> t_Nonce &
    /* The NonceClient is remembered by the client in Sess_Creating */
    a_NonceClient : s_session +-> t_Nonce;

    d_close_session (session) ==
    s_session := s_session - {session}                   || // If session was valid session, remove it
    a_channel := {session} <<| a_channel                 || // If session had a channel, remove this relation
    a_client_orphaned := {session} <<| a_client_orphaned || // If session was orphaned, remove this relation
    a_user_server := {session} <<| a_user_server         || // If session had a user, remove this relation
    a_user_client := {session} <<| a_user_client         || // If session had a user, remove this relation
    a_client_token := {session} <<| a_client_token       || // If session had a token, remove this relation
    a_server_token := a_server_token |>> {session}       || // If session had a token, remove this relation
    a_state(session) := e_session_closed                 || // keep session token as used
    a_NonceServer := {session} <<| a_NonceServer         || /* If session had a server nonce, remove it */
    a_NonceClient := {session} <<| a_NonceClient            /* If session had a client nonce, remove it */


ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing &

    // All sessions not (orphaned or closed or init) have a channel associated and only those sessions
    dom(a_state |>> {e_session_scOrphaned, e_session_closed, e_session_init}) = dom(a_channel) &

    // All sessions not (created or creating or closed or init) have a user associated and only those sessions
    dom(a_state |>> {e_session_created, e_session_creating, e_session_closed, e_session_init}) = dom(a_user_server) \/ dom(a_user_client) &

    // All sessions to be created are included in session in init state
    dom(a_client_to_create) <: dom(a_state |> {e_session_init}) &

    // All sessions to be activated keep application context for notifications until session closed (since re-activation can occur)
    dom(a_client_app_context) <: dom(a_state |>> {e_session_closed}) &

    // All server sessions have a user token (except when in state creating, init or closed (since it creating can be predecessor))
    dom(a_state |>> {e_session_creating, e_session_closed, e_session_init}) = ran(a_server_token) &

    // All client sessions have a user token
    dom(a_state |>> {e_session_closed}) = dom(a_client_token) &

    // Client orphaned sessions are included in orphaned sessions
    dom(a_client_orphaned) <: dom(a_state |> {e_session_scOrphaned}) &

    /* Sessions not (init or creating or closed) have a Nonce when their security policy is not None*/
    /* The security policy part cannot be modeled in this invariant, but is in the operations */
    /* TODO: decide whether this invariant is useful or not, as it is weak. */
    dom(a_NonceServer) <: dom(a_state |>> {e_session_init, e_session_creating, e_session_closed}) &

    /* Sessions in Sess_Creating and which security policy is not None have a Nonce */
    dom(a_NonceClient) <: dom(a_state |> {e_session_creating})


INITIALISATION
    s_session                    := {} ||
    a_state                      := {} ||
    a_channel                    := {} ||
    a_client_to_create           := {} ||
    a_client_app_context         := {} ||
    a_client_orphaned            := {} ||
    a_client_token               := {} ||
    a_server_token               := {} ||
    a_user_server                := {} ||
    a_user_client                := {} ||
    a_NonceServer                := {} ||
    a_NonceClient                := {}

OPERATIONS

    /* Note: _sm suffix is for (session) State Machine, to indicate a transition */

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
            IF a_SecurityPolicy(channel) /= e_secpol_None THEN
                a_NonceClient
                :(
                    a_NonceClient : s_session +-> t_Nonce &
                    {session} <<| a_NonceClient = {session} <<| a_NonceClient$0 &
                    session : dom(a_NonceClient)
                )
            END
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
            nsession := c_session_indet ||
            service_ret := e_sc_bad_too_many_sessions
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
                    service_ret :: {e_sc_bad_too_many_sessions, e_sc_bad_out_of_memory, e_sc_bad_unexpected_error}
                END
            END
        END
    END
    ;

    // Client receive create session response from server
    bret <-- client_create_session_resp_sm (channel, session, session_token, create_resp_msg) =
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
                bret := TRUE                                ||
                a_NonceClient := {session} <<| a_NonceClient ||
                a_NonceServer :(
                {session} <<| a_NonceServer = {session} <<| a_NonceServer$0 &
                session : dom(a_NonceServer))
            OR
                // Constraints on message content not verified
                bret := FALSE
            END
        ELSE
            bret := FALSE
        END
    END
    ;

    // Client sends a user activate request for current session
    ret, channel, session_token <-- client_user_activate_session_req_sm (session, p_user_token, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        a_state(session) : {e_session_created, e_session_userActivated} &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_out &
        c_msg_out : t_msg &
        a_msg_out_type = e_msg_session_activate_req
    THEN
        CHOICE
            a_state(session) := e_session_userActivating ||
            a_user_client(session) := p_user_token       ||
            ret := e_sc_ok                               ||
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
            session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
        OR
            // applicative code provided invalid p_user_token: no change
            // ...
            ret :: {e_sc_bad_security_checks_failed,
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
        a_client_orphaned := {session} <<| a_client_orphaned ||
        session_token :(session_token : t_session_token_i & session_token = a_client_token(session))
    END
    ;

    p_sc_valid_user, p_user <-- allocate_authenticated_user (p_channel, p_session, p_user_token) =
    PRE
        p_channel : t_channel_i &
        p_channel : s_channel_connected &
        p_session : t_session_i &
        p_session : s_session &
        a_state(p_session) : {e_session_created, e_session_userActivated,
                              e_session_scOrphaned} &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        p_sc_valid_user,
        p_user
        :(p_sc_valid_user : t_StatusCode_i &
          p_sc_valid_user : t_StatusCode &
          p_user : t_user_i &
          (p_sc_valid_user = e_sc_ok => p_user : t_user) &
          (p_sc_valid_user /= e_sc_ok =>
              p_user = c_user_indet))
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
                a_user_server(session) := user ||
                ret := e_sc_ok
                // modify credential if user was already associated, sends response
            OR
                ret :: {e_sc_bad_invalid_state, e_sc_bad_application_signature_invalid}
                // sends response with error
            END
        ELSIF a_state(session) : {e_session_scOrphaned, e_session_userActivated} &
            a_user_server(session) = user // user provided in activate request is the user associated to the current session
        THEN
            /* ScActivateReq */
            CHOICE
                a_client_orphaned := {session} <<| a_client_orphaned ||
                a_channel(session) := channel         ||
                a_state(session) := e_session_userActivated ||
                ret := e_sc_ok
            OR
                ret := e_sc_bad_application_signature_invalid
                // sends response with error
            END
        ELSE
            ret :: {e_sc_bad_invalid_argument,
                    e_sc_bad_invalid_state}
        END
    END
    ;

    bret <-- client_activate_session_resp_sm (channel, session, activate_resp_msg) =
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
            bret := TRUE
            // modify credential if user was already associated, sends response
        OR
            // TBD: invalid response msg content
            bret := FALSE
        END
    END
    ;

    // Client channel lost
    client_secure_channel_lost_session_sm (p_lost_channel, p_channel_config_idx) =
    PRE
        p_lost_channel       : t_channel_i            &
        p_lost_channel       : s_channel_connected    &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        // We have a new channel to provide (waiting for connection)
        LET active_sessions, inactive_sessions BE
            active_sessions   = ( t_session /\ a_channel~[{p_lost_channel}] /\ a_state~[{e_session_userActivated}]) &
            inactive_sessions = ((t_session /\ a_channel~[{p_lost_channel}]) - a_state~[{e_session_userActivated}])
        IN
            s_session                    := s_session - inactive_sessions                                                       ||
            a_state                      := inactive_sessions <<| a_state <+ (active_sessions * {e_session_scOrphaned})         ||
            a_channel                    := inactive_sessions <<| a_channel |>> {p_lost_channel}                                ||
            a_client_to_create           := inactive_sessions <<| a_client_to_create                                            ||
            a_client_app_context         := inactive_sessions <<| a_client_app_context                                          ||
            a_client_orphaned            := inactive_sessions <<| a_client_orphaned <+ active_sessions * {p_channel_config_idx} ||
            a_user_client                := inactive_sessions <<| a_user_client                                                 ||
            a_server_token               := a_server_token |>> inactive_sessions                                                ||
            a_client_token               := inactive_sessions <<| a_client_token                                                ||
            a_NonceServer                := inactive_sessions <<| a_NonceServer                                                 ||
            a_NonceClient                := inactive_sessions <<| a_NonceClient
        END
    END
    ;

    // Server channel lost
    server_secure_channel_lost_session_sm (p_lost_channel) =
    PRE
        p_lost_channel : t_channel_i &
        p_lost_channel : s_channel_connected
    THEN
        LET active_sessions, inactive_sessions BE
            active_sessions   = ( t_session /\ a_channel~[{p_lost_channel}] /\ a_state~[{e_session_userActivated}]) &
            inactive_sessions = ((t_session /\ a_channel~[{p_lost_channel}]) - a_state~[{e_session_userActivated}])
        IN
            // sessions on lost channel that were in activated state are now channel orphaned,
            // other created sessions on channel are closed
            // channel is not the channel of any session
            // sessions CLOSED due to channel loss do not have user anymore
            // No  need to record channel lost since session will be associated to a new channel by client
            // TODO: trigger a timer on session ?
            s_session                    := s_session - inactive_sessions                                               ||
            a_state                      := inactive_sessions <<| a_state <+ (active_sessions * {e_session_scOrphaned}) ||
            a_channel                    := inactive_sessions <<| a_channel |>> {p_lost_channel}                        ||
            a_client_to_create           := inactive_sessions <<| a_client_to_create                                    ||
            a_client_app_context         := inactive_sessions <<| a_client_app_context                                  ||
            a_client_orphaned            := inactive_sessions <<| a_client_orphaned                                     ||
            a_user_server                := inactive_sessions <<| a_user_server                                         ||
            a_server_token               := a_server_token |>> inactive_sessions                                        ||
            a_client_token               := inactive_sessions <<| a_client_token                                        ||
            a_NonceServer                := inactive_sessions <<| a_NonceServer                                         ||
            a_NonceClient                := inactive_sessions <<| a_NonceClient
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
            channel :(channel : t_channel_i & channel : s_channel_connected) ||
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


    client_close_session_sm (session, sc_reason) =
    PRE
        session : t_session_i &
        session : s_session &
        sc_reason : t_StatusCode_i &
        sc_reason : t_StatusCode
    THEN
        // In any case session is closed, invalid cases or valid cases have same result
        d_close_session(session)
    END
    ;

    server_close_session_sm (session, sc_reason) =
    PRE
        session : t_session_i &
        session : s_session &
        sc_reason : t_StatusCode_i &
        sc_reason : t_StatusCode
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
        channel : t_channel &
        session : t_session_i &
        session : s_session &
        a_channel(session) = channel &
        a_state(session) = e_session_userActivated
    THEN
        // Only to guarantee PRE are proved
        ret := TRUE
    END;

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

    p_dom, p_channel <-- getall_session_channel (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        p_dom := bool(p_session : dom(a_channel)) ||
        IF  p_session : dom(a_channel) THEN
            p_channel
            :(  p_channel : t_channel_i &
                p_channel = a_channel(p_session)
            )
        ELSE
            p_channel :: t_channel_i
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

    reset_session_to_create(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_client_to_create := {p_session} <<| a_client_to_create
    END
    ;

    p_dom, p_channel_config_idx <-- getall_to_create(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        p_dom := bool(p_session : dom(a_client_to_create)) ||
        IF  p_session : dom(a_client_to_create) THEN
            p_channel_config_idx
            :(  p_channel_config_idx : t_channel_config_idx_i &
                p_channel_config_idx = a_client_to_create(p_session)
            )
        ELSE
            p_channel_config_idx :: t_channel_config_idx_i
        END
    END
    ;

    p_dom, p_channel_config_idx <-- getall_orphaned(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        p_dom := bool(p_session : dom(a_client_orphaned)) ||
        IF  p_session : dom(a_client_orphaned) THEN
            p_channel_config_idx
            :(  p_channel_config_idx : t_channel_config_idx_i &
                p_channel_config_idx = a_client_orphaned(p_session)
            )
        ELSE
            p_channel_config_idx :: t_channel_config_idx_i
        END
    END
    ;

    set_session_to_create(p_session, p_channel_config_idx) =
    PRE
        p_session            : t_session_i            &
        p_session            : s_session              &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        a_client_to_create(p_session) := p_channel_config_idx
    END
    ;

    set_session_app_context(p_session, p_app_context) =
    PRE
        p_session     : t_session_i    &
        p_session     : s_session      &
        p_app_context : t_session_application_context_i
    THEN
        a_client_app_context(p_session) := p_app_context
    END
    ;

    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : s_session)
    END
    ;

    drop_user_server (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_user_server := {p_session} <<| a_user_server
        /* Also deallocate user */
    END
    ;

    set_session_user_client (session, p_user_token) =
    PRE
        session      : t_session_i    &
        session      : t_session      &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        a_user_client (session) := p_user_token
    END
    ;

    p_user_token <-- get_session_user_client (session) =
    PRE
        session : t_session_i &
        session : t_session   &
        session : dom(a_user_client)
    THEN
        p_user_token :(p_user_token : t_user_token_i &
                       p_user_token = a_user_client (session))
    END
    ;

    p_user <-- get_session_user_server (session) =
    PRE
        session : t_session_i &
        session : t_session   &
        session : dom(a_user_server)
    THEN
        p_user :(p_user : t_user_i &
                 p_user = a_user_server (session))
    END
    ;

    client_gen_create_session_internal_event (session, channel_config_idx) =
    PRE
        session : t_session_i &
        session : t_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    client_gen_activate_orphaned_session_internal_event (session, channel_config_idx) =
    PRE
        session : t_session_i &
        session : t_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        skip
    END
    ;

    client_gen_activate_user_session_internal_event (session, p_user_token) =
    PRE
        session : t_session_i &
        session : t_session &
        p_user_token : t_user_token_i &
        p_user_token : t_user_token
    THEN
        skip
    END
    ;

    timer_created <-- server_session_timeout_start_timer (session, resp_msg) =
    PRE
        session : t_session_i &
        session : t_session &
        resp_msg : t_msg_i &
        resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        timer_created :: BOOL
    END
    ;

    expired <-- server_session_timeout_evaluation (session) =
    PRE
        session : t_session_i &
        session : s_session
    THEN
        expired :: BOOL
    END
    ;

    server_session_timeout_msg_received (session) =
    PRE
        session : t_session_i &
        session : t_session
    THEN
        skip
    END
    ;

    p_localeIds <-- get_server_session_preferred_locales(p_session) =
    PRE
        p_session    : t_session_i &
        p_session    : t_session
    THEN
        p_localeIds :(p_localeIds : t_LocaleIds_i & p_localeIds : t_LocaleIds)
    END
    ;

    p_has_channel_to_close, p_channel_to_close <-- find_channel_to_close =
    BEGIN
        p_has_channel_to_close,
        p_channel_to_close
        :(p_has_channel_to_close : BOOL &
            p_channel_to_close : t_channel_i &
            p_has_channel_to_close = bool(s_channel_connected - ran(a_channel) /= {}) &
            (p_has_channel_to_close = TRUE => p_channel_to_close : s_channel_connected - ran(a_channel)) &
            (p_has_channel_to_close = FALSE => p_channel_to_close = c_channel_indet)
        )
    END
    ;

    bres, channel <-- getall_valid_session_channel (session) =
    PRE
        session  : t_session_i
    THEN
        CHOICE
            bres := FALSE ||
            channel := c_channel_indet
        OR
            bres := TRUE ||
            channel :(channel : t_channel_i & channel : t_channel)
        END
    END
    ;

    deallocate_user (p_user) =
    PRE
        p_user : t_user_i
    THEN
        skip
    END
    ;

    p_user <-- get_local_user (endpoint_config_idx) =
    PRE
        endpoint_config_idx : t_endpoint_config_idx_i &
        endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user :( p_user : t_user_i &
                  p_user : t_user )
    END
    ;

    p_user_token_policy_available <-- has_user_token_policy_available (p_channel_config_idx, p_endpoint_config_idx) =
    PRE
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx
    THEN
        p_user_token_policy_available :: BOOL
    END

END
