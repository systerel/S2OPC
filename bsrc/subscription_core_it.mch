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
