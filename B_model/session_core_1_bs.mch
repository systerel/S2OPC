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
    request_handle_bs,
    channel_mgr_bs,
    message_out_bs,
    message_in_bs

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
    a_NonceClient : s_session +-> t_Nonce

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    d_variables :(d_typing)

OPERATIONS

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

    token <-- server_get_fresh_session_token (session) =
    PRE
        session : t_session_i &
        session : s_session &
        session /: ran(a_server_token)
    THEN
        IF t_session_token = dom(a_server_token)
        THEN
            token := c_session_token_indet
        ELSE
            ANY l_ntoken WHERE
                l_ntoken : t_session_token - dom(a_server_token)
            THEN
                a_server_token (l_ntoken) := session ||
                token := l_ntoken
            END
        END
    END
    ;

    ret <-- server_is_valid_session_token (token) =
    PRE
        token : t_session_token_i
    THEN
        ret := bool(token : t_session_token & token : dom(a_server_token))
    END
    ;

    client_set_session_token (session, token) =
    PRE
        session : t_session_i &
        session : s_session &
        session /: dom(a_client_token) &
        token : t_session_token_i &
        token : t_session_token
    THEN
        a_client_token (session) := token
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
        a_server_token := a_server_token |>> {session} ||
        a_client_token := {session} <<| a_client_token ||
        a_orphaned := {session} <<| a_orphaned
    END;

    session <-- init_new_session (is_client) =
    PRE
        is_client : BOOL
    THEN
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

    set_session_state_closed (session, is_client) =
    PRE
        session : t_session_i &
        session : s_session &
        is_client : BOOL
    THEN
        a_channel := {session} <<| a_channel               || // If session had a channel, remove this relation
        a_orphaned := {session} <<| a_orphaned             || // If session was orphaned, remove this relation
        a_user := {session} <<| a_user                     || // If session had a user, remove this relation
        a_server_token := a_server_token |>> {session}     || // If session had token, remove this relation
        a_client_token := {session} <<| a_client_token
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
    ;

    /* This implements the crypto challenge in CreateSession.
     * valid tracks the alloc and success of both the a_NonceServer() and signature
     * TODO? split to gather common Nonce-generation code with others using it
     */
    valid, signature <-- server_create_session_req_do_crypto(p_session, p_req_msg, p_endpoint_config_idx, p_channel_config_idx) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_req_msg : t_msg_i &
        p_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_req &
        p_endpoint_config_idx : t_endpoint_config_idx_i &
        p_endpoint_config_idx : t_endpoint_config_idx &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        valid,
        signature,
        a_NonceServer
        :(
            valid : BOOL &
            signature : t_SignatureData_i &

            (valid = TRUE
             =>
             signature : t_SignatureData &
             a_NonceServer(p_session) : t_Nonce)
            &
            (valid = FALSE
             =>
             a_NonceServer = a_NonceServer$0)
        )
    END
    ;
    
    valid, signature <-- client_activate_session_req_do_crypto(session, channel_config_idx, server_nonce) = 
    PRE
        session : t_session_i &
        session : s_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        server_nonce : t_Nonce_i &
        server_nonce : t_Nonce &
        server_nonce = a_NonceServer(session)
    THEN
        valid,
        signature,
        a_NonceClient
        :(
            valid : BOOL &
            signature : t_SignatureData_i &

            (valid = TRUE
             =>
             signature : t_SignatureData)
            &
            (valid = FALSE
             =>
             signature = c_SignatureData_indet)
        )
    END
    ;
    
    nonce <-- get_NonceServer(p_session) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_session : dom(a_NonceServer)
    THEN
        nonce :(
            nonce : t_Nonce_i &
            nonce = a_NonceServer(p_session)
        )
    END
    ;

    valid, nonce_needed <-- client_create_session_req_do_crypto(p_session, p_channel, p_channel_config_idx) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_channel : t_channel_i &
        p_channel : s_channel_connected &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        IF a_SecurityPolicy(p_channel) = e_secpol_None THEN
            valid :: BOOL || /* This fails when we cannot retreive the SecurityPolicy  <-- but this should not be possible */
            nonce_needed := FALSE
        ELSE
            nonce_needed := TRUE ||
            IF valid = TRUE THEN
                a_NonceClient
                :(
                    a_NonceClient : s_session +-> t_Nonce &
                    {p_session} <<| a_NonceClient = {p_session} <<| a_NonceClient$0 &
                    a_NonceClient(p_session) : t_Nonce
                )
            END
        END
    END
    ;

    nonce <-- get_NonceClient(p_session) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_session : dom(a_NonceClient)
    THEN
        nonce :(
            nonce : t_Nonce_i &
            nonce = a_NonceClient(p_session)
        )
    END
    ;

    drop_NonceClient(p_session) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_session : dom(a_NonceClient)
    THEN
        a_NonceClient := {p_session} <<| a_NonceClient
    END
    ;

    /* This function is not called when there is no client nonce (ie SC:Sec_pol is None) */
    valid <-- client_create_session_check_crypto(p_session, p_channel_config_idx, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : s_session &
        p_session : dom(a_NonceClient) &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx &
        p_resp_msg : t_msg_i &
        p_resp_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_create_resp
    THEN
        CHOICE
            valid := TRUE ||
            a_NonceServer
            :(
                a_NonceServer : s_session +-> t_Nonce &
                {p_session} <<| a_NonceServer = {p_session} <<| a_NonceServer$0 &
                a_NonceServer(p_session) : t_Nonce
            )
        OR
            valid := FALSE
        END
    END
    ;
    
    valid <-- server_activate_session_check_crypto(session, channel, channel_config_idx, activate_req_msg) =
    PRE
        session : t_session_i &
        session : s_session &
        session : dom(a_NonceClient) &
        channel : t_channel_i &
        channel : s_channel_connected &
        a_SecurityPolicy(channel) /= e_secpol_None &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx &
        activate_req_msg : t_msg_i &
        activate_req_msg = c_msg_in &
        c_msg_in : t_msg &
        a_msg_in_type = e_msg_session_activate_req
    THEN
        CHOICE
            valid := TRUE ||
            a_NonceServer
            :(
                a_NonceServer : s_session +-> t_Nonce &
                {session} <<| a_NonceServer = {session} <<| a_NonceServer$0 &
                a_NonceServer(session) : t_Nonce
            )
        OR
            valid := FALSE
        END
    END

END
