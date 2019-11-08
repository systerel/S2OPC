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
    session_channel_it

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
