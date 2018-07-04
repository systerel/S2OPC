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
    monitored_item_queue_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_monitoredItemQueue,
    a_defined_monitored_items

INVARIANT
    s_monitoredItemQueue <: t_monitoredItemQueue &
    a_defined_monitored_items : s_monitoredItemQueue <-> t_monitoredItemPointer

INITIALISATION
    s_monitoredItemQueue :(s_monitoredItemQueue = {}) ||
    a_defined_monitored_items :(a_defined_monitored_items = {})

OPERATIONS


    bres, queue <-- allocate_new_monitored_item_queue =
    BEGIN
        IF s_monitoredItemQueue = t_monitoredItemQueue
        THEN
            bres := FALSE ||
            queue := c_monitoredItemQueue_indet
        ELSE
            ANY l_queue WHERE
                l_queue : t_monitoredItemQueue_i &
                l_queue : t_monitoredItemQueue &
                l_queue /: s_monitoredItemQueue
            THEN
                s_monitoredItemQueue := s_monitoredItemQueue \/ {l_queue} ||
                queue := l_queue ||
                bres := TRUE
            END
        END
    END
    ;

    clear_and_deallocate_monitored_item_queue (p_queue) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : s_monitoredItemQueue
    THEN
        s_monitoredItemQueue := s_monitoredItemQueue - {p_queue}
    END
    ;

    bres <-- add_monitored_item_to_queue (p_queue,
                                          p_monitoredItem) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer
    THEN
        CHOICE
            bres := FALSE
        OR
            bres := TRUE ||
            a_defined_monitored_items := a_defined_monitored_items \/ {p_queue |-> p_monitoredItem}
        END
    END
    ;

    bres <-- remove_monitored_item (p_queue, p_monitoredItem) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        p_monitoredItem : t_monitoredItemPointer_i &
        p_monitoredItem : t_monitoredItemPointer
    THEN
        CHOICE
            bres := FALSE
        OR
            bres := TRUE ||
            a_defined_monitored_items := a_defined_monitored_items - {p_queue |-> p_monitoredItem}
        END
    END

END
