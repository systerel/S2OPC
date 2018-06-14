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
