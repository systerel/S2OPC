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
    session_async_bs
SEES
    constants
ABSTRACT_VARIABLES
    a_session_async_create_session,
    a_session_async_activate_session
INVARIANT
    /* Relation à inverser pour traiter plusieurs sessions en attente sur une SC => besoin itérateur */
    a_session_async_create_session : t_channel_config_idx +-> t_session  &
    a_session_async_activate_session : t_session +-> t_user
INITIALISATION
    a_session_async_create_session :: {}   ||
    a_session_async_activate_session :: {}
OPERATIONS
    
    /* Session to create on channel (corresponding to given config) establishment */
    ret <-- add_session_to_create(session, channel_config_idx) =
    PRE
        session : t_session_i &
        session : t_session &
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        IF channel_config_idx : dom(a_session_async_create_session)
        THEN
            ret := FALSE
        ELSE
            ret := TRUE ||
            a_session_async_create_session(channel_config_idx) := session
        END
    END
    ;
    
    /* Is there a session to create on channel (corresponding to given config) establishment */
    session <-- is_session_to_create (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        IF channel_config_idx /: dom(a_session_async_create_session)
        THEN
            session := c_session_indet
        ELSE
            session := a_session_async_create_session(channel_config_idx)
        END
    END
    ;
    
    
    /* Session to create on channel (corresponding to given config) establishment */
    ret <-- add_session_to_activate(session, user) =
    PRE
        session : t_session_i &
        session : t_session &
        user : t_user_i &
        user : t_user
    THEN
        IF session : dom(a_session_async_activate_session)
        THEN
            ret := FALSE
        ELSE
            ret := TRUE ||
            a_session_async_activate_session(session) := user
        END
    END
    ;
    
    /* Is there a session to create on channel (corresponding to given config) establishment */
    user <-- is_session_to_activate (session) =
    PRE
        session : t_session_i &
        session : t_session
    THEN
        IF session /: dom(a_session_async_activate_session)
        THEN
            user := c_user_indet
        ELSE
            user := a_session_async_activate_session(session)
        END
    END
    
END

