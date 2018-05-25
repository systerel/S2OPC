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
    s_notificationQueue,
    a_monitored_item_notifications,
    monitored_item_notification_queue_iter
    
INVARIANT
    s_notificationQueue <: t_notificationQueue &
    a_monitored_item_notifications : s_notificationQueue <-> t_monitoredItemPointer  &
    /* In C implementation, same monitored item can be present several times (multiple notifications) 
       but it will not change the proof constraints when dequeueing. Therefore it is a simplified representation. */
    monitored_item_notification_queue_iter : s_notificationQueue
       
INITIALISATION
    s_notificationQueue :(s_notificationQueue = {}) ||
    a_monitored_item_notifications :(a_monitored_item_notifications = {}) ||
    monitored_item_notification_queue_iter := c_notificationQueue_indet

OPERATIONS
    
    bres, queue <-- allocate_new_monitored_item_notification_queue =
    BEGIN
        IF s_notificationQueue = t_notificationQueue
        THEN
            bres := FALSE ||
            queue := c_notificationQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_notificationQueue_i &
                l_queue : t_notificationQueue &
                l_queue /: s_notificationQueue
            THEN
                s_notificationQueue := s_notificationQueue \/ {l_queue} ||
                queue := l_queue ||
                bres := TRUE
            END
        END
    END
    ;
    
    clear_and_deallocate_monitored_item_notification_queue (p_queue) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue
    THEN
        s_notificationQueue := s_notificationQueue - {p_queue}
    END
    ;
    
    bres <-- add_first_monitored_item_notification_to_queue (p_queue,
                                                             p_monitoredItem,
                                                             p_nid,
                                                             p_aid,
                                                             p_VariantValuePointer,
                                                             p_ValueSc) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_VariantValuePointer : t_Variant_i &
        p_VariantValuePointer : t_Variant &
        p_ValueSc : t_StatusCode_i &
        p_ValueSc : t_StatusCode
    THEN
        CHOICE
            bres := FALSE
        OR 
            bres := TRUE ||
            a_monitored_item_notifications := a_monitored_item_notifications \/ {p_queue |-> p_monitoredItem}
        END
    END
    ;
    
    bres <-- add_monitored_item_notification_to_queue (p_queue,
                                                       p_monitoredItem,
                                                       p_writeValuePointer) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
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
    
    p_nb_notifications <-- init_iter_monitored_item_notification (p_queue) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue
    THEN
        p_nb_notifications := card(a_monitored_item_notifications[{p_queue}]) ||
        monitored_item_notification_queue_iter := p_queue
    END
    ;

    p_continue,
    p_monitoredItem,
    p_writeValuePointer <-- continue_pop_iter_monitor_item_notification (p_queue) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue &
        a_monitored_item_notifications[{p_queue}] /= {} &
        monitored_item_notification_queue_iter = p_queue
    THEN
        ANY l_monitoredItem WHERE
            l_monitoredItem : t_monitoredItemPointer_i &
            l_monitoredItem : t_monitoredItemPointer &
            l_monitoredItem : a_monitored_item_notifications[{p_queue}]
        THEN
            p_continue := bool(a_monitored_item_notifications[{p_queue}] /= {l_monitoredItem}) ||
            a_monitored_item_notifications := a_monitored_item_notifications - {p_queue |-> l_monitoredItem} ||                 
            p_monitoredItem := l_monitoredItem ||
            p_writeValuePointer :(p_writeValuePointer : t_WriteValuePointer_i & p_writeValuePointer : t_WriteValuePointer)
        END
    END

END
