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
    subscription_priority_sub_queue_bs

SEES
    constants

ABSTRACT_VARIABLES
    a_prio_queue_defined_subscriptions

INVARIANT
    a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription

INITIALISATION
    a_prio_queue_defined_subscriptions :(a_prio_queue_defined_subscriptions = {})

OPERATIONS

    bres, queue <-- allocate_new_prio_queue (p_first_sub) =
    PRE
        p_first_sub : t_subscription_i &
        p_first_sub : t_subscription
    THEN
        IF dom(a_prio_queue_defined_subscriptions) = t_prioritySubQueue
        THEN
            bres := FALSE ||
            queue := c_prioritySubQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_prioritySubQueue_i &
                l_queue : t_prioritySubQueue &
                l_queue /: dom(a_prio_queue_defined_subscriptions)
            THEN
                a_prio_queue_defined_subscriptions := a_prio_queue_defined_subscriptions \/ {l_queue |-> p_first_sub} ||
                queue := l_queue ||
                bres := TRUE
            END
        END
    END
    ;

    clear_and_deallocate_prio_queue (p_queue) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue &
        /* Last element removed through clear_and_deallocate_prio_queue */
        card(a_prio_queue_defined_subscriptions[{p_queue}]) = 1
    THEN
        a_prio_queue_defined_subscriptions := {p_queue} <<| a_prio_queue_defined_subscriptions
    END
    ;

    bres <-- add_subscription_to_prio_queue (p_queue, p_sub) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue &
        p_sub : t_subscription_i &
        p_sub : t_subscription
    THEN
        bres, a_prio_queue_defined_subscriptions :
        (bres : BOOL &
         a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
         (bres = TRUE =>
           a_prio_queue_defined_subscriptions = a_prio_queue_defined_subscriptions$0 \/ {p_queue |-> p_sub}) &
         (bres = FALSE => a_prio_queue_defined_subscriptions = a_prio_queue_defined_subscriptions$0)
        )
    END
    ;

    remove_subscription_from_queue (p_queue, p_sub) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue &
        p_sub : t_subscription_i &
        p_sub : t_subscription &
        /* Last element removed through clear_and_deallocate_prio_queue */
        card(a_prio_queue_defined_subscriptions[{p_queue}]) > 1
    THEN
        a_prio_queue_defined_subscriptions := a_prio_queue_defined_subscriptions - {p_queue |-> p_sub}
    END
    ;

    reconfigure_priority_subscriptions_queue_last_sub (p_queue, p_sub) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue &
        p_sub : t_subscription_i &
        p_sub : t_subscription
    THEN
        skip /* Remove and add to tail */
    END
    ;

    /* Get the number of subscriptions in a queue */
    p_nb_subs <-- get_queue_nb_subs (p_queue) =
    PRE
        p_queue : t_prioritySubQueue_i
    THEN
        p_nb_subs :(p_nb_subs : t_sub_idx_i &
                    p_nb_subs = card(a_prio_queue_defined_subscriptions[{p_queue}]))
    END

END
