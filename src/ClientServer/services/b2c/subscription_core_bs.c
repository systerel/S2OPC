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

#include "subscription_core_bs.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include <assert.h>

#include "address_space_impl.h"
#include "inttypes.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_services_api_internal.h"
#include "util_b2c.h"

SOPC_Dict* nodeIdToMonitoredItemQueue = NULL;

static void free_monitored_item_queue(void* data)
{
    SOPC_SLinkedList* miQueue = (SOPC_SLinkedList*) data;
    // No deallocation for queue elements: done in UNINITIALISATION in monitored_item_pointer_bs
    SOPC_SLinkedList_Delete(miQueue);
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_bs__INITIALISATION(void)
{
    assert(nodeIdToMonitoredItemQueue == NULL);

    nodeIdToMonitoredItemQueue = SOPC_NodeId_Dict_Create(true, free_monitored_item_queue);
    assert(nodeIdToMonitoredItemQueue != NULL);
}

void subscription_core_bs__subscription_core_bs_UNINITIALISATION(void)
{
    SOPC_Dict_Delete(nodeIdToMonitoredItemQueue);
    nodeIdToMonitoredItemQueue = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_bs__compute_create_subscription_revised_params(
    const constants__t_opcua_duration_i subscription_core_bs__p_reqPublishInterval,
    const t_entier4 subscription_core_bs__p_reqLifetimeCount,
    const t_entier4 subscription_core_bs__p_reqMaxKeepAlive,
    const t_entier4 subscription_core_bs__p_maxNotificationsPerPublish,
    constants__t_opcua_duration_i* const subscription_core_bs__revisedPublishInterval,
    t_entier4* const subscription_core_bs__revisedLifetimeCount,
    t_entier4* const subscription_core_bs__revisedMaxKeepAlive,
    t_entier4* const subscription_core_bs__revisedMaxNotificationsPerPublish)
{
    if (subscription_core_bs__p_reqPublishInterval < SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION)
    {
        *subscription_core_bs__revisedPublishInterval = SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION;
    }
    else if (subscription_core_bs__p_reqPublishInterval > SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION)
    {
        *subscription_core_bs__revisedPublishInterval = SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION;
    }
    else
    {
        *subscription_core_bs__revisedPublishInterval = subscription_core_bs__p_reqPublishInterval;
    }

    if (subscription_core_bs__p_reqMaxKeepAlive < SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqMaxKeepAlive > SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS;
    }
    else
    {
        *subscription_core_bs__revisedMaxKeepAlive = subscription_core_bs__p_reqMaxKeepAlive;
    }

    if (subscription_core_bs__p_reqLifetimeCount < 3 * *subscription_core_bs__revisedMaxKeepAlive)
    {
        *subscription_core_bs__revisedLifetimeCount = 3 * *subscription_core_bs__revisedMaxKeepAlive;
    }
    else
    {
        *subscription_core_bs__revisedLifetimeCount = subscription_core_bs__p_reqLifetimeCount;
    }

    if (*subscription_core_bs__revisedLifetimeCount < SOPC_MIN_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MIN_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (*subscription_core_bs__revisedLifetimeCount > SOPC_MAX_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MAX_LIFETIME_PUBLISH_INTERVALS;
    }

    if (subscription_core_bs__p_maxNotificationsPerPublish > SOPC_MAX_OPERATIONS_PER_MSG)
    {
        *subscription_core_bs__revisedMaxNotificationsPerPublish = SOPC_MAX_OPERATIONS_PER_MSG;
    }
    else
    {
        *subscription_core_bs__revisedMaxNotificationsPerPublish = subscription_core_bs__p_maxNotificationsPerPublish;
    }
}

void subscription_core_bs__get_nodeToMonitoredItemQueue(
    const constants__t_NodeId_i subscription_core_bs__p_nid,
    t_bool* const subscription_core_bs__p_bres,
    constants__t_monitoredItemQueue_i* const subscription_core_bs__p_monitoredItemQueue)
{
    *subscription_core_bs__p_bres = false;
    *subscription_core_bs__p_monitoredItemQueue = constants__c_monitoredItemQueue_indet;
    bool valFound = false;
    bool valAdded = false;
    SOPC_SLinkedList* monitoredItemQueue =
        SOPC_Dict_Get(nodeIdToMonitoredItemQueue, subscription_core_bs__p_nid, &valFound);
    if (valFound)
    {
        *subscription_core_bs__p_bres = true;
    }
    else
    {
        // Insert a new allocated queue for the new nodeId

        SOPC_NodeId* nid = SOPC_Malloc(sizeof(SOPC_NodeId));
        monitoredItemQueue = SOPC_SLinkedList_Create(0);

        if (NULL == monitoredItemQueue || NULL == nid)
        {
            SOPC_SLinkedList_Delete(monitoredItemQueue);
            SOPC_Free(nid);
            return;
        }

        SOPC_ReturnStatus retStatus = SOPC_STATUS_NOK;
        SOPC_NodeId_Initialize(nid);
        retStatus = SOPC_NodeId_Copy(nid, subscription_core_bs__p_nid);

        if (SOPC_STATUS_OK == retStatus)
        {
            valAdded = SOPC_Dict_Insert(nodeIdToMonitoredItemQueue, nid, monitoredItemQueue);
            if (valAdded)
            {
                *subscription_core_bs__p_bres = true;
            }
        }

        if (false == *subscription_core_bs__p_bres)
        {
            SOPC_SLinkedList_Delete(monitoredItemQueue);
            monitoredItemQueue = NULL;
            SOPC_Free(nid);
        }
    }
    *subscription_core_bs__p_monitoredItemQueue = monitoredItemQueue;
}

static uint64_t getPublishCycle(double publishInterval)
{
    uint64_t msCycle = 0;
    if (publishInterval < 0)
    {
        msCycle = 0;
    }
    else if ((uint64_t) publishInterval < UINT64_MAX)
    {
        msCycle = (uint64_t) publishInterval;
    }
    else
    {
        msCycle = UINT64_MAX;
    }

    return msCycle;
}

void subscription_core_bs__create_periodic_publish_timer(
    const constants__t_subscription_i subscription_core_bs__p_subscription,
    const constants__t_opcua_duration_i subscription_core_bs__p_publishInterval,
    t_bool* const subscription_core_bs__bres,
    constants__t_timer_id_i* const subscription_core_bs__timerId)
{
    *subscription_core_bs__bres = false;

    uint64_t msCycle = getPublishCycle(subscription_core_bs__p_publishInterval);
    SOPC_Event event;

    event.eltId = (uint32_t) subscription_core_bs__p_subscription;
    event.event = TIMER_SE_PUBLISH_CYCLE_TIMEOUT;
    event.params = (uintptr_t) NULL;
    event.auxParam = 0;

    *subscription_core_bs__timerId = SOPC_EventTimer_CreatePeriodic(SOPC_Services_GetEventHandler(), event, msCycle);

    if (constants__c_timer_id_indet != *subscription_core_bs__timerId)
    {
        *subscription_core_bs__bres = true;
    }
    else
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Services: subscription=%" PRIu32 " periodic publish timer creation failed",
                                 subscription_core_bs__p_subscription);
    }
}

void subscription_core_bs__modify_publish_timer_period(
    const constants__t_timer_id_i subscription_core_bs__p_timerId,
    const constants__t_opcua_duration_i subscription_core_bs__p_revPublishInterval)
{
    uint64_t msCycle = getPublishCycle(subscription_core_bs__p_revPublishInterval);

    bool res = SOPC_EventTimer_ModifyPeriodic(subscription_core_bs__p_timerId, msCycle);
    if (!res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Subscription modify publish timer: failed to modify period of time %" PRIu32,
                               subscription_core_bs__p_timerId);
    }
}

void subscription_core_bs__delete_publish_timer(const constants__t_timer_id_i subscription_core_bs__p_timer_id)
{
    SOPC_EventTimer_Cancel(subscription_core_bs__p_timer_id);
}

void subscription_core_bs__get_next_subscription_sequence_number(
    const constants__t_sub_seq_num_i subscription_core_bs__p_prev_seq_num,
    constants__t_sub_seq_num_i* const subscription_core_bs__p_next_seq_num)
{
    if (subscription_core_bs__p_prev_seq_num == UINT32_MAX)
    {
        *subscription_core_bs__p_next_seq_num = constants__c_sub_seq_num_init;
    }
    else
    {
        *subscription_core_bs__p_next_seq_num = subscription_core_bs__p_prev_seq_num + 1;
    }
}
