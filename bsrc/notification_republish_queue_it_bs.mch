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
    notification_republish_queue_it_bs

SEES
    constants,
    notification_republish_queue_bs

ABSTRACT_VARIABLES
    notification_republish_iter_queue,
    s_to_iterate_sub_seq_num,
    s_iterated_sub_seq_num

INVARIANT
    notification_republish_iter_queue : t_notifRepublishQueue &
    s_to_iterate_sub_seq_num <: t_sub_seq_num &
    s_iterated_sub_seq_num <: t_sub_seq_num &
    s_to_iterate_sub_seq_num /\ s_iterated_sub_seq_num = {}

INITIALISATION
    notification_republish_iter_queue := c_notifRepublishQueue_indet ||
    s_to_iterate_sub_seq_num :(s_to_iterate_sub_seq_num = {}) ||
    s_iterated_sub_seq_num :(s_iterated_sub_seq_num = {})

OPERATIONS

    nb_seq_nums <-- get_available_republish (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        nb_seq_nums := card(a_republish_notification~[{p_queue}])
    END
    ;

    continue, iterator <-- init_iter_notif_republish (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        continue := bool(a_republish_notification~[{p_queue}] /= {}) ||
        notification_republish_iter_queue := p_queue ||
        s_iterated_sub_seq_num := {} ||
        s_to_iterate_sub_seq_num := a_notification_seq_num[a_republish_notification~[{p_queue}]] ||
        iterator :(iterator : t_notifRepublishQueueIterator_i &
                   (a_republish_notification~[{p_queue}] /= {} =>
                     iterator : t_notifRepublishQueueIterator))
    END
    ;

    continue, seq_num <-- continue_iter_notif_republish (p_queue, p_iterator) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_queue = notification_republish_iter_queue &
        p_iterator : t_notifRepublishQueueIterator_i &
        p_iterator : t_notifRepublishQueueIterator &
        s_to_iterate_sub_seq_num /= {}
    THEN
        ANY l_seq_num WHERE
            l_seq_num : t_sub_seq_num_i &
            l_seq_num : s_to_iterate_sub_seq_num
        THEN
            seq_num := l_seq_num ||
            continue := bool(s_to_iterate_sub_seq_num - {l_seq_num} /= {}) ||
            s_to_iterate_sub_seq_num := s_to_iterate_sub_seq_num - {l_seq_num} ||
            s_iterated_sub_seq_num := s_iterated_sub_seq_num \/ {l_seq_num}
        END
    END
    ;

    clear_notif_republish_iterator (p_queue, p_iterator) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_queue = notification_republish_iter_queue &
        p_iterator : t_notifRepublishQueueIterator_i &
        p_iterator : t_notifRepublishQueueIterator
    THEN
        notification_republish_iter_queue := c_notifRepublishQueue_indet ||
        s_to_iterate_sub_seq_num := {} ||
        s_iterated_sub_seq_num := {}
    END

END
