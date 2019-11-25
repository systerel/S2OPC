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
    monitored_item_notification_queue_bs

SEES
    constants,
    constants_statuscodes_bs

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
        s_notificationQueue := s_notificationQueue - {p_queue} ||
        a_monitored_item_notifications :=  {p_queue} <<| a_monitored_item_notifications
    END
    ;

    bres <-- add_first_monitored_item_notification_to_queue (p_queue,
                                                             p_monitoredItem,
                                                             p_nid,
                                                             p_aid,
                                                             p_VariantValuePointer,
                                                             p_ValueSc,
                                                             p_val_ts_src,
                                                             p_val_ts_srv) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_VariantValuePointer : t_Variant_i & /* value may be indet if status code /= Good */
        p_ValueSc : t_RawStatusCode &
        p_val_ts_src : t_Timestamp &
        p_val_ts_srv : t_Timestamp
    THEN
        CHOICE
            bres := FALSE
        OR
            bres := TRUE ||
            a_monitored_item_notifications := a_monitored_item_notifications \/ {p_queue |-> p_monitoredItem}
        END
    END
    ;

    free_first_monitored_item_notification_value (p_VariantValuePointer) =
    PRE
        p_VariantValuePointer : t_Variant_i &
        p_VariantValuePointer : t_Variant
    THEN
        skip
    END
    ;

    /* Note: indexRange filtering on value done in C implementation for now */
    bres <-- add_monitored_item_notification_to_queue (p_queue,
                                                       p_monitoredItem,
                                                       p_timestampToReturn,
                                                       p_writeValuePointer) =
    PRE
        p_queue : t_notificationQueue_i &
        p_queue : s_notificationQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
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
