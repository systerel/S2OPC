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
