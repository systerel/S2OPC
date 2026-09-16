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
    subscription_priority_sub_queue_it_bs

SEES
    constants

ABSTRACT_VARIABLES
    a_priority_subscriptions_queue_iterator,

    prio_sub_queue_it,
    prio_sub_iterator_it,
    nb_subscriptions_it,
    s_prio_sub_to_iterate_it,
    s_prio_sub_iterated_it

INVARIANT
    a_priority_subscriptions_queue_iterator : t_prioritySubQueue >->> t_prioritySubQueueIterator &

    prio_sub_queue_it : t_prioritySubQueue_i &
    prio_sub_iterator_it : t_prioritySubQueueIterator_i &
    nb_subscriptions_it : NAT &
    s_prio_sub_to_iterate_it <: t_subscription_i &
    s_prio_sub_to_iterate_it <: t_subscription &
    s_prio_sub_iterated_it <: t_subscription_i &
    s_prio_sub_iterated_it <: t_subscription &
    s_prio_sub_to_iterate_it /\ s_prio_sub_iterated_it = {} &
    card(s_prio_sub_to_iterate_it \/ s_prio_sub_iterated_it) = nb_subscriptions_it

INITIALISATION
    a_priority_subscriptions_queue_iterator :: t_prioritySubQueue >->> t_prioritySubQueueIterator ||
    prio_sub_queue_it := c_prioritySubQueue_indet ||
    prio_sub_iterator_it := c_prioritySubQueueIterator_indet ||
    nb_subscriptions_it := 0 ||
    s_prio_sub_to_iterate_it := {} ||
    s_prio_sub_iterated_it := {}

OPERATIONS

    iterator <-- init_prio_subscriptions_queue_iterator (p_queue) =
    PRE
        p_queue : t_prioritySubQueue_i &
        p_queue : t_prioritySubQueue
    THEN
        prio_sub_queue_it,
        prio_sub_iterator_it,
        nb_subscriptions_it,
        s_prio_sub_to_iterate_it,
        s_prio_sub_iterated_it,
        iterator
        :(prio_sub_queue_it : t_prioritySubQueue_i &
          prio_sub_iterator_it : t_prioritySubQueueIterator_i &
          s_prio_sub_to_iterate_it <: t_subscription &
          s_prio_sub_iterated_it <: t_subscription &
          iterator : t_prioritySubQueueIterator_i &
          /* Note: every allocated queue contains always at least 1 element */
          prio_sub_queue_it = p_queue &
          iterator =  a_priority_subscriptions_queue_iterator(p_queue) &
          prio_sub_iterator_it = iterator &
          nb_subscriptions_it > 0 &
          s_prio_sub_to_iterate_it /= {} &
          card(s_prio_sub_to_iterate_it) = nb_subscriptions_it &
          s_prio_sub_iterated_it = {}
        )
    END
    ;

    p_continue, p_subscription, p_iterator_updated <-- continue_iter_subscription_priority_queue (p_iterator) =
    PRE
        p_iterator : t_prioritySubQueueIterator_i &
        prio_sub_iterator_it = p_iterator &
        prio_sub_queue_it = a_priority_subscriptions_queue_iterator~(p_iterator) &
        s_prio_sub_to_iterate_it /= {}
    THEN
        ANY l_subscription WHERE
            l_subscription : t_subscription_i &
            l_subscription : s_prio_sub_to_iterate_it
        THEN
            p_subscription := l_subscription ||
            s_prio_sub_to_iterate_it := s_prio_sub_to_iterate_it - {p_subscription} ||
            s_prio_sub_iterated_it := s_prio_sub_to_iterate_it \/ {p_subscription} ||
            p_continue,
            prio_sub_iterator_it,
            p_iterator_updated :(
                p_continue : BOOL &
                prio_sub_iterator_it : t_prioritySubQueueIterator_i &
                p_iterator_updated : t_prioritySubQueueIterator_i &
                p_continue = bool(s_prio_sub_to_iterate_it /= {l_subscription}) &
                (p_continue = TRUE =>
                        prio_sub_iterator_it = a_priority_subscriptions_queue_iterator(prio_sub_queue_it)) &
                (p_continue = FALSE => prio_sub_iterator_it = c_prioritySubQueueIterator_indet) &
                p_iterator_updated = prio_sub_iterator_it
            )
        END
    END

END
