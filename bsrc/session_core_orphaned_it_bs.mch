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
    session_core_orphaned_it_bs
    
SEES
    constants,
    session_core_1
    
ABSTRACT_VARIABLES
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned
    
INVARIANT
    av_sessions_orphaned <: t_session &
    av_sessions_to_iterate <: av_sessions_orphaned &
    av_sessions_iterated <: av_sessions_orphaned &
    av_sessions_to_iterate /\ av_sessions_iterated = {}
    
INITIALISATION
    av_sessions_to_iterate,
    av_sessions_iterated,
    av_sessions_orphaned :(av_sessions_orphaned <: t_session &
        av_sessions_to_iterate <: av_sessions_orphaned &
        av_sessions_iterated <: av_sessions_orphaned &
        av_sessions_to_iterate /\ av_sessions_iterated = {})
    
OPERATIONS
    
    continue <-- init_iter_orphaned_t_session (channel_config_idx) =
    PRE
        channel_config_idx : t_channel_config_idx_i &
        channel_config_idx : t_channel_config_idx
    THEN
        av_sessions_orphaned := dom(a_orphaned |> {channel_config_idx}) ||
        av_sessions_to_iterate := dom(a_orphaned |> {channel_config_idx}) ||
        av_sessions_iterated := {} ||
        continue := bool(channel_config_idx : ran(a_orphaned))
    END
    ;
    
    session, continue <-- continue_iter_orphaned_t_session =
    PRE
        av_sessions_to_iterate /= {}
    THEN
        ANY l_session WHERE
            l_session : t_session_i &
            l_session : t_session &
            l_session : av_sessions_to_iterate
        THEN
            av_sessions_to_iterate := av_sessions_to_iterate - {l_session} ||
            av_sessions_iterated := av_sessions_iterated \/ {l_session} ||
            session := l_session ||
            continue := bool((av_sessions_to_iterate - {l_session}) /= {})
        END
    END
    
END
