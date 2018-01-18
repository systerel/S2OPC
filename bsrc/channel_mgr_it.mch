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
    channel_mgr_it

SEES
    constants

DEFINITIONS
    d_variables ==
        channels_to_iterate,
        channels_iterated
    ;

    d_inv ==
        channels_to_iterate <: t_channel &
        channels_iterated   <: t_channel &
        channels_to_iterate /\ channels_iterated = {}

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :(d_inv)

OPERATIONS

    p_continue <-- init_iter_channel =
    BEGIN
        channels_iterated   := {}        ||
        channels_to_iterate := t_channel ||
        p_continue          := bool(t_channel /= {})
    END
    ;

    p_continue, p_channel <-- continue_iter_channel =
    PRE
        channels_to_iterate /= {}
    THEN
        ANY l_channel WHERE
            l_channel : t_channel_i &
            l_channel : t_channel   &
            l_channel : channels_to_iterate
        THEN
            channels_iterated   := channels_iterated   \/ {l_channel} ||
            channels_to_iterate := channels_to_iterate -  {l_channel} ||
            p_channel           := l_channel                          ||
            p_continue          := bool(channels_to_iterate /= {l_channel})
        END
    END
    
END
