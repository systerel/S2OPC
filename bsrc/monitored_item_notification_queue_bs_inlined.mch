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
    monitored_item_notification_queue_bs_inlined

SEES
    constants,
    monitored_item_notification_queue_bs

OPERATIONS

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

END
