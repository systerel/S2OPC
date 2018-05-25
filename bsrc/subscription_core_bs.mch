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
    subscription_core_bs

SEES
    constants,
    monitored_item_queue_bs
    
ABSTRACT_VARIABLES
    a_nodeToMonitoredItemQueue
    
INVARIANT
    a_nodeToMonitoredItemQueue : t_NodeId --> t_monitoredItemQueue
    
INITIALISATION
    a_nodeToMonitoredItemQueue :(a_nodeToMonitoredItemQueue : t_NodeId --> s_monitoredItemQueue)
    
OPERATIONS
    
    /* Needs UNINIT to deallocate node to monitored item queues */
    UNINITIALISATION_subscription_core_bs =
    BEGIN
        skip
    END
    ;
    
    p_monitoredItemQueue <-- get_nodeToMonitoredItemQueue (p_nid) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        p_monitoredItemQueue :(p_monitoredItemQueue : t_monitoredItemQueue_i & 
                               p_monitoredItemQueue = a_nodeToMonitoredItemQueue(p_nid))
    END
    ;
    
    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive, revisedMaxNotificationsPerPublish
      <-- compute_create_subscription_revised_params (p_session,
                                                      p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive,
                                                      p_maxNotificationsPerPublish) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_reqPublishInterval : t_opcua_duration_i &
        p_reqLifetimeCount : t_counter &
        p_reqMaxKeepAlive : t_counter &
        p_maxNotificationsPerPublish : t_counter
    THEN
        revisedPublishInterval,
            revisedLifetimeCount,
            revisedMaxKeepAlive,
            revisedMaxNotificationsPerPublish
            :(revisedPublishInterval : t_opcua_duration_i & revisedPublishInterval /= c_opcua_duration_indet &
                revisedLifetimeCount : t_counter &
                revisedLifetimeCount > 0 &
                revisedMaxKeepAlive : t_counter &
                revisedMaxKeepAlive > 0 &
                revisedLifetimeCount >= 3 * revisedMaxKeepAlive & /* part 4 table 86 */
                revisedMaxNotificationsPerPublish : t_counter &
                revisedMaxNotificationsPerPublish > 0
            )
    END
    ;
    
    bres, timerId <-- create_publish_timer (p_subscription, p_publishInterval) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_publishInterval : t_opcua_duration_i
    THEN
        CHOICE
            bres := TRUE ||
            timerId :(timerId : t_timer_id_i & timerId : t_timer_id)
        OR
            bres := FALSE ||
            timerId :(timerId : t_timer_id_i & timerId = c_timer_id_indet)
        END
    END
    ;
    
    delete_publish_timer (p_timer_id) =
    PRE
        p_timer_id : t_timer_id_i &
        p_timer_id : t_timer_id
    THEN
        skip
    END
    ;
    
    p_next_seq_num <-- get_next_subscription_sequence_number (p_prev_seq_num) =
    PRE
        p_prev_seq_num : t_sub_seq_num_i &
        p_prev_seq_num : t_sub_seq_num
    THEN
        p_next_seq_num :(p_next_seq_num : t_sub_seq_num_i & p_next_seq_num : t_sub_seq_num)
    END
    
END
