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
    session_core_channel_lost_it_bs
    
SEES
    constants,
    session_core_1_bs
    
ABSTRACT_VARIABLES
    av_channel_sessions_to_iterate,
    av_channel_sessions_iterated,
    av_channel_lost_sessions
    
INVARIANT
    av_channel_lost_sessions <: t_session &
    av_channel_sessions_to_iterate <: av_channel_lost_sessions &
    av_channel_sessions_iterated <: av_channel_lost_sessions &
    av_channel_sessions_to_iterate /\ av_channel_sessions_iterated = {}
    
INITIALISATION
    av_channel_sessions_to_iterate,
    av_channel_sessions_iterated,
    av_channel_lost_sessions :(av_channel_lost_sessions <: t_session &
        av_channel_sessions_to_iterate <: av_channel_lost_sessions &
        av_channel_sessions_iterated <: av_channel_lost_sessions &
        av_channel_sessions_to_iterate /\ av_channel_sessions_iterated = {})
    
OPERATIONS
    
    continue <-- init_iter_channel_lost_t_session (lost_channel) =
    PRE
        lost_channel : t_channel_i &
        lost_channel : t_channel
    THEN
        av_channel_lost_sessions := dom(a_channel |> {lost_channel}) ||
        av_channel_sessions_to_iterate := dom(a_channel |> {lost_channel}) ||
        av_channel_sessions_iterated := {} ||
        continue := bool(lost_channel : ran(a_channel))
    END
    ;
    
    session, continue <-- continue_iter_channel_lost_t_session =
    PRE
        av_channel_sessions_to_iterate /= {}
    THEN
        ANY l_session WHERE
            l_session : t_session_i &
            l_session : t_session &
            l_session : av_channel_sessions_to_iterate
        THEN
            av_channel_sessions_to_iterate := av_channel_sessions_to_iterate - {l_session} ||
            av_channel_sessions_iterated := av_channel_sessions_iterated \/ {l_session} ||
            session := l_session ||
            continue := bool((av_channel_sessions_to_iterate - {l_session}) /= {})
        END
    END
    
END
