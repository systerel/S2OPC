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
    session_core_2

SEES
    constants,
    request_handle_bs,
    message_out_bs,
    message_in_bs,
    channel_mgr

ABSTRACT_VARIABLES
    s_session,
    s_client_session,
    a_state,
    a_session_init_time,
    a_channel,
    a_channel_nb_sessions,
    a_client_to_create,
    a_client_orphaned,
    a_server_user_auth_attempts,
    a_server_client_locales

INVARIANT
    s_session         <: t_session                            &
    s_client_session  <: t_session                            &
    a_state            : t_session +-> t_sessionState         &
    a_session_init_time : t_session +-> t_timeref             &
    a_channel          : t_session +-> t_channel              &
    a_channel_nb_sessions : t_channel --> NAT                 &
    a_client_to_create : t_session +-> t_channel_config_idx_i &
    a_client_orphaned  : t_session +-> t_channel_config_idx_i &
    a_server_user_auth_attempts : t_session --> NAT &
    a_server_client_locales : t_session --> t_LocaleIds

INITIALISATION
    s_session          := {} ||
    s_client_session   := {} ||
    a_state            := {} ||
    a_session_init_time := {} ||
    a_channel          := {} ||
    a_channel_nb_sessions := t_channel * {0} ||
    a_client_to_create := {} ||
    a_client_orphaned  := {} ||
    a_server_user_auth_attempts := t_session * {0} ||
    a_server_client_locales := t_session * {c_LocaleIds_empty}

OPERATIONS

    add_session(p_session, p_is_client, p_timeref) =
    PRE
        p_session : t_session_i &
        p_session : t_session   &
        p_is_client : BOOL      &
        p_timeref : t_timeref_i &
        p_timeref : t_timeref
    THEN
        s_session := s_session \/ {p_session} ||
        a_state(p_session) := e_session_init ||
        IF p_is_client = TRUE THEN
            s_client_session := s_client_session \/ {p_session}
        END ||
        a_session_init_time(p_session) := p_timeref
    END
    ;

    remove_session(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        s_session := s_session - {p_session} ||
        a_state   := {p_session} <<| a_state ||
        s_client_session := s_client_session - {p_session} ||
        a_session_init_time := {p_session} <<| a_session_init_time
    END
    ;

    reset_session_channel(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_channel,
        a_channel_nb_sessions
        :(a_channel = {p_session} <<| a_channel$0 &
          a_channel_nb_sessions : t_channel --> NAT &
            /* No channel was associated: no change */
            (p_session /: dom(a_channel$0) =>
                a_channel_nb_sessions = a_channel_nb_sessions$0) &
            /* Channel was associated: decrement channel sessions counter */
            (p_session : dom(a_channel$0) =>
                {a_channel$0 (p_session)} <<| a_channel_nb_sessions = {a_channel$0 (p_session)} <<| a_channel_nb_sessions$0 &
                a_channel_nb_sessions (a_channel$0 (p_session)) = a_channel_nb_sessions$0 (a_channel$0 (p_session)) - 1)
        )
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

    reset_session_orphaned(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_client_orphaned := {p_session} <<| a_client_orphaned
    END
    ;

    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i
    THEN
        ret :=  bool(session : s_session)
    END
    ;

    ret <-- is_client_session (p_session) =
    PRE
        p_session : t_session_i  &
        p_session : dom(a_state)
    THEN
        ret := bool(p_session : s_client_session)
    END
    ;

    state <-- get_session_state (session) =
    PRE
        session : t_session_i &
        session : dom(a_state)
    THEN
        state := a_state(session)
    END
    ;

    set_session_state_1(p_session, p_state) =
    PRE
        p_session : t_session_i  &
        p_session : dom(a_state) &
        p_state   : t_sessionState_i &
        p_state   : t_sessionState
    THEN
        a_state(p_session) := p_state
    END
    ;

    set_session_channel (session, channel) =
    PRE
        session : t_session_i &
        session : t_session   &
        channel : t_channel_i &
        channel : t_channel
    THEN
        a_channel,
        a_channel_nb_sessions
        :(a_channel : t_session +-> t_channel &
          {session} <<| a_channel = {session} <<| a_channel$0 &
          a_channel (session) = channel &
          a_channel_nb_sessions : t_channel --> NAT &
           /* Same channel as previous associated to session: do not change channel sessions counter */
          (a_channel$0 (session) = channel => a_channel_nb_sessions = a_channel_nb_sessions$0) &
          (a_channel$0 (session) /= channel =>
                /* No previous channel associated to session: only increment the channel sessions counter */
                (a_channel$0 (session) = c_channel_indet =>
                    a_channel_nb_sessions (channel) = a_channel_nb_sessions (channel) + 1 &
                    {channel} <<| a_channel_nb_sessions = {channel} <<| a_channel_nb_sessions$0)
                ) &
                /* Previous channel associated to session: decrement previous channel and increment new channel sessions counters */
                (a_channel$0 (session) /= c_channel_indet =>
                    a_channel_nb_sessions (a_channel$0 (session)) = a_channel_nb_sessions (a_channel$0 (session)) - 1 &
                    a_channel_nb_sessions (channel) = a_channel_nb_sessions (channel) + 1 &
                    /* No change for other channels */
                    {channel, a_channel$0 (session)} <<| a_channel_nb_sessions = {channel, a_channel$0 (session)} <<| a_channel_nb_sessions$0
                )
          )
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
        channel :(channel : t_channel_i &
                  channel = a_channel(session))
    END
    ;

    p_nb_sessions <-- get_channel_nb_sessions (p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        p_nb_sessions
        :(p_nb_sessions : NAT &
          p_nb_sessions = a_channel_nb_sessions (p_channel)
        )
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

    set_session_orphaned_1(p_session, p_channel_config_idx) =
    PRE
        p_session            : t_session_i            &
        p_session            : t_session              &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        a_client_orphaned(p_session) := p_channel_config_idx
    END
    ;

    set_server_session_preferred_locales(p_session, p_localesIds) =
    PRE
        p_session    : t_session_i &
        p_session    : t_session &
        p_localesIds : t_LocaleIds_i &
        p_localesIds : t_LocaleIds
    THEN
        a_server_client_locales(p_session) := p_localesIds
    END
    ;

    p_localeIds <-- get_server_session_preferred_locales(p_session) =
    PRE
        p_session    : t_session_i &
        p_session    : t_session
    THEN
        p_localeIds :(p_localeIds : t_LocaleIds_i & p_localeIds = a_server_client_locales(p_session))
    END
    ;

    p_localeIds <-- reset_server_session_preferred_locales(p_session) =
    PRE
        p_session            : t_session_i &
        p_session            : t_session
    THEN
        /* Retrieve recorded localeIds if not the empty one (constant value) and reset to empty one */
        p_localeIds,
        a_server_client_locales
        :(p_localeIds : t_LocaleIds_i &
          a_server_client_locales : t_session --> t_LocaleIds &
          (a_server_client_locales$0(p_session) = c_LocaleIds_empty => p_localeIds = c_LocaleIds_indet) &
          (a_server_client_locales$0(p_session) /= c_LocaleIds_empty => p_localeIds = a_server_client_locales$0(p_session)) &
          {p_session} <<| a_server_client_locales = {p_session} <<| a_server_client_locales$0 &
          {p_session} <| a_server_client_locales = {p_session |-> c_LocaleIds_empty})
    END
    ;

    set_server_session_user_auth_attempts(p_session, p_attempts) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_attempts : NAT
    THEN
        a_server_user_auth_attempts(p_session) := p_attempts
    END
    ;

    p_attempts <-- get_server_session_user_auth_attempts(p_session) =
    PRE
        p_session : t_session_i &
        p_session : dom(a_server_user_auth_attempts)
    THEN
        p_attempts := a_server_user_auth_attempts(p_session)
    END
    ;

    p_timeref <-- get_init_time(p_session) =
    PRE
        p_session : t_session_i &
        p_session : dom(a_session_init_time)
    THEN
        p_timeref :(p_timeref : t_timeref_i & p_timeref = a_session_init_time(p_session))
    END
    ;

    p_nb_sessions <-- get_card_s_session =
    BEGIN
        p_nb_sessions := card(s_session)
    END

END
