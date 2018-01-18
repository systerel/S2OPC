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
    session_core_1_it

SEES
    constants

DEFINITIONS
    d_variables ==
        sessions_to_iterate,
        sessions_iterated
    ;

    d_inv ==
    (
        sessions_to_iterate <: t_session &
        sessions_iterated   <: t_session &
        sessions_to_iterate /\ sessions_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_session =
    BEGIN
        sessions_iterated   := {}        ||
        sessions_to_iterate := t_session ||
        p_continue          := bool(t_session /= {})
    END
    ;

    p_continue, p_session <-- continue_iter_session =
    PRE
        sessions_to_iterate /= {}
    THEN
        ANY l_session WHERE
            l_session : t_session_i &
            l_session : t_session   &
            l_session : sessions_to_iterate
        THEN
            sessions_iterated   := sessions_iterated   \/ {l_session} ||
            sessions_to_iterate := sessions_to_iterate -  {l_session} ||
            p_session           := l_session                          ||
            p_continue          := bool(sessions_to_iterate /= {l_session})
        END
    END
    
END
