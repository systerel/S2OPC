/*
 *  Copyright (C) 2018 Systerel and others.
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
    a_orphaned

INVARIANT
    s_session <: t_session                    &
    a_state    : t_session +-> t_sessionState &
    a_channel  : t_session +-> t_channel      &
    a_orphaned : t_session +-> t_channel_config_idx_i

INITIALISATION
    s_session  := {}                             ||
    a_state    := t_session * {e_session_closed} ||
    a_channel  := {}                             ||
    a_orphaned := {}

OPERATIONS

    add_session(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        s_session := s_session \/ {p_session}
    END
    ;

    remove_session(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        s_session := s_session - {p_session}
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
    
    reset_session_orphaned(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_orphaned := {p_session} <<| a_orphaned
    END
    ;
   
    ret <-- is_valid_session (session) =
    PRE
        session : t_session_i &
        session : t_session
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

    set_session_state_1(p_session, p_state) =
    PRE
        p_session : t_session_i &
        p_session : s_session   &
        p_state   : t_sessionState
    THEN
        a_state(p_session) := p_state
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
        channel :(channel : t_channel_i &
                  channel = a_channel(session))
    END
    ;

    set_session_orphaned_1(p_session, p_channel_config_idx) =
    PRE
        p_session            : t_session_i            &
        p_session            : s_session              &
        p_channel_config_idx : t_channel_config_idx_i &
        p_channel_config_idx : t_channel_config_idx
    THEN
        a_orphaned(p_session) := p_channel_config_idx
    END

END
