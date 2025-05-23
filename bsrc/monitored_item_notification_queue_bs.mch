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
    a_monitored_item_notification_queue,
    a_notification_queue_notifications,
    monitored_item_notification_queue_iter,

    s_event_notification_queue,
    a_notif_conv_data,
    a_notif_conv_event

INVARIANT
    a_monitored_item_notification_queue : t_monitoredItemPointer >+> t_notificationQueue &
    /* Note: it should be refined to represent the ordered aspect of the queue, limited size and the discard oldest */
    a_notification_queue_notifications : t_notificationQueue <-> t_notificationData &
    monitored_item_notification_queue_iter : t_notificationQueue_i &

    s_event_notification_queue <: t_notificationQueue &
    a_notif_conv_data : t_notificationData <-> t_WriteValuePointer &
    a_notif_conv_event : t_notificationData <-> t_eventFieldList

INITIALISATION
    a_monitored_item_notification_queue :(a_monitored_item_notification_queue = {}) ||
    a_notification_queue_notifications :(a_notification_queue_notifications = {}) ||
    monitored_item_notification_queue_iter := c_notificationQueue_indet ||

    s_event_notification_queue :(s_event_notification_queue <: t_notificationQueue) ||
    a_notif_conv_data :(a_notif_conv_data : t_notificationData <-> t_WriteValuePointer) ||
    a_notif_conv_event :(a_notif_conv_event : t_notificationData <-> t_eventFieldList)

OPERATIONS

    bres, queue <-- allocate_new_monitored_item_notification_queue (p_monitoredItem, p_isEvent) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_monitoredItem /: dom(a_monitored_item_notification_queue) &
        p_isEvent : BOOL
    THEN
        CHOICE
            ANY l_queue WHERE
                l_queue : t_notificationQueue_i &
                l_queue : t_notificationQueue &
                l_queue /: ran(a_monitored_item_notification_queue)
            THEN
                bres := TRUE ||
                queue := l_queue ||
                a_monitored_item_notification_queue (p_monitoredItem) := l_queue
            END
        OR
            /* out of memory case */
            bres := FALSE ||
            queue := c_notificationQueue_indet
        END
    END
    ;

    resize_monitored_item_notification_queue (p_monitoredItem) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_monitoredItem : dom(a_monitored_item_notification_queue)
    THEN
        /* Notifications might be dropped if size reduced */
        a_notification_queue_notifications
        :(a_notification_queue_notifications : t_notificationQueue <-> t_notificationData &
            {a_monitored_item_notification_queue(p_monitoredItem)} <<| a_notification_queue_notifications =
            {a_monitored_item_notification_queue(p_monitoredItem)} <<| a_notification_queue_notifications$0)
    END
    ;

    bres, queue <-- get_monitored_item_notification_queue (p_monitoredItem) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer
    THEN
        IF p_monitoredItem /: dom(a_monitored_item_notification_queue) THEN
            bres := FALSE ||
            queue := c_notificationQueue_indet
        ELSE
            bres := TRUE ||
            queue := a_monitored_item_notification_queue(p_monitoredItem)
        END
    END
    ;

    p_nb_available_notifs <-- get_length_monitored_item_notification_queue (p_mi_notif_queue) =
    PRE
        p_mi_notif_queue : t_notificationQueue_i &
        p_mi_notif_queue : t_notificationQueue
    THEN
        p_nb_available_notifs := card(a_notification_queue_notifications[{p_mi_notif_queue}])
    END
    ;

    p_is_event_queue <-- is_event_monitored_item_notification_queue (p_monitoredItem, p_mi_notif_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_mi_notif_queue : t_notificationQueue_i &
        p_mi_notif_queue : t_notificationQueue &
        p_mi_notif_queue = a_monitored_item_notification_queue(p_monitoredItem)
    THEN
        p_is_event_queue := bool(p_mi_notif_queue : s_event_notification_queue)
    END
    ;

    clear_monitored_item_notification_queue (p_monitoredItem, p_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        a_monitored_item_notification_queue(p_monitoredItem) = p_queue
    THEN
        a_notification_queue_notifications := {p_queue} <<|a_notification_queue_notifications
    END
    ;

    clear_and_deallocate_monitored_item_notification_queue (p_monitoredItem, p_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        a_monitored_item_notification_queue(p_monitoredItem) = p_queue
    THEN
        a_monitored_item_notification_queue := {p_monitoredItem} <<| a_monitored_item_notification_queue ||
        a_notification_queue_notifications := {p_queue} <<|a_notification_queue_notifications
    END
    ;

    bres <-- add_first_monitored_item_notification_to_queue (p_monitoredItem,
                                                             p_queue,
                                                             p_nid,
                                                             p_aid,
                                                             p_VariantValuePointer,
                                                             p_ValueSc,
                                                             p_val_ts_src,
                                                             p_val_ts_srv) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue &
        a_monitored_item_notification_queue(p_monitoredItem) = p_queue &
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
            ANY l_notif WHERE
                l_notif : t_WriteValuePointer_i &
                l_notif : t_WriteValuePointer
            THEN
                bres := TRUE ||
                a_notification_queue_notifications := a_notification_queue_notifications \/ {p_queue |-> a_notif_conv_data~(l_notif)}
            END
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

    /* Note: indexRange filtering on value done in C implementation */
    bres <-- add_monitored_item_data_notification_to_queue (p_localeIds,
                                                            p_monitoredItem,
                                                            p_queue,
                                                            p_timestampToReturn,
                                                            p_writeValuePointer) =
    PRE
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        a_monitored_item_notification_queue(p_monitoredItem) = p_queue &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_writeValuePointer : t_WriteValuePointer_i &
        p_writeValuePointer : t_WriteValuePointer &
        p_queue /: s_event_notification_queue
    THEN
        bres,
        a_notification_queue_notifications
        :(bres : BOOL &
            a_notification_queue_notifications : t_notificationQueue <-> t_notificationData &
            /* Due to discard, previous content may have been modified */
            (bres = TRUE => p_queue |-> a_notif_conv_data~(p_writeValuePointer) : a_notification_queue_notifications) &
            (bres = FALSE => a_notification_queue_notifications = a_notification_queue_notifications$0)
        )
    END
    ;

    bres <-- add_monitored_item_event_notification_to_queue (p_monitoredItem,
                                                             p_queue,
                                                             p_eventFieldList) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        a_monitored_item_notification_queue(p_monitoredItem) = p_queue &
        p_eventFieldList : t_eventFieldList_i &
        p_eventFieldList : t_eventFieldList &
        p_queue : s_event_notification_queue
    THEN
        bres,
        a_notification_queue_notifications
        :(bres : BOOL &
            a_notification_queue_notifications : t_notificationQueue <-> t_notificationData &
            /* Due to discard, previous content may have been modified */
            (bres = TRUE => p_queue |-> a_notif_conv_event~(p_eventFieldList) : a_notification_queue_notifications) &
            (bres = FALSE => a_notification_queue_notifications = a_notification_queue_notifications$0)
        )
    END
    ;

    p_continue, p_isEvent <-- init_iter_monitored_item_notification (p_monitoredItem, p_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue &
        p_queue = a_monitored_item_notification_queue(p_monitoredItem)
    THEN
        p_continue := bool(a_notification_queue_notifications[{p_queue}] /= {}) ||
        monitored_item_notification_queue_iter := p_queue ||
        p_isEvent := bool(p_queue : s_event_notification_queue)
    END
    ;

    p_continue,
    p_writeValuePointer <-- continue_pop_iter_monitor_item_data_notification (p_monitoredItem, p_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue &
        p_queue = a_monitored_item_notification_queue(p_monitoredItem) &
        a_notification_queue_notifications[{p_queue}] /= {} &
        monitored_item_notification_queue_iter = p_queue &
        p_queue /: s_event_notification_queue
    THEN
        ANY l_notif WHERE
            l_notif : t_notificationData &
            l_notif : a_notification_queue_notifications[{p_queue}]
        THEN
            p_continue := bool(a_notification_queue_notifications[{p_queue}] /= {l_notif}) ||
            a_notification_queue_notifications := a_notification_queue_notifications - {p_queue |-> l_notif} ||
            p_writeValuePointer :(p_writeValuePointer : t_WriteValuePointer_i & p_writeValuePointer = a_notif_conv_data(l_notif))
        END
    END
    ;

    p_continue,
    p_eventFieldList <-- continue_pop_iter_monitor_item_event_notification (p_monitoredItem, p_queue) =
    PRE
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer &
        p_queue : t_notificationQueue_i &
        p_queue : t_notificationQueue &
        p_queue = a_monitored_item_notification_queue(p_monitoredItem) &
        a_notification_queue_notifications[{p_queue}] /= {} &
        monitored_item_notification_queue_iter = p_queue &
        p_queue /: s_event_notification_queue
    THEN
        ANY l_notif WHERE
            l_notif : t_notificationData &
            l_notif : a_notification_queue_notifications[{p_queue}]
        THEN
            p_continue := bool(a_notification_queue_notifications[{p_queue}] /= {l_notif}) ||
            a_notification_queue_notifications := a_notification_queue_notifications - {p_queue |-> l_notif} ||
            p_eventFieldList :(p_eventFieldList : t_eventFieldList_i & p_eventFieldList = a_notif_conv_event(l_notif))
        END
    END

END
