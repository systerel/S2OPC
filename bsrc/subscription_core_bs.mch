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
    subscription_core_bs_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    p_bres, p_monitoredItemQueue <-- get_nodeToMonitoredItemQueue (p_nid) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        CHOICE
            p_bres := FALSE ||
            p_monitoredItemQueue := c_monitoredItemQueue_indet
        OR
            p_bres := TRUE ||
            p_monitoredItemQueue :(p_monitoredItemQueue : t_monitoredItemQueue_i &
                p_monitoredItemQueue = a_nodeToMonitoredItemQueue(p_nid))
        END
    END
    ;

    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive, revisedMaxNotificationsPerPublish
      <-- compute_create_subscription_revised_params (p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive,
                                                      p_maxNotificationsPerPublish) =
    PRE
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

    bres, timerId <-- create_periodic_publish_timer (p_subscription, p_publishInterval) =
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

    modify_publish_timer_period (p_timerId, p_revPublishInterval) =
    PRE
        p_timerId : t_timer_id_i &
        p_timerId : t_timer_id &
        p_revPublishInterval : t_opcua_duration_i
    THEN
        skip
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
