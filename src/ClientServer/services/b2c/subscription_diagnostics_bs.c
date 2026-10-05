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

#include <string.h>

#include "libs2opc_server_internal.h"
#include "libs2opc_server_runtime_variables.h"

#include "sopc_assert.h"
#include "sopc_event_handler.h"
#include "sopc_macros.h"

#include "subscription_diagnostics_bs.h"

static t_entier4 currentSubscriptionCount = 0;
static t_entier4 cumulatedSubscriptionCount = 0;
static SOPC_Dict* publishingIntervalDict = NULL; // publishingInterval <-> nbSubscriptions

#define PUBLISH_INTERVAL_EMPTY_KEY ((uintptr_t) 0)
#define PUBLISH_INTERVAL_TOMBSTONE_KEY UINTPTR_MAX

/* Keys are revised publishing intervals truncated to milliseconds (as publish timers do), bounded by
 * [SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION, SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION]: they never match the empty or
 * tombstone keys and the dictionary contains a few entries, identity hash is sufficient. */
static uint64_t publish_interval_hash(const uintptr_t data)
{
    return (uint64_t) data;
}

static bool publish_interval_values_equal(const constants__t_opcua_duration_i left,
                                          const constants__t_opcua_duration_i right)
{
    return 0 == memcmp(&left, &right, sizeof(left));
}

// Compare publishing interval keys by value
static bool publish_interval_equal(const uintptr_t left, const uintptr_t right)
{
    return left == right;
}

static void increment_publish_interval_usage(const constants__t_opcua_duration_i publishInterval)
{
    SOPC_ASSERT(NULL != publishingIntervalDict);

    const uintptr_t key = (uintptr_t) publishInterval;

    bool found = false;

    uintptr_t nbSubscriptions = SOPC_Dict_Get(publishingIntervalDict, key, &found);

    if (found)
    {
        nbSubscriptions++;
    }
    else
    {
        // First subscription using this publishing interval
        nbSubscriptions = 1;
    }

    const bool inserted = SOPC_Dict_Insert(publishingIntervalDict, key, nbSubscriptions);
    SOPC_ASSERT(inserted);
}

static void decrement_publish_interval_usage(const constants__t_opcua_duration_i publishInterval)
{
    SOPC_ASSERT(NULL != publishingIntervalDict);

    const uintptr_t key = (uintptr_t) publishInterval;

    bool found = false;

    uintptr_t nbSubscriptions = SOPC_Dict_Get(publishingIntervalDict, key, &found);

    // Guaranteed by the B precondition : the old publishing interval shall exist in the dictionary
    SOPC_ASSERT(found);
    SOPC_ASSERT(nbSubscriptions > 0);

    if (1 == nbSubscriptions)
    {
        // Last subscription using this interval: remove the dictionary entry completely
        SOPC_Dict_Remove(publishingIntervalDict, key);
    }
    else
    {
        const bool inserted = SOPC_Dict_Insert(publishingIntervalDict, key, nbSubscriptions - 1);
        SOPC_ASSERT(inserted);
    }
}

static void notify_server_diagnostics_update(void)
{
    SOPC_EventHandler* diagnosticsEventHandler = SOPC_ServerInternal_GetDiagnosticsEventHandler();

    if (NULL != diagnosticsEventHandler)
    {
        const SOPC_ReturnStatus status =
            SOPC_EventHandler_Post(diagnosticsEventHandler, OpcUaId_Server_ServerDiagnostics_ServerDiagnosticsSummary,
                                   (uint32_t) currentSubscriptionCount, (uintptr_t) cumulatedSubscriptionCount,
                                   (uintptr_t) SOPC_Dict_Size(publishingIntervalDict));

        SOPC_UNUSED_RESULT(status);
    }
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/

void subscription_diagnostics_bs__INITIALISATION(void)
{
    currentSubscriptionCount = 0;
    cumulatedSubscriptionCount = 0;

    if (NULL != publishingIntervalDict)
    {
        SOPC_Dict_Delete(publishingIntervalDict);
        publishingIntervalDict = NULL;
    }

    publishingIntervalDict =
        SOPC_Dict_Create(PUBLISH_INTERVAL_EMPTY_KEY, publish_interval_hash, publish_interval_equal, NULL, NULL);

    SOPC_ASSERT(NULL != publishingIntervalDict);

    if (NULL != publishingIntervalDict)
    {
        SOPC_Dict_SetTombstoneKey(publishingIntervalDict, PUBLISH_INTERVAL_TOMBSTONE_KEY);
    }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void subscription_diagnostics_bs__subscription_created(
    const constants__t_opcua_duration_i subscription_diagnostics_bs__p_publishInterval)
{
    SOPC_ASSERT(currentSubscriptionCount < SOPC_MAX_SUBSCRIPTIONS);

    increment_publish_interval_usage(subscription_diagnostics_bs__p_publishInterval);
    currentSubscriptionCount++;
    cumulatedSubscriptionCount++;

    notify_server_diagnostics_update();
}

void subscription_diagnostics_bs__subscription_deleted(
    const constants__t_opcua_duration_i subscription_diagnostics_bs__p_publishInterval)
{
    SOPC_ASSERT(currentSubscriptionCount > 0);

    decrement_publish_interval_usage(subscription_diagnostics_bs__p_publishInterval);
    currentSubscriptionCount--;

    notify_server_diagnostics_update();
}

void subscription_diagnostics_bs__subscription_modified(
    const constants__t_opcua_duration_i subscription_diagnostics_bs__p_oldPublishInterval,
    const constants__t_opcua_duration_i subscription_diagnostics_bs__p_newPublishInterval)
{
    if (!publish_interval_values_equal(subscription_diagnostics_bs__p_oldPublishInterval,
                                       subscription_diagnostics_bs__p_newPublishInterval))
    {
        // Move the subscription from its previous publishing interval to the new one
        decrement_publish_interval_usage(subscription_diagnostics_bs__p_oldPublishInterval);
        increment_publish_interval_usage(subscription_diagnostics_bs__p_newPublishInterval);

        notify_server_diagnostics_update();
    }
}

void subscription_diagnostics_bs__subscription_diagnostics_bs_UNINITIALISATION(void)
{
    if (NULL != publishingIntervalDict)
    {
        SOPC_Dict_Delete(publishingIntervalDict);
        publishingIntervalDict = NULL;
    }

    currentSubscriptionCount = 0;
    cumulatedSubscriptionCount = 0;
}
