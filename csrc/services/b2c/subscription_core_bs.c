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

#include "subscription_core_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

#include <assert.h>

#include "address_space_impl.h"
#include "util_b2c.h"

SOPC_Dict* nodeIdToMonitoredItemQueue = NULL;

static uint64_t nodeid_hash(const void* id)
{
    uint64_t hash;
    SOPC_NodeId_Hash((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const void* a, const void* b)
{
    int32_t cmp;
    SOPC_NodeId_Compare((const SOPC_NodeId*) a, (const SOPC_NodeId*) b, &cmp);

    return cmp == 0;
}

static void free_monitored_item_queue(void* data)
{
    SOPC_SLinkedList* miQueue = (SOPC_SLinkedList*) data;
    SOPC_SLinkedList_Delete(miQueue);
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_bs__INITIALISATION(void)
{
    assert(nodeIdToMonitoredItemQueue == NULL);

    nodeIdToMonitoredItemQueue = SOPC_Dict_Create(NULL, nodeid_hash, nodeid_equal, NULL, free_monitored_item_queue);
    assert(nodeIdToMonitoredItemQueue != NULL);
}

void subscription_core_bs__UNINITIALISATION_subscription_core_bs(void)
{
    assert(nodeIdToMonitoredItemQueue != NULL);
    SOPC_Dict_Delete(nodeIdToMonitoredItemQueue);
    nodeIdToMonitoredItemQueue = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_bs__compute_create_subscription_revised_params(
    const constants__t_session_i subscription_core_bs__p_session,
    const constants__t_opcua_duration_i subscription_core_bs__p_reqPublishInterval,
    const t_entier4 subscription_core_bs__p_reqLifetimeCount,
    const t_entier4 subscription_core_bs__p_reqMaxKeepAlive,
    const t_entier4 subscription_core_bs__p_maxNotificationsPerPublish,
    constants__t_opcua_duration_i* const subscription_core_bs__revisedPublishInterval,
    t_entier4* const subscription_core_bs__revisedLifetimeCount,
    t_entier4* const subscription_core_bs__revisedMaxKeepAlive,
    t_entier4* const subscription_core_bs__revisedMaxNotificationsPerPublish)
{
    (void) subscription_core_bs__p_session;
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

    if (subscription_core_bs__p_reqMaxKeepAlive < SOPC_MIN_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MIN_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqMaxKeepAlive > SOPC_MAX_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MAX_LIFETIME_PUBLISH_INTERVALS;
    }
    else
    {
        *subscription_core_bs__revisedMaxKeepAlive = subscription_core_bs__p_reqMaxKeepAlive;
    }

    if (subscription_core_bs__p_reqLifetimeCount < SOPC_MIN_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MIN_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqLifetimeCount > SOPC_MAX_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MAX_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqLifetimeCount < 3 * *subscription_core_bs__revisedMaxKeepAlive)
    {
        *subscription_core_bs__revisedLifetimeCount = 3 * *subscription_core_bs__revisedMaxKeepAlive;
    }
    else
    {
        *subscription_core_bs__revisedLifetimeCount = subscription_core_bs__p_reqLifetimeCount;
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
    constants__t_monitoredItemQueue_i* const subscription_core_bs__p_monitoredItemQueue)
{
    bool valFound = false;
    bool valAdded = false;
    SOPC_SLinkedList* monitoredItemQueue =
        SOPC_Dict_Get(nodeIdToMonitoredItemQueue, subscription_core_bs__p_nid, &valFound);
    if (false == valFound)
    {
        monitoredItemQueue = SOPC_SLinkedList_Create(0);
        valAdded = SOPC_Dict_Insert(nodeIdToMonitoredItemQueue, subscription_core_bs__p_nid, monitoredItemQueue);
        if (false == valAdded)
        {
            SOPC_SLinkedList_Delete(monitoredItemQueue);
            monitoredItemQueue = NULL;
        }
    }
    *subscription_core_bs__p_monitoredItemQueue = monitoredItemQueue;
}
