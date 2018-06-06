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
