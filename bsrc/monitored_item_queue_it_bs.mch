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
    monitored_item_queue_it_bs

SEES
    constants

ABSTRACT_VARIABLES
    mi_queue_it,
    mi_iterator_it,
    nb_monitored_items_it,
    s_mi_to_iterate_it,
    s_mi_iterated_it

INVARIANT
    mi_queue_it : t_monitoredItemQueue_i &
    mi_iterator_it : t_monitoredItemQueueIterator_i &
    nb_monitored_items_it : NAT &
    s_mi_to_iterate_it <: t_monitoredItemPointer_i &
    s_mi_to_iterate_it <: t_monitoredItemPointer &
    s_mi_iterated_it <: t_monitoredItemPointer_i &
    s_mi_iterated_it <: t_monitoredItemPointer &
    s_mi_to_iterate_it /\ s_mi_iterated_it = {} &
    card(s_mi_to_iterate_it \/ s_mi_iterated_it) = nb_monitored_items_it

INITIALISATION
    mi_queue_it := c_monitoredItemQueue_indet ||
    mi_iterator_it := c_monitoredItemQueueIterator_indet ||
    nb_monitored_items_it := 0 ||
    s_mi_to_iterate_it :(s_mi_to_iterate_it = {}) ||
    s_mi_iterated_it :(s_mi_iterated_it = {})

OPERATIONS

    continue, iterator <-- init_iter_monitored_item (p_queue) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue
    THEN
        mi_queue_it,
        mi_iterator_it,
        nb_monitored_items_it,
        s_mi_to_iterate_it,
        s_mi_iterated_it,
        continue,
        iterator
        :(mi_queue_it : t_monitoredItemQueue_i &
          mi_iterator_it : t_monitoredItemQueueIterator_i &
          s_mi_to_iterate_it <: t_monitoredItemPointer &
          s_mi_iterated_it <: t_monitoredItemPointer &
          continue : BOOL &
          iterator : t_monitoredItemQueueIterator_i &
          (continue = TRUE =>
              mi_queue_it = p_queue &
              iterator : t_monitoredItemQueueIterator &
              mi_iterator_it = iterator &
              nb_monitored_items_it > 0 &
              s_mi_to_iterate_it /= {} &
              card(s_mi_to_iterate_it) = nb_monitored_items_it &
              s_mi_iterated_it = {}
          )
        )
    END
    ;

    clear_iter_monitored_item (p_iterator) =
    PRE
        p_iterator : t_monitoredItemQueueIterator_i &
        p_iterator : t_monitoredItemQueueIterator &
        mi_iterator_it = p_iterator
    THEN
        mi_queue_it := c_monitoredItemQueue_indet ||
        mi_iterator_it := c_monitoredItemQueueIterator_indet ||
        nb_monitored_items_it := 0 ||
        s_mi_to_iterate_it :(s_mi_to_iterate_it = {}) ||
        s_mi_iterated_it :(s_mi_iterated_it = {})
    END
    ;

    continue,
    p_monitoredItem <-- continue_iter_monitored_item (p_iterator, p_queue) =
    PRE
        p_iterator : t_monitoredItemQueueIterator_i &
        mi_iterator_it = p_iterator &
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        mi_queue_it = p_queue &
        s_mi_to_iterate_it /= {}
    THEN
        ANY l_monitoredItem WHERE
            l_monitoredItem : t_monitoredItemPointer_i &
            l_monitoredItem : s_mi_to_iterate_it
        THEN
            continue := bool(s_mi_to_iterate_it /= {l_monitoredItem}) ||
            p_monitoredItem := l_monitoredItem ||
            s_mi_to_iterate_it := s_mi_to_iterate_it - {p_monitoredItem} ||
            s_mi_iterated_it := s_mi_to_iterate_it \/ {p_monitoredItem}
        END
    END

END
