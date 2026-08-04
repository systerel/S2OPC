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

#include "subscription_priority_sub_queue_bs.h"

#include "sopc_assert.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "sopc_logger.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_priority_sub_queue_bs__INITIALISATION(void)
{ /*Translated from B but an initialisation is not needed in this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_priority_sub_queue_bs__allocate_new_prio_queue(
    const constants__t_subscription_i subscription_priority_sub_queue_bs__p_first_sub,
    t_bool* const subscription_priority_sub_queue_bs__bres,
    constants__t_prioritySubQueue_i* const subscription_priority_sub_queue_bs__queue)
{
    *subscription_priority_sub_queue_bs__bres = false;
    *subscription_priority_sub_queue_bs__queue = NULL;
    SOPC_ASSERT(subscription_priority_sub_queue_bs__p_first_sub != 0);
    SOPC_SLinkedList* resList = SOPC_SLinkedList_Create(0);
    if (resList == NULL)
    {
        return;
    }

    uintptr_t res = SOPC_SLinkedList_Append(resList, subscription_priority_sub_queue_bs__p_first_sub,
                                            (uintptr_t) subscription_priority_sub_queue_bs__p_first_sub);

    if (res == 0)
    {
        SOPC_SLinkedList_Delete(resList);
    }
    else
    {
        *subscription_priority_sub_queue_bs__bres = true;
        *subscription_priority_sub_queue_bs__queue = resList;
    }
}

void subscription_priority_sub_queue_bs__clear_and_deallocate_prio_queue(
    const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue)
{
    SOPC_SLinkedList_Delete(subscription_priority_sub_queue_bs__p_queue);
}

void subscription_priority_sub_queue_bs__add_subscription_to_prio_queue(
    const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
    const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub,
    t_bool* const subscription_priority_sub_queue_bs__bres)
{
    SOPC_ASSERT(subscription_priority_sub_queue_bs__p_sub != 0);
    uintptr_t res =
        SOPC_SLinkedList_Append(subscription_priority_sub_queue_bs__p_queue, subscription_priority_sub_queue_bs__p_sub,
                                (uintptr_t) subscription_priority_sub_queue_bs__p_sub);
    *subscription_priority_sub_queue_bs__bres = (res != 0);
}

void subscription_priority_sub_queue_bs__get_queue_nb_subs(
    const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
    t_entier4* const subscription_priority_sub_queue_bs__p_nb_subs)
{
    SOPC_ASSERT(NULL != subscription_priority_sub_queue_bs__p_queue);
    uint32_t length = SOPC_SLinkedList_GetLength(subscription_priority_sub_queue_bs__p_queue);
    SOPC_ASSERT(length <= INT32_MAX);
    *subscription_priority_sub_queue_bs__p_nb_subs = (int32_t) length;
}

void subscription_priority_sub_queue_bs__remove_subscription_from_queue(
    const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
    const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub)
{
    uintptr_t removed = SOPC_SLinkedList_RemoveFromId(subscription_priority_sub_queue_bs__p_queue,
                                                      subscription_priority_sub_queue_bs__p_sub);
    if (0 == removed)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "subscription_priority_sub_queue_bs__remove_subscription_from_queue: ignored unexpected "
                               "failure for subscription %" PRIu32,
                               subscription_priority_sub_queue_bs__p_sub);
    }
}

void subscription_priority_sub_queue_bs__reconfigure_priority_subscriptions_queue_last_sub(
    const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
    const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub)
{
    SOPC_ASSERT(subscription_priority_sub_queue_bs__p_sub != 0);
    uintptr_t removed = SOPC_SLinkedList_RemoveFromId(subscription_priority_sub_queue_bs__p_queue,
                                                      subscription_priority_sub_queue_bs__p_sub);
    if (removed != 0)
    {
        uintptr_t result = SOPC_SLinkedList_Append(subscription_priority_sub_queue_bs__p_queue,
                                                   subscription_priority_sub_queue_bs__p_sub,
                                                   (uintptr_t) subscription_priority_sub_queue_bs__p_sub);
        if (0 == result)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "reconfigure_priority_subscriptions_queue_last_sub: "
                                   "ignored unexpected append "
                                   "failure for subscription %" PRIu32,
                                   subscription_priority_sub_queue_bs__p_sub);
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "reconfigure_priority_subscriptions_queue_last_sub: "
                               "ignored unexpected remove "
                               "failure for subscription %" PRIu32,
                               subscription_priority_sub_queue_bs__p_sub);
    }
}
