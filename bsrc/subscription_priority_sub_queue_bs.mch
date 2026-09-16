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
    s_prioritySubQueue,
    a_prio_queue_defined_subscriptions

INVARIANT
    s_prioritySubQueue <: t_prioritySubQueue &
    a_prio_queue_defined_subscriptions : s_prioritySubQueue <-> t_subscription &
    !prioSubQueue.(prioSubQueue : s_prioritySubQueue
                   =>
                   {prioSubQueue} <| a_prio_queue_defined_subscriptions /= {})

INITIALISATION
    s_prioritySubQueue := {} ||
    a_prio_queue_defined_subscriptions := {}

OPERATIONS


    bres, queue <-- allocate_new_prio_queue (p_first_sub) =
    PRE
        p_first_sub : t_subscription_i &
        p_first_sub : t_subscription
    THEN
        IF s_prioritySubQueue = t_prioritySubQueue
        THEN
            bres := FALSE ||
            queue := c_prioritySubQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_prioritySubQueue_i &
                l_queue : t_prioritySubQueue &
                l_queue /: s_prioritySubQueue
            THEN
                s_prioritySubQueue := s_prioritySubQueue \/ {l_queue} ||
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
        p_queue : s_prioritySubQueue
    THEN
        s_prioritySubQueue := s_prioritySubQueue - {p_queue}
    END
    ;

    bres <-- add_subscription_to_prio_queue (p_queue, p_sub) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue &
        p_sub : t_subscription_i &
        p_sub : t_subscription
    THEN
        /* TODO: add invariant in parent to guarantee p_queue : s_prioritySubQueue as precondition ? */
        bres, a_prio_queue_defined_subscriptions :
        (bres : BOOL &
         a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
         (bres = TRUE =>
           p_queue : s_prioritySubQueue &
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
        /* Last element always removed through clear_and_deallocate_prio_queue */
        a_prio_queue_defined_subscriptions /= {p_queue |-> p_sub}
    THEN
        /* TODO: add invariant in parent to guarantee p_queue : s_prioritySubQueue as precondition ? */
        a_prio_queue_defined_subscriptions :
        (a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
         (p_queue : s_prioritySubQueue =>
           a_prio_queue_defined_subscriptions = a_prio_queue_defined_subscriptions$0 - {p_queue |-> p_sub}) &
         (p_queue /: s_prioritySubQueue => a_prio_queue_defined_subscriptions = a_prio_queue_defined_subscriptions$0)
        )
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

END
