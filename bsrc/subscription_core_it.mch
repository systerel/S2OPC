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
    subscription_core_it
    
SEES
    constants

DEFINITIONS
    d_variables ==
    subscriptions_to_iterate,
    subscriptions_iterated
    ;

    d_inv ==
    subscriptions_to_iterate <: t_subscription &
    subscriptions_iterated   <: t_subscription &
    subscriptions_to_iterate /\ subscriptions_iterated = {}

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_subscription =
    BEGIN
        subscriptions_iterated   := {}             ||
        subscriptions_to_iterate := t_subscription ||
        p_continue                := bool(t_subscription /= {})
    END
    ;

    p_continue, p_subscription <-- continue_iter_subscription =
    PRE
        subscriptions_to_iterate /= {}
    THEN
        ANY l_subscription WHERE
            l_subscription : t_subscription_i &
            l_subscription : t_subscription   &
            l_subscription : subscriptions_to_iterate
        THEN
            subscriptions_iterated   := subscriptions_iterated   \/ {l_subscription} ||
            subscriptions_to_iterate := subscriptions_to_iterate -  {l_subscription} ||
            p_subscription           := l_subscription                               ||
            p_continue               := bool(subscriptions_to_iterate /= {l_subscription})
        END
    END
    
END
