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
    a_state,
    a_channel,
    a_client_to_create,
    a_client_orphaned,
    a_server_client_locales

INVARIANT
    s_session         <: t_session                            &
    a_state            : t_session +-> t_sessionState         &
    a_channel          : t_session >+> t_channel              &
    a_client_to_create : t_session +-> t_channel_config_idx_i &
    a_client_orphaned  : t_session +-> t_channel_config_idx_i &
    a_server_client_locales : t_session --> t_LocaleIds

INITIALISATION
    s_session          := {} ||
    a_state            := {} ||
    a_channel          := {} ||
    a_client_to_create := {} ||
    a_client_orphaned  := {} ||
    a_server_client_locales := t_session * {c_LocaleIds_empty}

OPERATIONS

    add_session(p_session, p_state) =
    PRE
        p_session : t_session_i &
        p_session : t_session   &
        p_state   : t_sessionState
    THEN
        s_session := s_session \/ {p_session} ||
        a_state(p_session) := p_state
    END
    ;

    remove_session(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        s_session := s_session - {p_session} ||
        a_state   := {p_session} <<| a_state
    END
    ;

    reset_session_channel(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_channel := {p_session} <<| a_channel
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
        a_channel (session) := channel
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

    p_session <-- get_channel_session (p_channel) =
    PRE
        p_channel : t_channel_i &
        p_channel : t_channel
    THEN
        p_session
        :(p_session : t_session_i &
            (p_channel : dom(a_channel~) => p_session = a_channel~(p_channel)) &
            (p_channel /: dom(a_channel~) => p_session = c_session_indet)
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

END
