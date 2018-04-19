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
    monitored_item_notification_queue_bs

SEES
    constants
    
ABSTRACT_VARIABLES

    a_monitored_item_notifications
    
INVARIANT
    a_monitored_item_notifications : t_notificationQueue <-> t_monitoredItem
    
INITIALISATION
    a_monitored_item_notifications :(a_monitored_item_notifications = {})

OPERATIONS
    
    bres <-- add_monitored_item_notification_to_queue (p_queue,
                                                       p_monitoredItem,
                                                       p_writeValuePointer) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue &
        p_monitoredItem : t_monitoredItem_i &
        p_monitoredItem : t_monitoredItem &
        p_writeValuePointer : t_WriteValuePointer_i &
        p_writeValuePointer : t_WriteValuePointer
    THEN
        CHOICE
            bres := FALSE
        OR 
            bres := TRUE ||
            a_monitored_item_notifications := a_monitored_item_notifications \/ {p_queue |-> p_monitoredItem}
        END
    END
    ;
    
    
    continue, iterator <-- init_iter_monitored_item_notification (p_queue) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue
    THEN
        continue,
        iterator
        :(continue : BOOL &
          iterator : t_monitoredItemQueueIterator_i &
          (continue = TRUE => 
            iterator : t_monitoredItemQueueIterator)
        )
    END
    ;

    p_continue, 
    p_monitoredItem,
    p_writeValuePointer <-- continue_pop_iter_monitor_item_notification (p_iterator, p_queue) =
    PRE
        p_iterator : t_notificationQueueIterator_i &
        p_iterator : t_notificationQueueIterator &
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue
    THEN
        IF p_queue : dom(a_monitored_item_notifications)
        THEN
            ANY l_monitoredItem WHERE
                l_monitoredItem : t_monitoredItem_i &
                l_monitoredItem : t_monitoredItem &
                l_monitoredItem : a_monitored_item_notifications[{p_queue}]
            THEN
                p_continue := TRUE ||
                a_monitored_item_notifications := a_monitored_item_notifications - {p_queue |-> l_monitoredItem} ||                 
                p_monitoredItem := l_monitoredItem ||
                p_writeValuePointer :(p_writeValuePointer : t_WriteValuePointer_i & p_writeValuePointer : t_WriteValuePointer)
            END
        ELSE
            p_continue := FALSE ||
            p_monitoredItem := c_monitoredItem_indet ||
            p_writeValuePointer := c_WriteValuePointer_indet
        END
    END

END
