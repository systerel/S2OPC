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

    a_defined_monitored_items
    
INVARIANT
    a_defined_monitored_items : t_monitoredItemQueue <-> t_monitoredItem
    
INITIALISATION
    a_defined_monitored_items :(a_defined_monitored_items : t_monitoredItemQueue <-> t_monitoredItem)

OPERATIONS
    
    bres <-- add_monitored_item_to_queue (p_queue,
                                          p_monitoredItem,
                                          p_subscription,
                                          p_node,
                                          p_aid,
                                          p_timestampToReturn,
                                          p_monitoringMode,
                                          p_clientHandle) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        p_monitoredItem : t_monitoredItem_i &
        p_monitoredItem : t_monitoredItem &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_node : t_Node_i &
        p_node : t_Node &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        CHOICE
            bres := FALSE
        OR 
            bres := TRUE ||
            a_defined_monitored_items
            :(
                a_defined_monitored_items : t_monitoredItemQueue <-> t_monitoredItem &
                {p_queue} <<| a_defined_monitored_items = {p_queue} <<| a_defined_monitored_items$0 &
                {p_queue} <| a_defined_monitored_items |>> {p_monitoredItem} = {p_queue} <| a_defined_monitored_items$0 |>> {p_monitoredItem}  &
                p_monitoredItem : ran({p_queue} <| a_defined_monitored_items)
            )
        END
    END
    ;
    
    p_bres,
    p_subscription,
    p_node,
    p_aid,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle <-- get_monitored_item (p_queue, p_monitoredItem) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        p_monitoredItem : t_monitoredItem_i &
        p_monitoredItem : t_monitoredItem
    THEN
        p_bres,
        p_subscription,
        p_node,
        p_aid,
        p_timestampToReturn,
        p_monitoringMode,
        p_clientHandle
        :(p_bres : BOOL &
            p_monitoredItem : t_monitoredItem_i &
            p_subscription : t_subscription_i &
            p_node : t_Node_i &
            p_aid : t_AttributeId_i &
            p_timestampToReturn : t_TimestampsToReturn_i &
            p_monitoringMode : t_monitoringMode_i &
            p_clientHandle : t_client_handle_i &
            (p_monitoredItem : ran({p_queue} <| a_defined_monitored_items) =>
              p_bres = TRUE &
              p_subscription : t_subscription &
              p_node : t_Node &        
              p_aid : t_AttributeId &
              p_timestampToReturn : t_TimestampsToReturn &
              p_monitoringMode : t_monitoringMode) &
            (p_monitoredItem : ran({p_queue} <| a_defined_monitored_items) => 
              p_bres = FALSE)
        )
    END
    ;
    
    bres <-- remove_monitored_item (p_queue, p_monitoredItem) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        p_monitoredItem : t_monitoredItem_i &
        p_monitoredItem : t_monitoredItem
    THEN
        CHOICE
            bres := FALSE
        OR 
            bres := TRUE ||
            a_defined_monitored_items
            :(
                a_defined_monitored_items : t_monitoredItemQueue <-> t_monitoredItem &
                {p_queue} <<| a_defined_monitored_items = {p_queue} <<| a_defined_monitored_items$0 &
                a_defined_monitored_items - {p_queue |-> p_monitoredItem} = a_defined_monitored_items$0 - {p_queue |-> p_monitoredItem}  &
                p_monitoredItem /: ran({p_queue} <| a_defined_monitored_items)
            )
        END
    END
    ;
    
    continue, iterator <-- init_iter_monitored_item (p_queue) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue
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

    continue, 
    p_monitoredItem,
    p_subscription,
    p_node,
    p_aid,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle <-- continue_iter_monitored_item (p_iterator, p_queue) =
    PRE
        p_iterator : t_monitoredItemQueueIterator_i &
        p_iterator : t_monitoredItemQueueIterator &
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue
    THEN
        continue,
        p_monitoredItem,
        p_subscription,
        p_node,
        p_aid,
        p_timestampToReturn,
        p_monitoringMode,
        p_clientHandle
        :(continue : BOOL &
            p_monitoredItem : t_monitoredItem_i &
            p_subscription : t_subscription_i &
            p_node : t_Node_i &
            p_aid : t_AttributeId_i &
            p_timestampToReturn : t_TimestampsToReturn_i &
            p_monitoringMode : t_monitoringMode_i &
            p_clientHandle : t_client_handle_i &
            (continue = TRUE =>  
              p_monitoredItem : t_monitoredItem &
              p_monitoredItem : ran({p_queue} <| a_defined_monitored_items) &
              p_subscription : t_subscription &
              p_node : t_Node &        
              p_aid : t_AttributeId &
              p_timestampToReturn : t_TimestampsToReturn &
              p_monitoringMode : t_monitoringMode
          )
        )
    END
    
END
